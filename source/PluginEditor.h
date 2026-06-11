#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AaLookAndFeel.h"
#include "ModuleTileList.h"
#include "CRTDisplay.h"
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

    juce::Label    titleLabel;
    ModuleTileList moduleTileList;
    CRTDisplay     crtDisplay;

    std::unique_ptr<ModulePanel> currentPanel;

    void showModulePanel(int moduleIdx);
    juce::Rectangle<int> getPanelBounds() const;

    // Layout constants
    static constexpr int kHeaderH  = 40;
    static constexpr int kListW    = 160;
    static constexpr int kCRTH     = 180;
    static constexpr int kWindowW  = 1000;
    static constexpr int kWindowH  = 640;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlteredAudioEditor)
};
