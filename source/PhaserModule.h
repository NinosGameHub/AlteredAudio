#pragma once
#include "EffectModule.h"
#include "LFOEngine.h"

class PhaserModule : public EffectModule
{
public:
    static constexpr int kMaxStages = 8;

    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Phaser"; }

    void setRate(float hz)       { lfo.setRate(hz); }
    void setDepth(float d)       { depth    = juce::jlimit(0.0f, 1.0f, d); }
    void setBaseFreq(float hz)   { baseFreq = juce::jlimit(50.0f, 8000.0f, hz); }
    void setFeedback(float f)    { feedback = juce::jlimit(-0.95f, 0.95f, f); }
    void setNumStages(int n)     { numStages = juce::jlimit(2, kMaxStages, n & ~1); } // even only
    void setWetDry(float w)      { wetDry   = juce::jlimit(0.0f, 1.0f, w); }

private:
    static constexpr int kMaxChannels = 2;

    // First-order all-pass state: y[n] = c*(x[n] - y[n-1]) + x[n-1]
    float apState[kMaxStages][kMaxChannels] = {};
    float feedbackSample[kMaxChannels]      = {};

    LFOEngine lfo;
    float depth     = 1.0f;
    float baseFreq  = 500.0f;
    float feedback  = 0.7f;
    float wetDry    = 0.7f;
    int   numStages = 4;
    double sampleRate_ = 44100.0;
};
