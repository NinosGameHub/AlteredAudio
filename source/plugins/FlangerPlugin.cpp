#include "../SingleModuleProcessor.h"
#include "../FlangerModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class FlangerPlugin final : public SingleModuleProcessor
{
public:
    FlangerPlugin() : SingleModuleProcessor(std::make_unique<FlangerModule>(), createLayout())
    {
        p_bypass   = apvts_.getRawParameterValue(ParamID::flangerBypass);
        p_rate     = apvts_.getRawParameterValue(ParamID::flangerRate);
        p_depth    = apvts_.getRawParameterValue(ParamID::flangerDepth);
        p_centre   = apvts_.getRawParameterValue(ParamID::flangerCentre);
        p_feedback = apvts_.getRawParameterValue(ParamID::flangerFeedback);
        p_wetDry   = apvts_.getRawParameterValue(ParamID::flangerWetDry);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("flanger", "Flanger", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::flangerBypass,   1}, "Bypass",      true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerRate,     1}, "Rate (Hz)",   NR(0.01f, 10.0f, 0.0f, 0.5f), 0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerDepth,    1}, "Depth (ms)",  NR(0.0f, 4.5f),                2.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerCentre,   1}, "Centre (ms)", NR(0.5f, 5.0f),                2.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerFeedback, 1}, "Feedback",    NR(-0.95f, 0.95f),             0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerWetDry,   1}, "Wet/Dry",     NR(0.0f, 1.0f),                0.7f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<FlangerModule*>(module_.get());
        mod->setBypassed (*p_bypass > 0.5f);
        mod->setRate     (*p_rate);
        mod->setDepthMs  (*p_depth);
        mod->setCentreMs (*p_centre);
        mod->setFeedback (*p_feedback);
        mod->setWetDry   (*p_wetDry);
    }

private:
    std::atomic<float>* p_bypass   = nullptr;
    std::atomic<float>* p_rate     = nullptr;
    std::atomic<float>* p_depth    = nullptr;
    std::atomic<float>* p_centre   = nullptr;
    std::atomic<float>* p_feedback = nullptr;
    std::atomic<float>* p_wetDry   = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new FlangerPlugin(); }
