#include "../SingleModuleProcessor.h"
#include "../ReverbModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class ReverbPlugin final : public SingleModuleProcessor
{
public:
    ReverbPlugin() : SingleModuleProcessor(std::make_unique<ReverbModule>(), createLayout())
    {
        p_bypass    = apvts_.getRawParameterValue(ParamID::reverbBypass);
        p_room      = apvts_.getRawParameterValue(ParamID::reverbRoom);
        p_damp      = apvts_.getRawParameterValue(ParamID::reverbDamp);
        p_diffusion = apvts_.getRawParameterValue(ParamID::reverbDiffusion);
        p_preDelay  = apvts_.getRawParameterValue(ParamID::reverbPreDelay);
        p_modRate   = apvts_.getRawParameterValue(ParamID::reverbModRate);
        p_erLevel   = apvts_.getRawParameterValue(ParamID::reverbErLevel);
        p_lowCut    = apvts_.getRawParameterValue(ParamID::reverbLowCut);
        p_shimmer   = apvts_.getRawParameterValue(ParamID::reverbShimmer);
        p_pitchSemi = apvts_.getRawParameterValue(ParamID::reverbPitchSemi);
        p_wetDry    = apvts_.getRawParameterValue(ParamID::reverbWetDry);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("reverb", "Reverb", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::reverbBypass,    1}, "Bypass",          true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbRoom,      1}, "Decay",           NR(0.0f,   1.0f),   0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbDamp,      1}, "Damping",         NR(0.0f,   1.0f),   0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbDiffusion, 1}, "Diffusion",       NR(0.0f,   1.0f),   0.75f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbPreDelay,  1}, "Pre-Delay ms",    NR(0.0f, 100.0f),  20.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbModRate,   1}, "Mod Rate Hz",     NR(0.01f,  4.0f),   0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbErLevel,   1}, "ER Level",        NR(0.0f,   1.0f),   0.3f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbLowCut,    1}, "Low Cut Hz",      NR(20.0f, 500.0f), 80.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbShimmer,   1}, "Shimmer",         NR(0.0f,   1.0f),   0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbPitchSemi, 1}, "Pitch Semitones", NR(-24.0f,24.0f),  12.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbWetDry,    1}, "Wet/Dry",         NR(0.0f,   1.0f),   0.3f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<ReverbModule*>(module_.get());
        mod->setBypassed(*p_bypass > 0.5f);
        mod->setDecay      (juce::jlimit(0.0f, 1.0f, (float)*p_room));
        mod->setDamping    (*p_damp);
        mod->setDiffusion  (*p_diffusion);
        mod->setPreDelayMs (*p_preDelay);
        mod->setModRate    (*p_modRate);
        mod->setErLevel    (*p_erLevel);
        mod->setLowCutHz   (*p_lowCut);
        mod->setShimmer    (*p_shimmer);
        mod->setPitchSemi  (*p_pitchSemi);
        mod->setWetDry     (*p_wetDry);
    }

private:
    std::atomic<float>* p_bypass    = nullptr;
    std::atomic<float>* p_room      = nullptr;
    std::atomic<float>* p_damp      = nullptr;
    std::atomic<float>* p_diffusion = nullptr;
    std::atomic<float>* p_preDelay  = nullptr;
    std::atomic<float>* p_modRate   = nullptr;
    std::atomic<float>* p_erLevel   = nullptr;
    std::atomic<float>* p_lowCut    = nullptr;
    std::atomic<float>* p_shimmer   = nullptr;
    std::atomic<float>* p_pitchSemi = nullptr;
    std::atomic<float>* p_wetDry    = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ReverbPlugin(); }
