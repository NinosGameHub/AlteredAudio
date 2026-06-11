#include "../SingleModuleProcessor.h"
#include "../CompressorModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class CompressorPlugin final : public SingleModuleProcessor
{
public:
    CompressorPlugin() : SingleModuleProcessor(std::make_unique<CompressorModule>(), createLayout())
    {
        p_bypass      = apvts_.getRawParameterValue(ParamID::compBypass);
        p_threshold   = apvts_.getRawParameterValue(ParamID::compThreshold);
        p_ratio       = apvts_.getRawParameterValue(ParamID::compRatio);
        p_attack      = apvts_.getRawParameterValue(ParamID::compAttack);
        p_release     = apvts_.getRawParameterValue(ParamID::compRelease);
        p_knee        = apvts_.getRawParameterValue(ParamID::compKnee);
        p_makeup      = apvts_.getRawParameterValue(ParamID::compMakeup);
        p_rmsPeak     = apvts_.getRawParameterValue(ParamID::compRmsPeak);
        p_autoRel     = apvts_.getRawParameterValue(ParamID::compAutoRel);
        p_saturation  = apvts_.getRawParameterValue(ParamID::compSaturation);
        p_scHP        = apvts_.getRawParameterValue(ParamID::compScHP);
        p_scLP        = apvts_.getRawParameterValue(ParamID::compScLP);
        p_stereoLink  = apvts_.getRawParameterValue(ParamID::compStereoLink);
        p_mix         = apvts_.getRawParameterValue(ParamID::compMix);
        p_lookahead   = apvts_.getRawParameterValue(ParamID::compLookahead);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("compressor", "Compressor", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::compBypass,    1}, "Bypass",         true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compThreshold, 1}, "Threshold (dB)", NR(-60.0f, 0.0f),              -12.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compRatio,     1}, "Ratio",          NR(1.0f, 100.0f, 0.0f, 0.3f),   4.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compAttack,    1}, "Attack (ms)",    NR(0.1f, 500.0f, 0.0f, 0.5f),   10.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compRelease,   1}, "Release (ms)",   NR(1.0f, 5000.0f, 0.0f, 0.5f), 100.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compKnee,      1}, "Knee (dB)",      NR(0.0f, 24.0f),                 6.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compMakeup,    1}, "Makeup (dB)",    NR(-12.0f, 12.0f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compRmsPeak,   1}, "Peak/RMS",       NR(0.0f, 1.0f),                  0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compAutoRel,   1}, "Auto Release",   NR(0.0f, 1.0f),                  0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compSaturation,1}, "Saturation",     NR(0.0f, 1.0f),                  0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compScHP,      1}, "SC HP Hz",       NR(20.0f, 500.0f, 0.0f, 0.4f),  20.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compScLP,      1}, "SC LP Hz",       NR(2000.0f, 20000.0f, 0.0f, 0.4f), 20000.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compStereoLink,1}, "Stereo Link",    NR(0.0f, 1.0f),                  1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compMix,       1}, "Mix",            NR(0.0f, 1.0f),                  1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compLookahead, 1}, "Lookahead (ms)", NR(0.0f, 10.0f),                 0.0f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<CompressorModule*>(module_.get());
        mod->setBypassed     (*p_bypass > 0.5f);
        mod->setThresholdDb  (*p_threshold);
        mod->setRatio        (*p_ratio);
        mod->setAttackMs     (*p_attack);
        mod->setReleaseMs    (*p_release);
        mod->setKneeDb       (*p_knee);
        mod->setMakeupGainDb (*p_makeup);
        mod->setRmsPeak      (*p_rmsPeak);
        mod->setAutoRelease  (*p_autoRel);
        mod->setSaturation   (*p_saturation);
        mod->setScHPHz       (*p_scHP);
        mod->setScLPHz       (*p_scLP);
        mod->setStereoLink   (*p_stereoLink);
        mod->setMix          (*p_mix);
        mod->setLookaheadMs  (*p_lookahead);
    }

private:
    std::atomic<float>* p_bypass     = nullptr;
    std::atomic<float>* p_threshold  = nullptr;
    std::atomic<float>* p_ratio      = nullptr;
    std::atomic<float>* p_attack     = nullptr;
    std::atomic<float>* p_release    = nullptr;
    std::atomic<float>* p_knee       = nullptr;
    std::atomic<float>* p_makeup     = nullptr;
    std::atomic<float>* p_rmsPeak    = nullptr;
    std::atomic<float>* p_autoRel    = nullptr;
    std::atomic<float>* p_saturation = nullptr;
    std::atomic<float>* p_scHP       = nullptr;
    std::atomic<float>* p_scLP       = nullptr;
    std::atomic<float>* p_stereoLink = nullptr;
    std::atomic<float>* p_mix        = nullptr;
    std::atomic<float>* p_lookahead  = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CompressorPlugin(); }
