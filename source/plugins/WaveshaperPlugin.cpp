#include "../SingleModuleProcessor.h"
#include "../WaveshaperModule.h"
#include "../ParameterIDs.h"

using APVTS = juce::AudioProcessorValueTreeState;
using PID   = juce::ParameterID;
using NR    = juce::NormalisableRange<float>;

class WaveshaperPlugin final : public SingleModuleProcessor
{
public:
    WaveshaperPlugin() : SingleModuleProcessor(std::make_unique<WaveshaperModule>(), createLayout())
    {
        p_bypass   = apvts_.getRawParameterValue(ParamID::wsBypass);
        p_algo     = apvts_.getRawParameterValue(ParamID::wsAlgo);
        p_drive    = apvts_.getRawParameterValue(ParamID::wsDrive);
        p_wetDry   = apvts_.getRawParameterValue(ParamID::wsWetDry);
        p_bitDepth = apvts_.getRawParameterValue(ParamID::wsBitDepth);
    }

    static APVTS::ParameterLayout createLayout()
    {
        APVTS::ParameterLayout layout;
        const juce::StringArray wsAlgos = { "SoftClip","HardClip","TanhSat","BitCrush","FoldBack" };
        using Grp = juce::AudioProcessorParameterGroup;
        auto g = std::make_unique<Grp>("waveshaper", "Waveshaper", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::wsBypass,  1}, "Bypass",    true));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::wsAlgo,    1}, "Algorithm", wsAlgos, 2));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::wsDrive,   1}, "Drive",     NR(1.0f, 100.0f, 0.0f, 0.3f), 1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::wsWetDry,  1}, "Wet/Dry",   NR(0.0f, 1.0f),               1.0f));
        g->addChild(std::make_unique<juce::AudioParameterInt>   (PID{ParamID::wsBitDepth,1}, "Bit Depth", 1, 24, 8));
        layout.add(std::move(g));
        return layout;
    }

protected:
    void updateModuleParameters() override
    {
        auto* mod = static_cast<WaveshaperModule*>(module_.get());
        mod->setBypassed(*p_bypass > 0.5f);
        mod->setAlgorithm(static_cast<WaveshaperModule::Algorithm>((int)*p_algo));
        mod->setDrive    (juce::jlimit(1.0f, 100.0f, (float)*p_drive));
        mod->setWetDry   (*p_wetDry);
        mod->setBitDepth ((int)*p_bitDepth);
    }

private:
    std::atomic<float>* p_bypass   = nullptr;
    std::atomic<float>* p_algo     = nullptr;
    std::atomic<float>* p_drive    = nullptr;
    std::atomic<float>* p_wetDry   = nullptr;
    std::atomic<float>* p_bitDepth = nullptr;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new WaveshaperPlugin(); }
