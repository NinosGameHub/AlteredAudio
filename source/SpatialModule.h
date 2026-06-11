#pragma once
#include "EffectModule.h"

// Spatial stereo processor.
//
// Signal chain (in order):
//   1. M/S encode
//   2. Independent Mid and Side gain (smoothed)
//   3. Width — scales the Side channel (0=mono, 1=unity, 2=hyper-wide) (smoothed)
//   4. Bass Mono — single-pole LP on Side; subtracts the bass from Side so
//      low frequencies collapse to mono (avoids bass smear on speakers/clubs)
//   5. M/S decode
//   6. Stereo rotation — rotates the L/R vector by an angle (useful to
//      nudge the image or create Mid/Side at ±45°)
//   7. Balance pan — constant-power balance (smoothed)
//   8. Haas spread — delays one channel to push the apparent image (0–30 ms)
//   9. Wet/Dry (smoothed)

class SpatialModule : public EffectModule
{
public:
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Spatial"; }

    void setWidth      (float w)   { widthSmooth.setTargetValue  (juce::jlimit(0.0f,   2.0f,   w)); }
    void setPan        (float p)   { targetPan = juce::jlimit(-1.0f, 1.0f, p); }
    void setRotationDeg(float deg) { targetRotDeg = deg; }
    void setMidGainDb  (float dB)  { midSmooth.setTargetValue  (juce::Decibels::decibelsToGain(juce::jlimit(-24.0f, 24.0f, dB))); }
    void setSideGainDb (float dB)  { sideSmooth.setTargetValue (juce::Decibels::decibelsToGain(juce::jlimit(-24.0f, 24.0f, dB))); }
    void setBassMono   (float hz);
    void setHaasMs     (float ms)  { targetHaasMs = juce::jlimit(0.0f, 30.0f, ms); }
    void setWetDry     (float w)   { wetDrySmooth.setTargetValue(juce::jlimit(0.0f, 1.0f, w)); }

private:
    void updateBassLp();

    // ---- Audio thread state ----
    double sampleRate_  = 44100.0;

    // Smoothed per-sample parameters
    juce::LinearSmoothedValue<float> widthSmooth;    // target: 0–2
    juce::LinearSmoothedValue<float> midSmooth;      // target: linear gain
    juce::LinearSmoothedValue<float> sideSmooth;     // target: linear gain
    juce::LinearSmoothedValue<float> wetDrySmooth;   // target: 0–1

    // Block-rate parameters (updated once per processBlock, no per-sample ramp)
    float targetPan     = 0.0f;    // -1..+1
    float targetRotDeg  = 0.0f;    // degrees
    float targetHaasMs  = 0.0f;    // ms

    // Bass mono single-pole LP state (applied to the Side channel)
    float bassLpState   = 0.0f;
    float bassLpAlpha   = 0.0f;
    float bassMonoHz_   = 0.0f;    // 0 = disabled (< 20 Hz treated as off)

    // Haas ring buffer — delays the R channel
    static constexpr int kMaxHaasSamples = 2048;   // ~46 ms at 44.1 kHz
    float haasBuffer[kMaxHaasSamples] = {};
    int   haasWritePos = 0;
};
