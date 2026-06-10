#pragma once
#include "EffectModule.h"

class TransientShaperModule : public EffectModule
{
public:
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Transient Shaper"; }

    void setAttackGainDb (float dB) { attackGainDb  = juce::jlimit(-24.0f, 24.0f, dB); }
    void setSustainGainDb(float dB) { sustainGainDb = juce::jlimit(-24.0f, 24.0f, dB); }
    void setSpeed        (float v)  { speed        = juce::jlimit(0.0f, 1.0f, v); }
    void setStereoLink   (float v)  { stereoLink   = juce::jlimit(0.0f, 1.0f, v); }
    void setWetDry       (float v)  { wetDry       = juce::jlimit(0.0f, 1.0f, v); }

private:
    static constexpr int kMaxChannels = 2;

    // Parameters
    float attackGainDb  =  0.0f;
    float sustainGainDb =  0.0f;
    float speed         =  0.5f;  // 0=slow/smooth, 1=fast/snappy
    float stereoLink    =  1.0f;
    float wetDry        =  1.0f;

    // Per-channel state
    float envFast   [kMaxChannels] = {};
    float envSlow   [kMaxChannels] = {};
    float gainSmooth[kMaxChannels] = { 1.0f, 1.0f };

    double sampleRate_ = 44100.0;
};
