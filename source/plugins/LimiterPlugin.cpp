#include "../SingleModuleProcessor.h"
#include "../LimiterModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class LimiterPlugin final : public SingleModuleProcessor
{
public:
    LimiterPlugin() : SingleModuleProcessor(std::make_unique<LimiterModule>(), createLayout())
    {
        p_bypass  = apvts_.getRawParameterValue(ParamID::limBypass);
        p_ceiling = apvts_.getRawParameterValue(ParamID::limCeiling);
        p_release = apvts_.getRawParameterValue(ParamID::limRelease);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("limiter", "Limiter", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::limBypass,  1}, "Bypass",       true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::limCeiling, 1}, "Ceiling (dB)", NR(-24.0f, 0.0f),             -1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::limRelease, 1}, "Release (ms)", NR(1.0f, 500.0f, 0.0f, 0.5f), 50.0f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<LimiterModule*>(module_.get());
        mod->setBypassed  (*p_bypass > 0.5f);
        mod->setCeiling   (*p_ceiling);
        mod->setReleaseMs (*p_release);
    }

private:
    std::atomic<float>* p_bypass  = nullptr;
    std::atomic<float>* p_ceiling = nullptr;
    std::atomic<float>* p_release = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new LimiterPlugin(); }
