#include "../SingleModuleProcessor.h"
#include "../GainModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class GainPlugin final : public SingleModuleProcessor
{
public:
    GainPlugin() : SingleModuleProcessor(std::make_unique<GainModule>(), createLayout())
    {
        p_bypass = apvts_.getRawParameterValue(ParamID::gainBypass);
        p_gainDb = apvts_.getRawParameterValue(ParamID::gainDb);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("gain", "Gain", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::gainBypass,1}, "Bypass",    true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gainDb,    1}, "Gain (dB)", NR(-60.0f, 24.0f, 0.0f, 1.5f), 0.0f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<GainModule*>(module_.get());
        mod->setBypassed(*p_bypass > 0.5f);
        mod->setGainDb(*p_gainDb);
    }

private:
    std::atomic<float>* p_bypass = nullptr;
    std::atomic<float>* p_gainDb = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GainPlugin(); }
