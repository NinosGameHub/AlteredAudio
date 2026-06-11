#pragma once
#include "EffectModule.h"
#include <vector>
#include <cmath>

class DelayModule : public EffectModule
{
public:
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Delay"; }

    void setDelayTimesMs(float msL, float msR)
    {
        delayMsL = juce::jlimit(0.0f, kMaxDelayMs, msL);
        delayMsR = juce::jlimit(0.0f, kMaxDelayMs, msR);
        timeSmL.setTargetValue(delayMsL);
        timeSmR.setTargetValue(delayMsR);
    }
    void setFeedback    (float f)  { feedback    = juce::jlimit(0.0f, 0.99f, f); }
    void setWetDry      (float w)  { wetDry      = juce::jlimit(0.0f, 1.0f,  w); }
    void setPingPong    (bool pp)  { pingPong    = pp; }

    // ---- Tempo sync — shared by DSP and UI ----
    static const juce::StringArray& syncDivisionNames()
    {
        static const juce::StringArray names {
            "1/1","1/2","1/2D","1/2T","1/4","1/4D","1/4T",
            "1/8","1/8D","1/8T","1/16","1/16D","1/16T","1/32" };
        return names;
    }
    static float syncDivisionToMs(int divIndex, double bpm)
    {
        // Length in quarter notes; D = dotted (x1.5), T = triplet (x2/3)
        static constexpr float beats[] = {
            4.0f, 2.0f, 3.0f, 4.0f/3.0f, 1.0f, 1.5f, 2.0f/3.0f,
            0.5f, 0.75f, 1.0f/3.0f, 0.25f, 0.375f, 1.0f/6.0f, 0.125f };
        const int i = juce::jlimit(0, 13, divIndex);
        const double quarterMs = 60000.0 / juce::jlimit(20.0, 999.0, bpm);
        return (float)(beats[i] * quarterMs);
    }
    void setFbLPHz      (float hz) { fbLPHz      = juce::jlimit(500.0f, 20000.0f, hz); }
    void setFbHPHz      (float hz) { fbHPHz      = juce::jlimit(20.0f,  500.0f,   hz); }
    void setDucking     (float d)  { ducking     = juce::jlimit(0.0f, 1.0f,  d); }
    void setDiffusion   (float d)  { diffusion   = juce::jlimit(0.0f, 1.0f,  d); }
    void setWowFlutter  (float w)  { wowFlutter  = juce::jlimit(0.0f, 1.0f,  w); }
    void setModRate     (float hz) { modRate     = juce::jlimit(0.01f, 5.0f, hz); }
    void setModDepth    (float d)  { modDepth    = juce::jlimit(0.0f, 1.0f,  d); }

private:
    // Schroeder allpass used for echo diffusion
    struct APF
    {
        std::vector<float> buf;
        int wp = 0;
        void init  (int sz) { buf.assign(sz + 1, 0.0f); wp = 0; }
        void clear ()       { std::fill(buf.begin(), buf.end(), 0.0f); wp = 0; }
        float process (float x, float g)
        {
            const float dN = buf[wp];
            const float d  = x - g * dN;
            buf[wp] = d;
            if (++wp >= (int)buf.size()) wp = 0;
            return g * d + dN;
        }
    };

    // Hermite cubic interpolation for smooth modulated reads
    float readSample (int ch, float delaySamples) const;
    void  writeSample(int ch, float sample);

    // Padé-approximate tanh — accurate to ±0.5% for |x| < 4
    static float tanhApprox (float x) noexcept
    {
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    static constexpr int   kMaxChannels = 2;
    static constexpr float kMaxDelayMs  = 2500.0f;  // room for spread
    static constexpr int   kDiffStages  = 4;
    // Diffusion AP delay lengths in ms (prime-ish, avoid harmonic ratios)
    static constexpr float kDiffMs[kDiffStages] = { 5.3f, 10.7f, 17.2f, 26.1f };

    std::vector<float> delayBuffer[kMaxChannels];
    int   writePos[kMaxChannels] = {};
    int   bufferSize = 0;

    // Diffusion allpass filters: [stage][channel]
    APF diffAP[kDiffStages][kMaxChannels];

    // Per-channel feedback filter states
    float lpState [kMaxChannels] = {};   // tone LP
    float hpLpSt  [kMaxChannels] = {};   // tone HP internal LP

    // LFO / tape state
    float lfoPhase     = 0.0f;
    float wowPhase     = 0.0f;
    float flutterPhase = 0.0f;

    // Ducking envelope follower
    float duckEnv = 0.0f;

    // Smoothed per-channel delay times (ms) — glide like tape instead of clicking
    juce::SmoothedValue<float> timeSmL { 250.0f };
    juce::SmoothedValue<float> timeSmR { 250.0f };

    // Parameters
    float delayMsL    = 250.0f;
    float delayMsR    = 250.0f;
    float feedback    = 0.3f;
    float wetDry      = 0.5f;
    bool  pingPong    = false;
    float fbLPHz      = 6000.0f;
    float fbHPHz      = 80.0f;
    float ducking     = 0.0f;
    float diffusion   = 0.0f;
    float wowFlutter  = 0.0f;
    float modRate     = 1.0f;
    float modDepth    = 0.0f;
    double sr_        = 44100.0;
};
