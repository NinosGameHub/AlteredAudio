#pragma once
#include "EffectModule.h"

class GainModule : public EffectModule
{
public:
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Gain"; }

    void setGainDb(float db);

private:
    juce::LinearSmoothedValue<float> smoother;
    float targetLinear = 1.0f;
};
