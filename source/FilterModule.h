#pragma once
#include "EffectModule.h"
#include "BiquadFilter.h"

class FilterModule : public EffectModule
{
public:
    using FilterType = BiquadFilter::Type;

    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Filter"; }

    // All continuous parameters are smoothed inside process() — callers can
    // update at block rate (or from LFOs) without coefficient-step clicks.
    void setType(FilterType t)  { type = t; }
    void setSlope(int s)        { slope = juce::jlimit(0, 1, s); }
    void setFrequency(float hz) { freqSm.setTargetValue(juce::jlimit(20.0f, 20000.0f, hz)); }
    void setQ(float q_)         { qSm.setTargetValue(juce::jlimit(0.05f, 24.0f, q_)); }
    void setGainDb(float dB)    { gainSm.setTargetValue(juce::jlimit(-24.0f, 24.0f, dB)); }
    void setDriveDb(float dB)   { driveSm.setTargetValue(juce::jlimit(0.0f, 24.0f, dB)); }
    void setMix(float m)        { mixSm.setTargetValue(juce::jlimit(0.0f, 1.0f, m)); }
    void setOutputDb(float dB)  { outSm.setTargetValue(
                                      juce::Decibels::decibelsToGain(juce::jlimit(-24.0f, 24.0f, dB))); }

    // Exact magnitude response in dB — single source of truth for the UI curve.
    static float responseDb(FilterType type, int slope, float freq, float q,
                            float gainDb, float atFreqHz, double sampleRate);

private:
    struct Section { FilterType type; float q; };
    static int planSections(FilterType, int slope, float q, Section out[2]);

    // Re-plans and updates section coefficients from smoothed control values.
    void applyControl(float freq, float q, float gainDb);

    static float tanhApprox(float x) noexcept
    {
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    static constexpr int kMaxChannels   = 2;
    static constexpr int kMaxSections   = 2;    // 24 dB/oct = 2 biquads
    static constexpr int kCtrlInterval  = 16;   // coefficient update granularity (samples)

    BiquadFilter sections[kMaxSections];        // each holds stereo state
    int          numSections = 1;

    FilterType type  = FilterType::LowPass;
    int        slope = 0;                       // 0 = 12 dB/oct, 1 = 24 dB/oct

    // Frequency glides multiplicatively (log-domain) — equal musical speed
    // across the range; the rest are linear.
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> freqSm { 1000.0f };
    juce::SmoothedValue<float> qSm     { 0.707f };
    juce::SmoothedValue<float> gainSm  { 0.0f };
    juce::SmoothedValue<float> driveSm { 0.0f };
    juce::SmoothedValue<float> mixSm   { 1.0f };
    juce::SmoothedValue<float> outSm   { 1.0f };

    juce::AudioBuffer<float> dryBuffer;
};
