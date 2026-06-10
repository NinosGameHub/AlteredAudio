#pragma once
#include "EffectModule.h"

class WaveshaperModule : public EffectModule
{
public:
    enum class Algorithm { SoftClip, HardClip, TanhSat, BitCrush, FoldBack };

    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Waveshaper"; }

    void setAlgorithm(Algorithm alg) { algorithm = alg; }
    void setDrive(float d)           { drive = juce::jlimit(1.0f, 100.0f, d); }
    void setWetDry(float w)          { wetDry = juce::jlimit(0.0f, 1.0f, w); }
    void setBitDepth(int bits)       { bitDepth = juce::jlimit(1, 24, bits); }

    int getLatencySamples() const { return (int)oversampling.getLatencyInSamples(); }

private:
    float processSample(float input) const;

    Algorithm algorithm  = Algorithm::TanhSat;
    float     drive      = 1.0f;
    float     wetDry     = 1.0f;
    int       bitDepth   = 8;

    // 2x oversampling (factor = 1 means 2^1 = 2x)
    juce::dsp::Oversampling<float> oversampling{
        2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };

    juce::AudioBuffer<float> dryBuffer;
};
