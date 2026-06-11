#include "../SingleModuleProcessor.h"
#include "../ChorusModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class ChorusPlugin final : public SingleModuleProcessor
{
public:
    ChorusPlugin() : SingleModuleProcessor(std::make_unique<ChorusModule>(), createLayout())
    {
        p_bypass = apvts_.getRawParameterValue(ParamID::chorusBypass);
        p_rate   = apvts_.getRawParameterValue(ParamID::chorusRate);
        p_depth  = apvts_.getRawParameterValue(ParamID::chorusDepth);
        p_centre = apvts_.getRawParameterValue(ParamID::chorusCentre);
        p_voices = apvts_.getRawParameterValue(ParamID::chorusVoices);
        p_wetDry = apvts_.getRawParameterValue(ParamID::chorusWetDry);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("chorus", "Chorus", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::chorusBypass, 1}, "Bypass",      true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::chorusRate,   1}, "Rate (Hz)",   NR(0.1f, 10.0f, 0.0f, 0.5f), 0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::chorusDepth,  1}, "Depth (ms)",  NR(0.0f, 20.0f),              3.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::chorusCentre, 1}, "Centre (ms)", NR(1.0f, 30.0f),             10.0f));
        g->addChild(std::make_unique<juce::AudioParameterInt>  (PID{ParamID::chorusVoices, 1}, "Voices",      1, 4, 2));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::chorusWetDry, 1}, "Wet/Dry",     NR(0.0f, 1.0f),               0.5f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<ChorusModule*>(module_.get());
        mod->setBypassed  (*p_bypass > 0.5f);
        mod->setRate      (*p_rate);
        mod->setDepthMs   (*p_depth);
        mod->setCentreMs  (*p_centre);
        mod->setNumVoices ((int)*p_voices);
        mod->setWetDry    (*p_wetDry);
    }

private:
    std::atomic<float>* p_bypass = nullptr;
    std::atomic<float>* p_rate   = nullptr;
    std::atomic<float>* p_depth  = nullptr;
    std::atomic<float>* p_centre = nullptr;
    std::atomic<float>* p_voices = nullptr;
    std::atomic<float>* p_wetDry = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ChorusPlugin(); }
