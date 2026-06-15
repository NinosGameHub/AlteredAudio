// ============================================================
//  Gain 76 — fully self-contained processor.
//  Everything the plugin needs lives in Gain76/ — no includes
//  from the AlteredAudio rack sources. Param IDs and the plugin
//  code (AGn1) match the old shared build, so sessions reload.
// ============================================================
#include "GainEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class Gain76Processor final : public juce::AudioProcessor
{
public:
    Gain76Processor()
        : AudioProcessor(BusesProperties()
            .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          apvts_(*this, nullptr, "Params", createLayout())
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

    // ---- AudioProcessor boilerplate ----
    const juce::String getName() const override { return "Gain 76"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms()                             override { return 1; }
    int getCurrentProgram()                          override { return 0; }
    void setCurrentProgram(int)                      override {}
    const juce::String getProgramName(int)           override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    bool hasEditor() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override
    {
        return new GainEditor(*this, apvts_, analysis_);
    }

    void getStateInformation(juce::MemoryBlock& destData) override
    {
        if (auto xml = apvts_.copyState().createXml())
            copyXmlToBinary(*xml, destData);
    }

    void setStateInformation(const void* data, int sizeInBytes) override
    {
        if (auto xml = getXmlFromBinary(data, sizeInBytes))
            if (xml->hasTagName(apvts_.state.getType()))
                apvts_.replaceState(juce::ValueTree::fromXml(*xml));
    }

    // ---- lifecycle ----
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
        osCur_ = -1;   // re-sync with the parameter on the first block

        smoother_.reset(sampleRate, 0.02);
        smoother_.setCurrentAndTargetValue(
            juce::Decibels::decibelsToGain(p_gainDb ? p_gainDb->load() : 0.0f));

        dryBuffer_.setSize(2, samplesPerBlock, false, true, false);
        bypassGain_ = (p_bypass && *p_bypass > 0.5f) ? 0.0f : 1.0f;

        // ITU-R BS.1770 K-weighting → 400 ms mean square (momentary LUFS)
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

        loadMeasurer_.reset(sampleRate, samplesPerBlock);
        setLatencySamples(0);
    }

    void releaseResources() override {}

    // ---- processing ----
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals noDenormals;
        juce::AudioProcessLoadMeasurer::ScopedTimer cpuTimer(loadMeasurer_, buffer.getNumSamples());

        for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
            buffer.clear(ch, 0, buffer.getNumSamples());

        analysis_.pushInput(buffer);

        // Reconfigure when the oversampling choice changes; report PDC
        const int osIdx = juce::jlimit(0, 2, (int)*p_oversamp);
        if (osIdx != osCur_)
        {
            osCur_ = osIdx;
            if (osIdx > 0) os_[osIdx - 1]->reset();
            const int mult = osIdx == 0 ? 1 : (osIdx == 1 ? 4 : 8);
            smoother_.reset(baseRate_ * mult, 0.02);
            smoother_.setCurrentAndTargetValue(smoother_.getTargetValue());
        }
        const int latency = osIdx > 0 ? (int)os_[osIdx - 1]->getLatencyInSamples() : 0;
        if (latency != getLatencySamples())
            setLatencySamples(latency);

        smoother_.setTargetValue(juce::Decibels::decibelsToGain(p_gainDb->load()));

        // Bypass crossfade — 64-sample ramp, no clicks
        const float targetGain = (*p_bypass > 0.5f) ? 0.0f : 1.0f;
        const bool  needsRamp  = std::abs(bypassGain_ - targetGain) > 1.0e-6f;

        if (!needsRamp && targetGain == 0.0f)
        {
            finishBlock(buffer);   // fully bypassed — meter the dry signal
            return;
        }

        const int numSamples  = buffer.getNumSamples();
        const int numChannels = juce::jmin(buffer.getNumChannels(), dryBuffer_.getNumChannels());
        const int safe        = juce::jmin(numSamples, dryBuffer_.getNumSamples());

        if (needsRamp)
            for (int ch = 0; ch < numChannels; ++ch)
                dryBuffer_.copyFrom(ch, 0, buffer, ch, 0, safe);

        processWet(buffer, osIdx);

        if (needsRamp)
        {
            constexpr int kRampSamples = 64;
            const float step = (targetGain > bypassGain_ ? 1.0f : -1.0f) / (float)kRampSamples;
            for (int s = 0; s < safe; ++s)
            {
                bypassGain_ = juce::jlimit(0.0f, 1.0f, bypassGain_ + step);
                const float dryAmt = 1.0f - bypassGain_;
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    float*       out = buffer.getWritePointer(ch);
                    const float* dry = dryBuffer_.getReadPointer(ch);
                    out[s] = dry[s] * dryAmt + out[s] * bypassGain_;
                }
            }
        }

        finishBlock(buffer);
    }

private:
    // MODE transform + smoothed gain, optionally inside the oversampler
    void processWet(juce::AudioBuffer<float>& buffer, int osIdx)
    {
        // MODE: stereo pass-through / mono sum / side signal
        const int mode = juce::jlimit(0, 2, (int)*p_mode);
        if (mode != 0 && buffer.getNumChannels() >= 2)
        {
            float* l = buffer.getWritePointer(0);
            float* r = buffer.getWritePointer(1);
            const int n = buffer.getNumSamples();
            if (mode == 1)          // MONO — sum on both channels
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

        if (osIdx == 0)
        {
            applyGain(buffer.getArrayOfWritePointers(),
                      juce::jmin(2, buffer.getNumChannels()), buffer.getNumSamples());
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

            applyGain(ptrs, nc, (int)up.getNumSamples());
            os.processSamplesDown(blk);
        }
    }

    void applyGain(float* const* chans, int numChannels, int numSamples)
    {
        if (!smoother_.isSmoothing())
        {
            const float g = smoother_.getCurrentValue();
            for (int ch = 0; ch < numChannels; ++ch)
                juce::FloatVectorOperations::multiply(chans[ch], g, numSamples);
            return;
        }
        for (int i = 0; i < numSamples; ++i)
        {
            const float g = smoother_.getNextValue();
            for (int ch = 0; ch < numChannels; ++ch)
                chans[ch][i] *= g;
        }
    }

    // output metering + momentary LUFS + footer stats
    void finishBlock(juce::AudioBuffer<float>& buffer)
    {
        analysis_.pushOutput(buffer);

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

        analysis_.sampleRate.store((float)(baseRate_ > 0.0 ? baseRate_ : 44100.0));
        analysis_.cpuPct.store((float)loadMeasurer_.getLoadAsPercentage());
    }

    APVTS apvts_;
    std::atomic<float>* p_bypass   = nullptr;
    std::atomic<float>* p_gainDb   = nullptr;
    std::atomic<float>* p_mode     = nullptr;
    std::atomic<float>* p_oversamp = nullptr;

    juce::LinearSmoothedValue<float> smoother_;
    float bypassGain_ = 1.0f;
    juce::AudioBuffer<float> dryBuffer_;

    std::unique_ptr<juce::dsp::Oversampling<float>> os_[2];   // [0]=4x, [1]=8x
    int    osCur_     = -1;
    double baseRate_  = 44100.0;
    int    baseBlock_ = 512;

    juce::dsp::IIR::Filter<float> kShelf_[2], kHip_[2];
    float meanSq_[2] = { 0.0f, 0.0f };
    float lufsAlpha_ = 0.0001f;

    juce::AudioProcessLoadMeasurer loadMeasurer_;
    GainAnalysisSource analysis_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Gain76Processor)
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Gain76Processor(); }
