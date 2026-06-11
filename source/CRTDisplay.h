#pragma once
#include <JuceHeader.h>

// ============================================================
//  CRTDisplay
//  Animated amber spectrum analyzer with scan-line overlay.
//  Pure JUCE Graphics — no OpenGL. 30fps via Timer.
// ============================================================
class CRTDisplay : public juce::Component, private juce::Timer
{
public:
    CRTDisplay();
    ~CRTDisplay() override;

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int kNumBars = 32;
    static constexpr int kFps     = 30;

    float barHeights[kNumBars] {};
    float barTargets[kNumBars] {};
    float driftPhase[kNumBars] {};

    juce::Random rng;

    void timerCallback() override;
    void initBars();

    static float baseShape(int bin);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CRTDisplay)
};
