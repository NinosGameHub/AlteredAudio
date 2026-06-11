#include "../SingleModuleProcessor.h"
#include "../EQModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class EQPlugin final : public SingleModuleProcessor
{
public:
    EQPlugin() : SingleModuleProcessor(std::make_unique<EQModule>(), createLayout())
    {
        p_bypass = apvts_.getRawParameterValue(ParamID::eqBypass);
        for (int n = 1; n <= EQModule::kMaxBands; ++n)
        {
            p_enabled[n-1] = apvts_.getRawParameterValue(ParamID::eqEnabled(n));
            p_type   [n-1] = apvts_.getRawParameterValue(ParamID::eqType(n));
            p_freq   [n-1] = apvts_.getRawParameterValue(ParamID::eqFreq(n));
            p_q      [n-1] = apvts_.getRawParameterValue(ParamID::eqQ(n));
            p_gain   [n-1] = apvts_.getRawParameterValue(ParamID::eqGain(n));
        }
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        const juce::StringArray filterTypes = { "LowPass","HighPass","BandPass","Notch","AllPass","Peak","LowShelf","HighShelf" };
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("eq", "EQ", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>(PID{ParamID::eqBypass,1}, "Bypass", true));
        for (int n = 1; n <= EQModule::kMaxBands; ++n)
        {
            auto band = std::make_unique<Grp>("eq_band_"+juce::String(n), "Band "+juce::String(n), "|");
            band->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::eqEnabled(n),1}, "On",        false));
            band->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::eqType(n),   1}, "Type",      filterTypes, 5));
            band->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::eqFreq(n),   1}, "Frequency", NR(20.0f, 20000.0f, 0.0f, 0.25f), 1000.0f));
            band->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::eqQ(n),      1}, "Q",         NR(0.1f, 10.0f, 0.0f, 0.5f),      0.707f));
            band->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::eqGain(n),   1}, "Gain (dB)", NR(-24.0f, 24.0f),                 0.0f));
            g->addChild(std::move(band));
        }
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* eq = static_cast<EQModule*>(module_.get());
        eq->setBypassed(*p_bypass > 0.5f);
        for (int n = 0; n < EQModule::kMaxBands; ++n)
        {
            EQModule::BandParams bp;
            bp.enabled   = *p_enabled[n] > 0.5f;
            bp.type      = static_cast<BiquadFilter::Type>((int)*p_type[n]);
            bp.frequency = juce::jlimit(20.0f, 20000.0f, (float)*p_freq[n]);
            bp.q         = juce::jlimit(0.1f, 10.0f, (float)*p_q[n]);
            bp.gainDb    = *p_gain[n];
            eq->setBand(n, bp);
        }
    }

private:
    std::atomic<float>* p_bypass = nullptr;
    std::atomic<float>* p_enabled[EQModule::kMaxBands] = {};
    std::atomic<float>* p_type   [EQModule::kMaxBands] = {};
    std::atomic<float>* p_freq   [EQModule::kMaxBands] = {};
    std::atomic<float>* p_q      [EQModule::kMaxBands] = {};
    std::atomic<float>* p_gain   [EQModule::kMaxBands] = {};
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new EQPlugin(); }
