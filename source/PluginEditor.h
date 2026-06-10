#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AaLookAndFeel.h"
#include "ChainStrip.h"
#include "ModulePanels.h"

class AlteredAudioEditor : public juce::AudioProcessorEditor
{
public:
    explicit AlteredAudioEditor(AlteredAudioProcessor&);
    ~AlteredAudioEditor() override;

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    AlteredAudioProcessor& processorRef;

    AaLookAndFeel laf;

    // ---- Header ----
    juce::Label titleLabel;

    // ---- Chain strip ----
    ChainStripComponent chainStrip;

    // ---- Detail panel (swapped when a slot is clicked) ----
    std::unique_ptr<ModulePanel> currentPanel;

    void showModulePanel(int moduleIdx);

    static constexpr int kHeaderH     = 48;
    static constexpr int kStripH      = 80;
    static constexpr int kWindowW     = 1000;
    static constexpr int kWindowH     = 620;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlteredAudioEditor)
};
