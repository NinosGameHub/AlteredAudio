#pragma once
#include "EffectModule.h"
#include "BiquadFilter.h"

class EQModule : public EffectModule
{
public:
    static constexpr int kMaxBands = 8;

    struct BandParams
    {
        BiquadFilter::Type type = BiquadFilter::Type::Peak;
        float frequency = 1000.0f;
        float q         = 0.707f;
        float gainDb    = 0.0f;
        bool  enabled   = false;
    };

    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "EQ"; }

    void setBand(int index, const BandParams& p);
    void setNumBands(int n) { numBands = juce::jlimit(0, kMaxBands, n); }
    int  getNumBands() const { return numBands; }

private:
    static constexpr int kMaxChannels = 2;
    BiquadFilter bands[kMaxBands][kMaxChannels];
    BandParams   params[kMaxBands];
    int          numBands = 0;
    double       currentSampleRate = 44100.0;
};
