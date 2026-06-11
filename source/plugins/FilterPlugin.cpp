#include "../SingleModuleProcessor.h"
#include "../FilterModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class FilterPlugin final : public SingleModuleProcessor
{
public:
    FilterPlugin() : SingleModuleProcessor(std::make_unique<FilterModule>(), createLayout())
    {
        p_bypass = apvts_.getRawParameterValue(ParamID::filterBypass);
        p_type   = apvts_.getRawParameterValue(ParamID::filterType);
        p_freq   = apvts_.getRawParameterValue(ParamID::filterFreq);
        p_q      = apvts_.getRawParameterValue(ParamID::filterQ);
        p_gain   = apvts_.getRawParameterValue(ParamID::filterGain);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        const juce::StringArray filterTypes = { "LowPass","HighPass","BandPass","Notch","AllPass","Peak","LowShelf","HighShelf" };
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("filter", "Filter", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::filterBypass,1}, "Bypass",    true));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::filterType,  1}, "Type",      filterTypes, 0));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterFreq,  1}, "Frequency", NR(20.0f, 20000.0f, 0.0f, 0.25f), 1000.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterQ,     1}, "Q",         NR(0.1f, 10.0f, 0.0f, 0.5f),      0.707f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterGain,  1}, "Gain (dB)", NR(-24.0f, 24.0f),                 0.0f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<FilterModule*>(module_.get());
        mod->setBypassed(*p_bypass > 0.5f);
        mod->setType(static_cast<BiquadFilter::Type>((int)*p_type));
        mod->setFrequency(juce::jlimit(20.0f, 20000.0f, (float)*p_freq));
        mod->setQ(*p_q);
        mod->setGainDb(*p_gain);
    }

private:
    std::atomic<float>* p_bypass = nullptr;
    std::atomic<float>* p_type   = nullptr;
    std::atomic<float>* p_freq   = nullptr;
    std::atomic<float>* p_q      = nullptr;
    std::atomic<float>* p_gain   = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new FilterPlugin(); }
