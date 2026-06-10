#pragma once
#include "EffectModule.h"
#include "LFOEngine.h"
#include <vector>

class FlangerModule : public EffectModule
{
public:
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Flanger"; }

    void setRate(float hz)     { lfo.setRate(hz); }
    void setDepthMs(float ms)  { depthMs   = juce::jlimit(0.0f, 4.5f, ms); }
    void setCentreMs(float ms) { centreMs  = juce::jlimit(0.5f, 5.0f, ms); }
    void setFeedback(float f)  { feedback  = juce::jlimit(-0.95f, 0.95f, f); }
    void setWetDry(float w)    { wetDry    = juce::jlimit(0.0f, 1.0f, w); }

private:
    float readInterpolated(const std::vector<float>& buf, int writePos, float delaySamples) const;

    static constexpr int   kMaxChannels = 2;
    static constexpr float kMaxDelayMs  = 10.0f;

    std::vector<float> delayBuf[kMaxChannels];
    int   writePos[kMaxChannels] = {};
    int   bufferSize = 0;

    LFOEngine lfo;
    float centreMs  = 2.5f;
    float depthMs   = 2.0f;
    float feedback  = 0.5f;
    float wetDry    = 0.7f;
    double sampleRate_ = 44100.0;
};
