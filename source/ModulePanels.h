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
    Combo typeCombo, slopeCombo;
    Knob  freqKnob, qKnob, gainKnob, driveKnob, mixKnob, outputKnob;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterPanel)
};

// ============================================================
//  EQCurveDisplay — interactive log-scale frequency response
// ============================================================
class EQCurveDisplay : public juce::Component, private juce::Timer
{
public:
    explicit EQCurveDisplay(AlteredAudioProcessor& p);
    void paint(juce::Graphics&) override;

    void setSelectedBand(int band);
    int  getSelectedBand() const { return selectedBand; }

    static juce::Colour bandColor(int bandIdx);

    std::function<void()> onDataChanged;

private:
    AlteredAudioProcessor& proc;
    struct BandData { bool on = false; int type = 5; float freq = 1000.f, q = 0.7f, gain = 0.f; int slope = 1; };
    BandData bands[EQModule::kMaxBands];
    int  selectedBand = 0;

    float computeResponseDb  (float freqHz) const;
    float computeSingleBandDb(int bandIdx, float freqHz) const;
    void  timerCallback() override;

    static constexpr float FMIN = 20.f, FMAX = 20000.f, GMAX = 18.f;
    static constexpr float padL = 8.f, padR = 8.f, padT = 10.f, padB = 18.f;
    float xForF(float f,  float w) const;
    float yForG(float db, float h) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQCurveDisplay)
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
        Combo              slopeCombo;
        Knob               freqKnob, qKnob, gainKnob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttach;
    };
    EqBand           bands[kBands];
    EQCurveDisplay   curveDisplay;
    juce::TextButton bandBtns[kBands];

    void showBand(int idx);
    void updateBandButtons();
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
    Knob  timeLKnob, timeRKnob, feedbackKnob, wetDryKnob;
    Knob  fbLPKnob, fbHPKnob, duckingKnob, diffusionKnob;
    Knob  wowKnob, modRateKnob, modDepthKnob;
    Combo divLCombo, divRCombo;   // shown instead of time knobs when synced
    juce::ToggleButton syncBtn     { "Sync" };
    juce::ToggleButton pingPongBtn { "Ping-Pong" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttach, pingPongAttach;
    void updateTimeControls();
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
//  GRMeter — horizontal gain-reduction bar for dynamics modules
// ============================================================
class GRMeter : public juce::Component, private juce::Timer
{
public:
    GRMeter();
    void setSource(std::function<float()> fn);
    void setScale (float maxGrDb);   // default 18dB; use 12 for Limiter
    void paint(juce::Graphics&) override;

    static constexpr int kHeight = 42;  // total component height to allocate
private:
    std::function<float()> getGRFn;
    float maxGrDb      = 18.0f;
    float currentDb    = 0.0f;
    float peakDb       = 0.0f;
    float peakHoldSecs = 0.0f;
    void  timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GRMeter)
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
    GRMeter grMeter;
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
    Knob    ceilingKnob, releaseKnob;
    GRMeter grMeter;
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
