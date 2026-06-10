#pragma once
#include "EffectModule.h"
#include "LFOEngine.h"
#include <vector>

class ChorusModule : public EffectModule
{
public:
    static constexpr int kMaxVoices = 4;

    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Chorus"; }

    void setRate(float hz)      { for (auto& l : lfos) l.setRate(hz); }
    void setDepthMs(float ms)   { depthMs  = juce::jlimit(0.0f, 20.0f, ms); }
    void setCentreMs(float ms)  { centreMs = juce::jlimit(1.0f, 30.0f, ms); }
    void setNumVoices(int v)    { numVoices = juce::jlimit(1, kMaxVoices, v); }
    void setWetDry(float w)     { wetDry = juce::jlimit(0.0f, 1.0f, w); }

private:
    float readInterpolated(const std::vector<float>& buf, int writePos, float delaySamples) const;

    static constexpr int   kMaxChannels   = 2;
    static constexpr float kMaxDelayMs    = 40.0f;

    std::vector<float> delayBuf[kMaxChannels];
    int   writePos[kMaxChannels] = {};
    int   bufferSize = 0;

    LFOEngine lfos[kMaxVoices];

    float  centreMs  = 10.0f;
    float  depthMs   =  3.0f;
    float  wetDry    =  0.5f;
    int    numVoices = 2;
    double sampleRate_ = 44100.0;
};
