#pragma once
#include <JuceHeader.h>

class BiquadFilter
{
public:
    enum class Type { LowPass, HighPass, BandPass, Notch, AllPass, Peak, LowShelf, HighShelf,
                      LowPass1, HighPass1 };   // first-order variants (6 dB/oct sections)

    void prepare(double sampleRate);
    void reset();
    float processSample(float input, int channel);

    // Change-detection: only flag a coefficient recompute when the value moves.
    void setType(Type t)        { if (type != t)       { type = t;       needsUpdate = true; } }
    void setFrequency(float hz) { if (frequency != hz) { frequency = hz; needsUpdate = true; } }
    void setQ(float q_)         { if (q != q_)         { q = q_;         needsUpdate = true; } }
    void setGainDb(float dB)    { if (gainDb != dB)    { gainDb = dB;    needsUpdate = true; } }

    // Shared coefficient math — the UI uses this to draw the exact response
    // the audio path produces. out = { b0, b1, b2, a1, a2 }, normalized by a0.
    static void calcCoefficients(Type, double sampleRate,
                                 float freq, float q, float gainDb, double out[5]);

    // Exact magnitude of the transfer function at freqHz, in dB.
    static double magnitudeDb(const double coeffs[5], double freqHz, double sampleRate);

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
