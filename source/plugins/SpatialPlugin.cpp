#include "../SingleModuleProcessor.h"
#include "../SpatialModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class SpatialPlugin final : public SingleModuleProcessor
{
public:
    SpatialPlugin() : SingleModuleProcessor(std::make_unique<SpatialModule>(), createLayout())
    {
        p_bypass   = apvts_.getRawParameterValue(ParamID::spatialBypass);
        p_width    = apvts_.getRawParameterValue(ParamID::spatialWidth);
        p_pan      = apvts_.getRawParameterValue(ParamID::spatialPan);
        p_rotation = apvts_.getRawParameterValue(ParamID::spatialRotation);
        p_midGain  = apvts_.getRawParameterValue(ParamID::spatialMidGain);
        p_sideGain = apvts_.getRawParameterValue(ParamID::spatialSideGain);
        p_bassMono = apvts_.getRawParameterValue(ParamID::spatialBassMono);
        p_haasMs   = apvts_.getRawParameterValue(ParamID::spatialHaasMs);
        p_wetDry   = apvts_.getRawParameterValue(ParamID::spatialWetDry);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("spatial", "Spatial", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::spatialBypass,   1}, "Bypass",          true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialWidth,    1}, "Width",           NR(0.0f, 2.0f),         1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialPan,      1}, "Pan",             NR(-1.0f, 1.0f),        0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialRotation, 1}, "Rotation (deg)",  NR(-180.0f, 180.0f),    0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialMidGain,  1}, "Mid Gain (dB)",   NR(-24.0f, 24.0f),      0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialSideGain, 1}, "Side Gain (dB)",  NR(-24.0f, 24.0f),      0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialBassMono, 1}, "Bass Mono (Hz)",  NR(0.0f, 500.0f),       0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialHaasMs,   1}, "Haas Delay (ms)", NR(0.0f, 30.0f),        0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialWetDry,   1}, "Wet/Dry",         NR(0.0f, 1.0f),         1.0f));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<SpatialModule*>(module_.get());
        mod->setBypassed   (*p_bypass > 0.5f);
        mod->setWidth      (*p_width);
        mod->setPan        (*p_pan);
        mod->setRotationDeg(*p_rotation);
        mod->setMidGainDb  (*p_midGain);
        mod->setSideGainDb (*p_sideGain);
        mod->setBassMono   (*p_bassMono);
        mod->setHaasMs     (*p_haasMs);
        mod->setWetDry     (*p_wetDry);
    }

private:
    std::atomic<float>* p_bypass   = nullptr;
    std::atomic<float>* p_width    = nullptr;
    std::atomic<float>* p_pan      = nullptr;
    std::atomic<float>* p_rotation = nullptr;
    std::atomic<float>* p_midGain  = nullptr;
    std::atomic<float>* p_sideGain = nullptr;
    std::atomic<float>* p_bassMono = nullptr;
    std::atomic<float>* p_haasMs   = nullptr;
    std::atomic<float>* p_wetDry   = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SpatialPlugin(); }
