#pragma once
#include <JuceHeader.h>

class BiquadFilter
{
public:
    enum class Type { LowPass, HighPass, BandPass, Notch, AllPass, Peak, LowShelf, HighShelf };

    void prepare(double sampleRate);
    void reset();
    float processSample(float input, int channel);

    void setType(Type t)        { type = t; needsUpdate = true; }
    void setFrequency(float hz) { frequency = hz; needsUpdate = true; }
    void setQ(float q_)         { q = q_; needsUpdate = true; }
    void setGainDb(float dB)    { gainDb = dB; needsUpdate = true; }

private:
    void updateCoefficients();

    Type  type      = Type::LowPass;
    float frequency = 1000.0f;
    float q         = 0.707f;
    float gainDb    = 0.0f;
    double sampleRate = 44100.0;

    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;

    static constexpr int kMaxChannels = 2;
    double z1[kMaxChannels] = {};
    double z2[kMaxChannels] = {};

    bool needsUpdate = true;
};
