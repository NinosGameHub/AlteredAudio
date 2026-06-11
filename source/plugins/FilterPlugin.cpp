#include "../SingleModuleProcessor.h"
#include "../FilterModule.h"
#include "../ParameterIDs.h"
#include "AuroraFilterEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class FilterPlugin final : public SingleModuleProcessor
{
public:
    FilterPlugin() : SingleModuleProcessor(std::make_unique<FilterModule>(), createLayout())
    {
        auto raw = [this](const char* id) { return apvts_.getRawParameterValue(id); };
        p_bypass    = raw(ParamID::filterBypass);
        p_type      = raw(ParamID::filterType);
        p_freq      = raw(ParamID::filterFreq);
        p_q         = raw(ParamID::filterQ);
        p_gain      = raw(ParamID::filterGain);
        p_slope     = raw(ParamID::filterSlope);
        p_drive     = raw(ParamID::filterDrive);
        p_mix       = raw(ParamID::filterMix);
        p_output    = raw(ParamID::filterOutput);
        p_modSource = raw(ParamID::fltModSource);
        p_modDest   = raw(ParamID::fltModDest);
        p_modAmount = raw(ParamID::fltModAmount);
        p_lfoWave[0]  = raw(ParamID::fltLfoAWave);   p_lfoWave[1]  = raw(ParamID::fltLfoBWave);
        p_lfoRate[0]  = raw(ParamID::fltLfoARate);   p_lfoRate[1]  = raw(ParamID::fltLfoBRate);
        p_lfoDepth[0] = raw(ParamID::fltLfoADepth);  p_lfoDepth[1] = raw(ParamID::fltLfoBDepth);
        p_lfoPhase[0] = raw(ParamID::fltLfoAPhase);  p_lfoPhase[1] = raw(ParamID::fltLfoBPhase);
        p_envAttack  = raw(ParamID::fltEnvAttack);
        p_envRelease = raw(ParamID::fltEnvRelease);
        p_envSens    = raw(ParamID::fltEnvSens);
        p_lfoSync[0] = raw(ParamID::fltLfoASync);  p_lfoSync[1] = raw(ParamID::fltLfoBSync);
        p_lfoDiv[0]  = raw(ParamID::fltLfoADiv);   p_lfoDiv[1]  = raw(ParamID::fltLfoBDiv);
        p_envAtkSync = raw(ParamID::fltEnvAtkSync);
        p_envAtkDiv  = raw(ParamID::fltEnvAtkDiv);
        p_envRelSync = raw(ParamID::fltEnvRelSync);
        p_envRelDiv  = raw(ParamID::fltEnvRelDiv);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        const juce::StringArray filterTypes = { "LowPass","HighPass","BandPass","Notch","AllPass","Peak","LowShelf","HighShelf" };
        const juce::StringArray waves       = { "Sine","Triangle","Square","Random" };
        using Grp = juce::AudioProcessorParameterGroup;

        auto g = std::make_unique<Grp>("filter", "Filter", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::filterBypass,1}, "Bypass",      false));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::filterType,  1}, "Type",        filterTypes, 0));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterFreq,  1}, "Frequency",   NR(20.0f, 20000.0f, 0.0f, 0.25f), 1000.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterQ,     1}, "Resonance",   NR(0.1f, 24.0f, 0.0f, 0.4f),      0.707f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterGain,  1}, "Gain (dB)",   NR(-24.0f, 24.0f),                 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::filterSlope, 1}, "Slope",       juce::StringArray{"12","24"},      0));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterDrive, 1}, "Drive (dB)",  NR(0.0f, 24.0f),                   0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterMix,   1}, "Mix",         NR(0.0f, 1.0f),                    1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterOutput,1}, "Output (dB)", NR(-24.0f, 24.0f),                 0.0f));
        layout.add(std::move(g));

        auto m = std::make_unique<Grp>("modulation", "Modulation", "|");
        m->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::fltModSource,1}, "Mod Source", juce::StringArray{"Off","LFO A","LFO B","Env"}, 0));
        m->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::fltModDest,  1}, "Mod Dest",   juce::StringArray{"Frequency","Resonance","Drive"}, 0));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltModAmount,1}, "Mod Amount", NR(-1.0f, 1.0f), 0.0f));
        m->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::fltLfoAWave, 1}, "LFO A Wave",  waves, 0));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltLfoARate, 1}, "LFO A Rate",  NR(0.01f, 20.0f, 0.0f, 0.4f), 1.0f));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltLfoADepth,1}, "LFO A Depth", NR(0.0f, 1.0f), 1.0f));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltLfoAPhase,1}, "LFO A Phase", NR(0.0f, 360.0f), 0.0f));
        m->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::fltLfoBWave, 1}, "LFO B Wave",  waves, 1));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltLfoBRate, 1}, "LFO B Rate",  NR(0.01f, 20.0f, 0.0f, 0.4f), 0.25f));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltLfoBDepth,1}, "LFO B Depth", NR(0.0f, 1.0f), 1.0f));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltLfoBPhase,1}, "LFO B Phase", NR(0.0f, 360.0f), 0.0f));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltEnvAttack, 1}, "Env Attack",  NR(0.1f, 200.0f, 0.0f, 0.4f), 10.0f));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltEnvRelease,1}, "Env Release", NR(5.0f, 2000.0f, 0.0f, 0.4f), 150.0f));
        m->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::fltEnvSens,   1}, "Env Sens",    NR(0.0f, 10.0f), 2.5f));
        m->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::fltLfoASync,  1}, "LFO A Sync",  false));
        m->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::fltLfoADiv,   1}, "LFO A Div",   aurora::divNames(), 4));
        m->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::fltLfoBSync,  1}, "LFO B Sync",  false));
        m->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::fltLfoBDiv,   1}, "LFO B Div",   aurora::divNames(), 4));
        m->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::fltEnvAtkSync,1}, "Env Atk Sync", false));
        m->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::fltEnvAtkDiv, 1}, "Env Atk Div",  aurora::divNames(), 10));
        m->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::fltEnvRelSync,1}, "Env Rel Sync", false));
        m->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::fltEnvRelDiv, 1}, "Env Rel Div",  aurora::divNames(), 4));
        layout.add(std::move(m));
        return layout;
    }

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        SingleModuleProcessor::prepareToPlay(sampleRate, samplesPerBlock);
        loadMeasurer_.reset(sampleRate, samplesPerBlock);
        env_  = 0.0f;
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        juce::AudioProcessLoadMeasurer::ScopedTimer cpuTimer(loadMeasurer_, buffer.getNumSamples());

        const double sr = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;

        if (auto* playHead = getPlayHead())
            if (auto pos = playHead->getPosition())
                if (auto bpm = pos->getBpm())
                    currentBpm_ = *bpm;

        // Envelope follower tracks the INPUT signal (pre-filter).
        // SENS knob reads 0..10 but maps to 0..4 actual gain.
        {
            float atk = *p_envAttack, rel = *p_envRelease;
            if (*p_envAtkSync > 0.5f)
                atk = juce::jmax(0.1f, aurora::divToMs((int)*p_envAtkDiv, currentBpm_));
            if (*p_envRelSync > 0.5f)
                rel = juce::jmax(5.0f, aurora::divToMs((int)*p_envRelDiv, currentBpm_));
            const float sens = *p_envSens * 0.4f;
            const float aC = 1.0f - std::exp(-1.0f / (atk * 0.001f * (float)sr));
            const float rC = 1.0f - std::exp(-1.0f / (rel * 0.001f * (float)sr));
            const int n  = buffer.getNumSamples();
            const int nc = juce::jmin(2, buffer.getNumChannels());
            for (int i = 0; i < n; ++i)
            {
                float lvl = 0.0f;
                for (int ch = 0; ch < nc; ++ch)
                    lvl = juce::jmax(lvl, std::abs(buffer.getSample(ch, i)));
                lvl *= sens;
                env_ += (lvl - env_) * (lvl > env_ ? aC : rC);
            }
        }

        // Block-rate LFOs
        for (int l = 0; l < 2; ++l)
            advanceLfo(l, buffer.getNumSamples(), sr);

        SingleModuleProcessor::processBlock(buffer, midi);   // calls updateModuleParameters

        analysis_.sampleRate.store((float)sr);
        analysis_.cpuPct.store((float)loadMeasurer_.getLoadAsPercentage());
        analysis_.envValue.store(juce::jlimit(0.0f, 1.5f, env_));
        const int src = (int)*p_modSource;
        const int scopeL = (src == 2) ? 1 : 0;
        analysis_.lfoPhase.store(lfoPhase_[scopeL]);
        analysis_.pushBlock(buffer);
    }

    juce::AudioProcessorEditor* createEditor() override
    {
        return new AuroraFilterEditor(*this, apvts_, analysis_);
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<FilterModule*>(module_.get());
        mod->setBypassed(*p_bypass > 0.5f);
        mod->setType(static_cast<BiquadFilter::Type>((int)*p_type));
        mod->setSlope((int)*p_slope);
        mod->setGainDb(*p_gain);
        mod->setMix(*p_mix);
        mod->setOutputDb(*p_output);

        // Modulation routing
        float freq  = *p_freq;
        float q     = *p_q;
        float drive = *p_drive;

        const int   src = (int)*p_modSource;
        const float amt = *p_modAmount;
        bool  modOn = false;
        float m     = 0.0f;

        if (src == 3)   // ENV — unipolar, amount sets direction and strength
        {
            modOn = std::abs(amt) > 0.001f;
            if (modOn)
                m = juce::jlimit(0.0f, 1.0f, env_) * amt;
        }
        else if (src == 1 || src == 2)   // LFO A / B
        {
            const int   l     = src - 1;
            const float depth = *p_lfoDepth[l];
            const float wave  = lfoValue_[l];   // raw -1..1
            modOn = depth > 0.001f;
            if (modOn)
            {
                if (std::abs(amt) < 0.005f)
                    m = wave * depth;                          // amount 0: bipolar, depth sets range both ways
                else
                    m = (wave + 1.0f) * 0.5f * depth * amt;    // +amount: up only, -amount: down only
            }
        }

        if (modOn)
        {
            switch ((int)*p_modDest)
            {
                case 0: freq  = juce::jlimit(20.0f, 20000.0f, freq * std::pow(2.0f, m * 4.0f)); break;
                case 1: q     = juce::jlimit(0.1f,  24.0f,    q    * std::pow(2.0f, m * 2.5f)); break;
                case 2: drive = juce::jlimit(0.0f,  24.0f,    drive + m * 24.0f);               break;
            }
        }

        mod->setFrequency(juce::jlimit(20.0f, 20000.0f, freq));
        mod->setQ(q);
        mod->setDriveDb(drive);

        // Publish live modulated state for the display's ghost curve
        analysis_.modActive.store(modOn);
        analysis_.modFreq.store(freq);
        analysis_.modQ.store(q);
    }

