#include "../SingleModuleProcessor.h"
#include "../PhaserModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class PhaserPlugin final : public SingleModuleProcessor
{
public:
    PhaserPlugin() : SingleModuleProcessor(std::make_unique<PhaserModule>(), createLayout())
    {
        p_bypass   = apvts_.getRawParameterValue(ParamID::phaserBypass);
        p_rate     = apvts_.getRawParameterValue(ParamID::phaserRate);
        p_depth    = apvts_.getRawParameterValue(ParamID::phaserDepth);
        p_baseFreq = apvts_.getRawParameterValue(ParamID::phaserBaseFreq);
        p_feedback = apvts_.getRawParameterValue(ParamID::phaserFeedback);
        p_stages   = apvts_.getRawParameterValue(ParamID::phaserStages);
        p_wetDry   = apvts_.getRawParameterValue(ParamID::phaserWetDry);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        const juce::StringArray phaserStages = { "2","4","6","8" };
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("phaser", "Phaser", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::phaserBypass,   1}, "Bypass",       true));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserRate,     1}, "Rate (Hz)",    NR(0.01f, 10.0f, 0.0f, 0.5f),  0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserDepth,    1}, "Depth",        NR(0.0f, 1.0f),                 1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserBaseFreq, 1}, "Base Freq Hz", NR(50.0f, 8000.0f, 0.0f, 0.3f), 500.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserFeedback, 1}, "Feedback",     NR(-0.95f, 0.95f),              0.7f));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::phaserStages,   1}, "Stages",       phaserStages, 1));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserWetDry,   1}, "Wet/Dry",      NR(0.0f, 1.0f),                 0.7f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<PhaserModule*>(module_.get());
        mod->setBypassed  (*p_bypass > 0.5f);
        mod->setRate      (*p_rate);
        mod->setDepth     (*p_depth);
        mod->setBaseFreq  (*p_baseFreq);
        mod->setFeedback  (*p_feedback);
        mod->setNumStages (((int)*p_stages + 1) * 2);
        mod->setWetDry    (*p_wetDry);
    }

private:
    std::atomic<float>* p_bypass   = nullptr;
    std::atomic<float>* p_rate     = nullptr;
    std::atomic<float>* p_depth    = nullptr;
    std::atomic<float>* p_baseFreq = nullptr;
    std::atomic<float>* p_feedback = nullptr;
    std::atomic<float>* p_stages   = nullptr;
    std::atomic<float>* p_wetDry   = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PhaserPlugin(); }
