#pragma once
#include "EffectModule.h"

class GateModule : public EffectModule
{
public:
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Gate"; }

    void setThresholdDb(float dB) { thresholdDb = dB; }
    void setAttackMs(float ms)    { if (ms != attackMs)  { attackMs  = ms; updateCoeffs(); } }
    void setReleaseMs(float ms)   { if (ms != releaseMs) { releaseMs = ms; updateCoeffs(); } }
    void setRangeDb(float dB)     { rangeDb = juce::jlimit(-96.0f, 0.0f, dB); }

private:
    void updateCoeffs();

    float thresholdDb = -40.0f;
    float attackMs    =   1.0f;
    float releaseMs   = 100.0f;
    float rangeDb     = -60.0f;

    float attackCoeff  = 0.0f;
    float releaseCoeff = 0.0f;
    float smoothedGain = 1.0f;
    double sampleRate_ = 44100.0;
};
