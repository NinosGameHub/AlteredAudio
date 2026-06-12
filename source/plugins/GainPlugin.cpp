#include "../SingleModuleProcessor.h"
#include "../GainModule.h"
#include "../ParameterIDs.h"
#include "GainEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class GainPlugin final : public SingleModuleProcessor
{
public:
    GainPlugin() : SingleModuleProcessor(std::make_unique<GainModule>(), createLayout())
    {
        p_bypass   = apvts_.getRawParameterValue(ParamID::gainBypass);
        p_gainDb   = apvts_.getRawParameterValue(ParamID::gainDb);
        p_mode     = apvts_.getRawParameterValue(ParamID::gainMode);
        p_oversamp = apvts_.getRawParameterValue(ParamID::gainOversamp);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("gain", "Gain", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::gainBypass,  1}, "Bypass",    false));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::gainDb,      1}, "Gain (dB)", NR(-24.0f, 24.0f, 0.1f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::gainMode,    1}, "Mode",      juce::StringArray{"Stereo","Mono","Side"}, 0));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::gainOversamp,1}, "Oversampling", juce::StringArray{"1x","4x","8x"}, 0));
        layout.add(std::move(g));
        return layout;
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        baseRate_  = sampleRate;
        baseBlock_ = samplesPerBlock;

        for (int i = 0; i < 2; ++i)
        {
            os_[i] = std::make_unique<juce::dsp::Oversampling<float>>(
                2, (size_t)(i + 2),
                juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);
            os_[i]->initProcessing((size_t)samplesPerBlock);
        }

        osCur_ = 0;
        SingleModuleProcessor::prepareToPlay(sampleRate, samplesPerBlock);
        loadMeasurer_.reset(sampleRate, samplesPerBlock);

        // ITU-R BS.1770 K-weighting: high shelf + high pass, then 400 ms
        // mean-square (momentary loudness)
        for (int ch = 0; ch < 2; ++ch)
        {
            kShelf_[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate, 1681.97, 0.70711f, juce::Decibels::decibelsToGain(3.9998f));
            kHip_[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(
                sampleRate, 38.13, 0.5f);
            kShelf_[ch].reset();
            kHip_[ch].reset();
            meanSq_[ch] = 0.0f;
        }
        lufsAlpha_ = 1.0f - std::exp(-1.0f / (0.400f * (float)sampleRate));

        osCur_ = -1;   // re-sync with the parameter on the first block
    }

    void applyOversampling(int idx)
    {
        osCur_ = idx;
        const int mult = idx == 0 ? 1 : (idx == 1 ? 4 : 8);
        SingleModuleProcessor::prepareToPlay(baseRate_ * mult, baseBlock_ * mult);
        if (idx > 0) os_[idx - 1]->reset();
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        juce::AudioProcessLoadMeasurer::ScopedTimer cpuTimer(loadMeasurer_, buffer.getNumSamples());

        analysis_.pushInput(buffer);

        // MODE: stereo pass-through / mono sum / side signal
        const int mode = juce::jlimit(0, 2, (int)*p_mode);
        if (mode != 0 && buffer.getNumChannels() >= 2)
        {
            float* l = buffer.getWritePointer(0);
            float* r = buffer.getWritePointer(1);
            const int n = buffer.getNumSamples();
            if (mode == 1)          // MONO — equal-power sum on both channels
            {
                for (int i = 0; i < n; ++i)
                    l[i] = r[i] = 0.5f * (l[i] + r[i]);
            }
            else                    // SIDE — solo the side signal
            {
                for (int i = 0; i < n; ++i)
                {
                    const float s = 0.5f * (l[i] - r[i]);
                    l[i] = s;
                    r[i] = -s;
                }
            }
        }

        const int osIdx = juce::jlimit(0, 2, (int)*p_oversamp);
        if (osIdx != osCur_)
            applyOversampling(osIdx);

        if (osIdx == 0)
        {
            SingleModuleProcessor::processBlock(buffer, midi);
        }
        else
        {
            auto& os = *os_[osIdx - 1];
            juce::dsp::AudioBlock<float> blk(buffer);
            auto up = os.processSamplesUp(blk);

            const int nc = juce::jmin(2, (int)up.getNumChannels());
            float* ptrs[2] = {};
            for (int ch = 0; ch < nc; ++ch)
                ptrs[ch] = up.getChannelPointer((size_t)ch);

            juce::AudioBuffer<float> osBuf(ptrs, nc, (int)up.getNumSamples());
            SingleModuleProcessor::processBlock(osBuf, midi);

            os.processSamplesDown(blk);
        }

        analysis_.pushOutput(buffer);

        // Momentary LUFS on the final output
        {
            const int n  = buffer.getNumSamples();
            const int nc = juce::jmin(2, buffer.getNumChannels());
            for (int ch = 0; ch < nc; ++ch)
            {
                const float* d = buffer.getReadPointer(ch);
                float ms = meanSq_[ch];
                for (int i = 0; i < n; ++i)
                {
                    const float y = kHip_[ch].processSample(kShelf_[ch].processSample(d[i]));
                    ms += (y * y - ms) * lufsAlpha_;
                }
                meanSq_[ch] = ms;
            }
            const float total = meanSq_[0] + (nc > 1 ? meanSq_[1] : meanSq_[0]);
            analysis_.lufs.store(total > 1.0e-10f
                ? -0.691f + 10.0f * std::log10(total) : -100.0f);
        }

        analysis_.sampleRate.store((float)(baseRate_ > 0.0 ? baseRate_ : 44100.0));
        analysis_.cpuPct.store((float)loadMeasurer_.getLoadAsPercentage());
    }

    juce::AudioProcessorEditor* createEditor() override
    {
        return new GainEditor(*this, apvts_, analysis_);
    }

protected:
    int getExtraLatencySamples() const override
    {
        return (osCur_ >= 1 && os_[osCur_ - 1] != nullptr)
                   ? (int)os_[osCur_ - 1]->getLatencyInSamples() : 0;
    }

    void updateModuleParameters() override
    {
        auto* mod = static_cast<GainModule*>(module_.get());
        mod->setBypassed(*p_bypass > 0.5f);
        mod->setGainDb(*p_gainDb);
    }

private:
    std::atomic<float>* p_bypass   = nullptr;
    std::atomic<float>* p_gainDb   = nullptr;
    std::atomic<float>* p_mode     = nullptr;
    std::atomic<float>* p_oversamp = nullptr;

    std::unique_ptr<juce::dsp::Oversampling<float>> os_[2];   // [0]=4x, [1]=8x
    int    osCur_     = -1;
    double baseRate_  = 44100.0;
    int    baseBlock_ = 512;

    // K-weighted momentary loudness state
    juce::dsp::IIR::Filter<float> kShelf_[2], kHip_[2];
    float meanSq_[2]  = { 0.0f, 0.0f };
    float lufsAlpha_  = 0.0001f;

    juce::AudioProcessLoadMeasurer loadMeasurer_;
    GainAnalysisSource analysis_;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GainPlugin(); }
