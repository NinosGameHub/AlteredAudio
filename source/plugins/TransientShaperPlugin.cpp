#include "../SingleModuleProcessor.h"
#include "../TransientShaperModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class TransientShaperPlugin final : public SingleModuleProcessor
{
public:
    TransientShaperPlugin() : SingleModuleProcessor(std::make_unique<TransientShaperModule>(), createLayout())
    {
        p_bypass  = apvts_.getRawParameterValue(ParamID::transBypass);
        p_attack  = apvts_.getRawParameterValue(ParamID::transAttack);
        p_sustain = apvts_.getRawParameterValue(ParamID::transSustain);
        p_speed   = apvts_.getRawParameterValue(ParamID::transSpeed);
        p_link    = apvts_.getRawParameterValue(ParamID::transLink);
        p_wetDry  = apvts_.getRawParameterValue(ParamID::transWetDry);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("transient", "Transient Shaper", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::transBypass,  1}, "Bypass",       true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transAttack,  1}, "Attack (dB)",  NR(-24.0f, 24.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transSustain, 1}, "Sustain (dB)", NR(-24.0f, 24.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transSpeed,   1}, "Speed",        NR(0.0f, 1.0f),    0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transLink,    1}, "Stereo Link",  NR(0.0f, 1.0f),    1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transWetDry,  1}, "Wet/Dry",      NR(0.0f, 1.0f),    1.0f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<TransientShaperModule*>(module_.get());
        mod->setBypassed      (*p_bypass > 0.5f);
        mod->setAttackGainDb  (*p_attack);
        mod->setSustainGainDb (*p_sustain);
        mod->setSpeed         (*p_speed);
        mod->setStereoLink    (*p_link);
        mod->setWetDry        (*p_wetDry);
    }

private:
    std::atomic<float>* p_bypass  = nullptr;
    std::atomic<float>* p_attack  = nullptr;
    std::atomic<float>* p_sustain = nullptr;
    std::atomic<float>* p_speed   = nullptr;
    std::atomic<float>* p_link    = nullptr;
    std::atomic<float>* p_wetDry  = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TransientShaperPlugin(); }
