#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ParameterIDs.h"

// ============================================================
//  ModulePanel   — base class for all module detail panels
// ============================================================
class ModulePanel : public juce::Component, private juce::Timer
{
public:
    ModulePanel(AlteredAudioProcessor& p,
                const juce::String&   name,
                const juce::String&   bypassParamId);
    ~ModulePanel() override;

    void paint  (juce::Graphics&) override;
    void resized() override;

    static constexpr int kHeaderH = 44;

    struct Knob
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;

        Knob() : slider(juce::Slider::RotaryHorizontalVerticalDrag,
                        juce::Slider::TextBoxBelow) {}
    };

    struct Combo
    {
        juce::ComboBox box;
        juce::Label    label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attach;
    };

protected:
    AlteredAudioProcessor& proc;
    juce::String            moduleName;
    juce::String            bypassParamId;

    juce::ToggleButton activeBtn { "Active" };

    void makeKnob (Knob&,  const juce::String& paramId, const juce::String& labelText);
    void makeCombo(Combo&, const juce::String& paramId, const juce::String& labelText,
                   const juce::StringArray& choices);

    juce::Rectangle<int> contentArea() const;

private:
    void timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulePanel)
};

// ============================================================
//  StubPanel — for modules not yet fully implemented
// ============================================================
class StubPanel : public ModulePanel
{
public:
    StubPanel(AlteredAudioProcessor& p,
              const juce::String& name,
              const juce::String& bypassParamId);
    void paint(juce::Graphics&) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StubPanel)
};

// ============================================================
//  FilterPanel
// ============================================================
class FilterPanel : public ModulePanel
{
public:
    explicit FilterPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Combo typeCombo;
    Knob  freqKnob, qKnob, gainKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterPanel)
};

// ============================================================
//  EQPanel
// ============================================================
class EQPanel : public ModulePanel
{
public:
    explicit EQPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    static constexpr int kBands = EQModule::kMaxBands;
    struct EqBand
    {
        juce::ToggleButton enableBtn;
        Combo              typeCombo;
        Knob               freqKnob, qKnob, gainKnob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttach;
    };
    EqBand bands[kBands];
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQPanel)
};

// ============================================================
//  GainPanel
// ============================================================
class GainPanel : public ModulePanel
{
public:
    explicit GainPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob gainKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainPanel)
};

// ============================================================
//  DelayPanel
// ============================================================
class DelayPanel : public ModulePanel
{
public:
    explicit DelayPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob timeKnob, feedbackKnob, spreadKnob, wetDryKnob;
    Knob fbLPKnob, fbHPKnob, duckingKnob, diffusionKnob;
    Knob wowKnob, modRateKnob, modDepthKnob;
    juce::ToggleButton pingPongBtn { "Ping-Pong" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pingPongAttach;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayPanel)
};

// ============================================================
//  ReverbPanel
// ============================================================
class ReverbPanel : public ModulePanel
{
public:
    explicit ReverbPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob decayKnob, dampKnob, diffusionKnob, preDelayKnob, wetDryKnob;
    Knob modRateKnob, erLevelKnob, lowCutKnob, shimmerKnob, pitchSemiKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbPanel)
};

// ============================================================
//  WaveshaperPanel
// ============================================================
class WaveshaperPanel : public ModulePanel
{
public:
    explicit WaveshaperPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Combo algoCombo;
    Knob  driveKnob, wetDryKnob, bitDepthKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveshaperPanel)
};

// ============================================================
//  CompressorPanel
// ============================================================
class CompressorPanel : public ModulePanel
{
public:
    explicit CompressorPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob threshKnob, ratioKnob, attackKnob, releaseKnob, kneeKnob, makeupKnob, rmsPeakKnob;
    Knob autoRelKnob, satKnob, scHPKnob, scLPKnob, stereoLinkKnob, mixKnob, lookaheadKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorPanel)
};

// ============================================================
//  TransientShaperPanel
// ============================================================
class TransientShaperPanel : public ModulePanel
{
public:
    explicit TransientShaperPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob attackKnob, sustainKnob, speedKnob, linkKnob, wetDryKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransientShaperPanel)
};

// ============================================================
//  LimiterPanel
// ============================================================
class LimiterPanel : public ModulePanel
{
public:
    explicit LimiterPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob ceilingKnob, releaseKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LimiterPanel)
};

// ============================================================
//  GatePanel
// ============================================================
class GatePanel : public ModulePanel
{
public:
    explicit GatePanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob threshKnob, attackKnob, releaseKnob, rangeKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GatePanel)
};

// ============================================================
//  ChorusPanel
// ============================================================
class ChorusPanel : public ModulePanel
{
public:
    explicit ChorusPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob rateKnob, depthKnob, centreKnob, voicesKnob, wetDryKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusPanel)
};

// ============================================================
//  FlangerPanel
// ============================================================
class FlangerPanel : public ModulePanel
{
public:
    explicit FlangerPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob rateKnob, depthKnob, centreKnob, feedbackKnob, wetDryKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlangerPanel)
};

// ============================================================
//  PhaserPanel
// ============================================================
class PhaserPanel : public ModulePanel
{
public:
    explicit PhaserPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Combo stagesCombo;
    Knob  rateKnob, depthKnob, baseFreqKnob, feedbackKnob, wetDryKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserPanel)
};

// ============================================================
//  ClipperPanel
// ============================================================
class ClipperPanel : public ModulePanel
{
public:
    explicit ClipperPanel(AlteredAudioProcessor&);
    void resized() override;

private:
    Knob  ceilingKnob, kneeKnob, mixKnob, biasKnob, emphFreqKnob, emphGainKnob;
    Combo modeCombo, osCombo;

    juce::ToggleButton autoGainBtn { "Auto Gain" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoGainAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipperPanel)
};

// ============================================================
//  SpatialPanel
// ============================================================
class SpatialPanel : public ModulePanel
{
public:
    explicit SpatialPanel(AlteredAudioProcessor&);
    void resized() override;
private:
    Knob widthKnob, panKnob, wetDryKnob;
    Knob rotationKnob, midGainKnob, sideGainKnob, bassMonoKnob, haasKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpatialPanel)
};

// ============================================================
//  Panel factory — returns the right panel for a module index
// ============================================================
std::unique_ptr<ModulePanel> createModulePanel(AlteredAudioProcessor& proc, int moduleIdx);
