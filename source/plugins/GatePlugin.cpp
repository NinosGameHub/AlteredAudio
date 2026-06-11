#include "../SingleModuleProcessor.h"
#include "../GateModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class GatePlugin final : public SingleModuleProcessor
{
public:
    GatePlugin() : SingleModuleProcessor(std::make_unique<GateModule>(), createLayout())
    {
        p_bypass    = apvts_.getRawParameterValue(ParamID::gateBypass);
        p_threshold = apvts_.getRawParameterValue(ParamID::gateThreshold);
        p_attack    = apvts_.getRawParameterValue(ParamID::gateAttack);
        p_release   = apvts_.getRawParameterValue(ParamID::gateRelease);
        p_range     = apvts_.getRawParameterValue(ParamID::gateRange);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("gate", "Gate", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::gateBypass,    1}, "Bypass",         true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gateThreshold, 1}, "Threshold (dB)", NR(-96.0f, 0.0f),             -40.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gateAttack,    1}, "Attack (ms)",    NR(0.1f, 500.0f, 0.0f, 0.5f),   1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gateRelease,   1}, "Release (ms)",   NR(1.0f, 5000.0f, 0.0f, 0.5f), 100.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gateRange,     1}, "Range (dB)",     NR(-96.0f, 0.0f),               -60.0f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<GateModule*>(module_.get());
        mod->setBypassed     (*p_bypass > 0.5f);
        mod->setThresholdDb  (*p_threshold);
        mod->setAttackMs     (*p_attack);
        mod->setReleaseMs    (*p_release);
        mod->setRangeDb      (*p_range);
    }

private:
    std::atomic<float>* p_bypass    = nullptr;
    std::atomic<float>* p_threshold = nullptr;
    std::atomic<float>* p_attack    = nullptr;
    std::atomic<float>* p_release   = nullptr;
    std::atomic<float>* p_range     = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GatePlugin(); }
