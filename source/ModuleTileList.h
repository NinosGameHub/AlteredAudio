#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ParameterIDs.h"
#include <numeric>
#include <vector>

// ============================================================
//  ModuleTileList
//  Vertical signal-chain list (left panel, ~160px wide).
//  Shows 15 numbered tiles with category LED and colored strip.
//  Drag tiles to reorder. Clicking fires onModuleSelected with
//  the logical module index (from moduleOrder[displayPos]).
// ============================================================
class ModuleTileList : public juce::Component, private juce::Timer
{
public:
    std::function<void(int)>                     onModuleSelected;
    std::function<void(const std::vector<int>&)> onOrderChanged;

    explicit ModuleTileList(AlteredAudioProcessor&);
    ~ModuleTileList() override;

    void paint    (juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp  (const juce::MouseEvent&) override;

    // logicalIdx is the module ID, not a display position
    void setSelectedModule(int logicalIdx);
    int  getSelectedModule() const { return selectedModule; }

    const std::vector<int>& getModuleOrder() const { return moduleOrder; }

    static constexpr int kNumModules = 15;

    static const char*   moduleName       (int idx);
    static juce::Colour  moduleColor      (int idx);
    static const char*   moduleBypassParam(int idx);
    static const char*   modulePosParam   (int idx);

private:
    AlteredAudioProcessor& proc;
    int selectedModule = 7;   // logical module index

    std::vector<int> moduleOrder;  // displayPos -> logicalIdx

    // Drag state (-1 = inactive)
    int  dragSourcePos = -1;
    int  dragTargetPos = -1;
    bool isDragging    = false;

    void timerCallback() override;

    juce::Rectangle<int> tileBounds(int displayPos) const;
    int  tileAtY(int y) const;

    void drawTile(juce::Graphics&, int displayPos,
                  bool active, bool selected,
                  bool dragging, bool dropTarget) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleTileList)
};
