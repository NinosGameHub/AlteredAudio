#include "../SingleModuleProcessor.h"
#include "../DelayModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class DelayPlugin final : public SingleModuleProcessor
{
public:
    DelayPlugin() : SingleModuleProcessor(std::make_unique<DelayModule>(), createLayout())
    {
        p_bypass    = apvts_.getRawParameterValue(ParamID::delayBypass);
        p_time      = apvts_.getRawParameterValue(ParamID::delayTime);
        p_timeR     = apvts_.getRawParameterValue(ParamID::delayTimeR);
        p_sync      = apvts_.getRawParameterValue(ParamID::delaySync);
        p_divL      = apvts_.getRawParameterValue(ParamID::delayDivL);
        p_divR      = apvts_.getRawParameterValue(ParamID::delayDivR);
        p_feedback  = apvts_.getRawParameterValue(ParamID::delayFeedback);
        p_fbLPHz    = apvts_.getRawParameterValue(ParamID::delayFbLPHz);
        p_fbHPHz    = apvts_.getRawParameterValue(ParamID::delayFbHPHz);
        p_ducking   = apvts_.getRawParameterValue(ParamID::delayDucking);
        p_diffusion = apvts_.getRawParameterValue(ParamID::delayDiffusion);
        p_wow       = apvts_.getRawParameterValue(ParamID::delayWow);
        p_modRate   = apvts_.getRawParameterValue(ParamID::delayModRate);
        p_modDepth  = apvts_.getRawParameterValue(ParamID::delayModDepth);
        p_wetDry    = apvts_.getRawParameterValue(ParamID::delayWetDry);
        p_pingPong  = apvts_.getRawParameterValue(ParamID::delayPingPong);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("delay", "Delay", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::delayBypass,    1}, "Bypass",        true));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::delayTime,  1}, "Time L (ms)", NR(0.0f, 2000.0f, 0.0f, 0.5f), 250.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::delayTimeR, 1}, "Time R (ms)", NR(0.0f, 2000.0f, 0.0f, 0.5f), 250.0f));
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::delaySync,  1}, "Tempo Sync",  false));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::delayDivL,  1}, "Division L",  DelayModule::syncDivisionNames(), 4));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::delayDivR,  1}, "Division R",  DelayModule::syncDivisionNames(), 4));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::delayFeedback,  1}, "Feedback",      NR(0.0f,   0.99f),              0.3f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayFbLPHz,    1}, "Tone LP Hz",    NR(500.0f, 20000.0f, 0.0f, 0.4f), 6000.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayFbHPHz,    1}, "Tone HP Hz",    NR(20.0f,   500.0f, 0.0f, 0.4f),    80.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayDucking,   1}, "Ducking",       NR(0.0f, 1.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayDiffusion, 1}, "Diffusion",     NR(0.0f, 1.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayWow,       1}, "Wow/Flutter",   NR(0.0f, 1.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayModRate,   1}, "Mod Rate Hz",   NR(0.01f, 5.0f), 1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayModDepth,  1}, "Mod Depth",     NR(0.0f, 1.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayWetDry,    1}, "Wet/Dry",       NR(0.0f, 1.0f), 0.5f));
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::delayPingPong,  1}, "Ping-Pong",     false));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<DelayModule*>(module_.get());
        mod->setBypassed(*p_bypass > 0.5f);

        if (auto* playHead = getPlayHead())
            if (auto pos = playHead->getPosition())
                if (auto bpm = pos->getBpm())
                    currentBpm_ = *bpm;

        float msL, msR;
        if (*p_sync > 0.5f)
        {
            msL = DelayModule::syncDivisionToMs((int)*p_divL, currentBpm_);
            msR = DelayModule::syncDivisionToMs((int)*p_divR, currentBpm_);
        }
        else
        {
            msL = *p_time;
            msR = *p_timeR;
        }
        mod->setDelayTimesMs(msL, msR);
        mod->setFeedback  (*p_feedback);
        mod->setFbLPHz    (*p_fbLPHz);
        mod->setFbHPHz    (*p_fbHPHz);
        mod->setDucking   (*p_ducking);
        mod->setDiffusion (*p_diffusion);
        mod->setWowFlutter(*p_wow);
        mod->setModRate   (*p_modRate);
        mod->setModDepth  (*p_modDepth);
        mod->setWetDry    (*p_wetDry);
        mod->setPingPong  (*p_pingPong > 0.5f);
    }

private:
    std::atomic<float>* p_bypass    = nullptr;
    std::atomic<float>* p_time      = nullptr;
    std::atomic<float>* p_timeR     = nullptr;
    std::atomic<float>* p_sync      = nullptr;
    std::atomic<float>* p_divL      = nullptr;
    std::atomic<float>* p_divR      = nullptr;
    std::atomic<float>* p_feedback  = nullptr;
    double currentBpm_ = 120.0;
    std::atomic<float>* p_fbLPHz    = nullptr;
    std::atomic<float>* p_fbHPHz    = nullptr;
    std::atomic<float>* p_ducking   = nullptr;
    std::atomic<float>* p_diffusion = nullptr;
    std::atomic<float>* p_wow       = nullptr;
    std::atomic<float>* p_modRate   = nullptr;
    std::atomic<float>* p_modDepth  = nullptr;
    std::atomic<float>* p_wetDry    = nullptr;
    std::atomic<float>* p_pingPong  = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DelayPlugin(); }
