#include "../SingleModuleProcessor.h"
#include "../ClipperModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class ClipperPlugin final : public SingleModuleProcessor
{
public:
    ClipperPlugin() : SingleModuleProcessor(std::make_unique<ClipperModule>(), createLayout())
    {
        p_bypass    = apvts_.getRawParameterValue(ParamID::clipBypass);
        p_mode      = apvts_.getRawParameterValue(ParamID::clipMode);
        p_ceiling   = apvts_.getRawParameterValue(ParamID::clipCeiling);
        p_mix       = apvts_.getRawParameterValue(ParamID::clipMix);
        p_autoGain  = apvts_.getRawParameterValue(ParamID::clipAutoGain);
        p_bias      = apvts_.getRawParameterValue(ParamID::clipBias);
        p_osQuality = apvts_.getRawParameterValue(ParamID::clipOsQuality);
        p_emphFreq  = apvts_.getRawParameterValue(ParamID::clipEmphFreq);
        p_emphGain  = apvts_.getRawParameterValue(ParamID::clipEmphGain);
        p_kneeWidth = apvts_.getRawParameterValue(ParamID::clipKneeWidth);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        const juce::StringArray clipModes   = { "Hard", "Soft", "Sine", "Tape" };
        const juce::StringArray clipOsQuals = { "2x", "4x", "8x" };
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("clipper", "Clipper", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::clipBypass,    1}, "Bypass",           true));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::clipMode,      1}, "Mode",             clipModes, 1));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipCeiling,   1}, "Ceiling (dB)",     NR(-24.0f, 0.0f),              0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipMix,       1}, "Mix",              NR(0.0f, 1.0f),                1.0f));
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::clipAutoGain,  1}, "Auto Gain",        false));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipBias,      1}, "Bias",             NR(-0.5f, 0.5f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::clipOsQuality, 1}, "Oversampling",     clipOsQuals, 1));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipEmphFreq,  1}, "Emphasis Freq Hz", NR(200.0f, 10000.0f, 0.0f, 0.4f), 3000.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipEmphGain,  1}, "Emphasis (dB)",    NR(0.0f, 12.0f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipKneeWidth, 1}, "Knee Width",       NR(0.0f, 1.0f),                0.33f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<ClipperModule*>(module_.get());
        mod->setBypassed  (*p_bypass > 0.5f);
        mod->setMode      (static_cast<ClipperModule::Mode>((int)*p_mode));
        mod->setCeilingDb (*p_ceiling);
        mod->setMix       (*p_mix);
        mod->setAutoGain  (*p_autoGain > 0.5f);
        mod->setBias      (*p_bias);
        mod->setOsQuality ((int)*p_osQuality);
        mod->setEmphFreqHz(*p_emphFreq);
        mod->setEmphGainDb(*p_emphGain);
        mod->setKneeWidth (*p_kneeWidth);
    }

private:
    std::atomic<float>* p_bypass    = nullptr;
    std::atomic<float>* p_mode      = nullptr;
    std::atomic<float>* p_ceiling   = nullptr;
    std::atomic<float>* p_mix       = nullptr;
    std::atomic<float>* p_autoGain  = nullptr;
    std::atomic<float>* p_bias      = nullptr;
    std::atomic<float>* p_osQuality = nullptr;
    std::atomic<float>* p_emphFreq  = nullptr;
    std::atomic<float>* p_emphGain  = nullptr;
    std::atomic<float>* p_kneeWidth = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ClipperPlugin(); }
