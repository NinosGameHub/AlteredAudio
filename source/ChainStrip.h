#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ParameterIDs.h"

// ============================================================
//  ChainStripComponent
//  Horizontal row of 14 module slots sorted by chain position.
//  Clicking a slot fires onModuleSelected(moduleIdx).
// ============================================================
class ChainStripComponent : public juce::Component, private juce::Timer
{
public:
    // Fires with the module index (0..13) when a slot is clicked.
    std::function<void(int)> onModuleSelected;

    explicit ChainStripComponent(AlteredAudioProcessor& p);
    ~ChainStripComponent() override;

    void paint  (juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;

    void setSelectedModule(int idx);
    int  getSelectedModule() const { return selectedModule; }

    // Human-readable short name for each module index (0..13)
    static const char* moduleName(int moduleIdx);
    // APVTS position parameter ID for each module index
    static const char* modulePosParamId(int moduleIdx);
    // APVTS bypass parameter ID for each module index
    static const char* moduleBypassParamId(int moduleIdx);

    // Returns module indices sorted by their current chain positions (0=first in chain)
    std::array<int, 14> sortedOrder() const;

private:
    AlteredAudioProcessor& proc;
    int selectedModule = 0;

    void timerCallback() override;

    juce::Rectangle<float> slotBounds(int slotIdx) const;
    int  slotAtX(float x) const;
    void drawSlot(juce::Graphics&, juce::Rectangle<float> r,
                  const char* name, bool active, bool selected) const;
};
