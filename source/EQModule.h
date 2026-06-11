#pragma once
#include "EffectModule.h"
#include "BiquadFilter.h"

class EQModule : public EffectModule
{
public:
    static constexpr int kMaxBands    = 8;
    static constexpr int kMaxSections = 4;   // 48 dB/oct = 8th order = 4 biquads

    struct BandParams
    {
        BiquadFilter::Type type = BiquadFilter::Type::Peak;
        float frequency = 1000.0f;
        float q         = 0.707f;
        float gainDb    = 0.0f;
        int   slope     = 1;      // 0..4 → 6/12/18/24/48 dB/oct (LP/HP only)
        bool  enabled   = false;
    };

    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "EQ"; }

    void setBand(int index, const BandParams& p);

    // Exact magnitude response of one band in dB — single source of truth,
    // used by both the audio path (via the same section plan) and the UI curve.
    static float bandResponseDb(const BandParams&, float freqHz, double sampleRate);

private:
    struct Section { BiquadFilter::Type type; float q; };

    // Translates BandParams into a Butterworth cascade. Returns section count.
    static int planSections(const BandParams&, Section out[kMaxSections]);

    static constexpr int kMaxChannels = 2;
    BiquadFilter sections[kMaxBands][kMaxSections];   // each filter holds stereo state
    int          numSections[kMaxBands] = {};
    BandParams   params[kMaxBands];
    double       currentSampleRate = 44100.0;
};