private:
    void advanceLfo(int l, int numSamples, double sr)
    {
        float rate = *p_lfoRate[l];
        if (*p_lfoSync[l] > 0.5f)   // one cycle per note division at host tempo
            rate = 1000.0f / aurora::divToMs((int)*p_lfoDiv[l], currentBpm_);
        const float phOff = *p_lfoPhase[l] / 360.0f;
        const int   wave  = (int)*p_lfoWave[l];

        const float oldPhase = lfoPhase_[l];
        float ph = oldPhase + (float)(rate * numSamples / sr);
        if (ph >= 1.0f)
        {
            ph -= std::floor(ph);
            heldRandom_[l] = rng_.nextFloat() * 2.0f - 1.0f;
        }
        lfoPhase_[l] = ph;

        const float p  = ph + phOff;
        const float fr = p - std::floor(p);
        float v = 0.0f;
        switch (wave)
        {
            case 0: v = std::sin(juce::MathConstants<float>::twoPi * fr); break;
            case 1: v = 4.0f * std::abs(fr - 0.5f) - 1.0f;                break;
            case 2: v = fr < 0.5f ? 1.0f : -1.0f;                          break;
            case 3: v = heldRandom_[l];                                    break;
        }
        lfoValue_[l] = v;   // raw -1..1; depth is applied in the mod routing
    }

    std::atomic<float>* p_bypass = nullptr;  std::atomic<float>* p_type   = nullptr;
    std::atomic<float>* p_freq   = nullptr;  std::atomic<float>* p_q      = nullptr;
    std::atomic<float>* p_gain   = nullptr;  std::atomic<float>* p_slope  = nullptr;
    std::atomic<float>* p_drive  = nullptr;  std::atomic<float>* p_mix    = nullptr;
    std::atomic<float>* p_output = nullptr;
    std::atomic<float>* p_modSource = nullptr;
    std::atomic<float>* p_modDest   = nullptr;
    std::atomic<float>* p_modAmount = nullptr;
    std::atomic<float>* p_lfoWave[2]  = {};
    std::atomic<float>* p_lfoRate[2]  = {};
    std::atomic<float>* p_lfoDepth[2] = {};
    std::atomic<float>* p_lfoPhase[2] = {};
    std::atomic<float>* p_envAttack  = nullptr;
    std::atomic<float>* p_envRelease = nullptr;
    std::atomic<float>* p_envSens    = nullptr;
    std::atomic<float>* p_lfoSync[2] = {};
    std::atomic<float>* p_lfoDiv[2]  = {};
    std::atomic<float>* p_envAtkSync = nullptr;
    std::atomic<float>* p_envAtkDiv  = nullptr;
    std::atomic<float>* p_envRelSync = nullptr;
    std::atomic<float>* p_envRelDiv  = nullptr;
    double currentBpm_ = 120.0;

    float lfoPhase_[2]   = { 0.0f, 0.0f };
    float lfoValue_[2]   = { 0.0f, 0.0f };
    float heldRandom_[2] = { 0.0f, 0.0f };
    float env_ = 0.0f;
    juce::Random rng_;

    juce::AudioProcessLoadMeasurer loadMeasurer_;
    FilterAnalysisSource analysis_;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new FilterPlugin(); }
