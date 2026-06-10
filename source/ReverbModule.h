#pragma once
#include "EffectModule.h"
#include <vector>
#include <cmath>

class ReverbModule : public EffectModule
{
public:
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Reverb"; }

    void setDecay      (float v) { decay      = juce::jlimit(0.0f,   1.0f,   v); }
    void setDamping    (float v) { damping    = juce::jlimit(0.0f,   1.0f,   v); }
    void setDiffusion  (float v) { diffusion  = juce::jlimit(0.0f,   1.0f,   v); }
    void setPreDelayMs (float v) { preDelayMs = juce::jlimit(0.0f, 100.0f,   v); }
    void setModRate    (float v) { modRate    = juce::jlimit(0.01f,  4.0f,   v); }
    void setErLevel    (float v) { erLevel    = juce::jlimit(0.0f,   1.0f,   v); }
    void setLowCutHz   (float v) { lowCutHz   = juce::jlimit(20.0f, 500.0f,  v); }
    void setShimmer    (float v) { shimmer    = juce::jlimit(0.0f,   1.0f,   v); }
    void setPitchSemi  (float v) { pitchSemi  = juce::jlimit(-24.0f, 24.0f,  v); }
    void setWetDry     (float v) { wetDry     = juce::jlimit(0.0f,   1.0f,   v); }

private:
    // ---------------------------------------------------------------
    struct DelayLine
    {
        std::vector<float> buf;
        int writePos = 0;

        void init (int maxDelay) { buf.assign(maxDelay + 4, 0.0f); writePos = 0; }
        void clear()             { std::fill(buf.begin(), buf.end(), 0.0f); writePos = 0; }

        void write (float v)
        {
            buf[writePos] = v;
            if (++writePos >= (int)buf.size()) writePos = 0;
        }
        float read (int delay) const
        {
            int idx = writePos - delay;
            if (idx < 0) idx += (int)buf.size();
            return buf[idx];
        }
        float readInterp (float delay) const
        {
            const int d0 = (int)delay;
            const float fr = delay - (float)d0;
            return read(d0) * (1.0f - fr) + read(d0 + 1) * fr;
        }
    };

    // Two-voice Hanning-crossfade grain pitch shifter
    struct PitchShifter
    {
        std::vector<float> buf;
        int   writePos  = 0;
        float phase     = 0.5f;   // 0..1, drives window and delay position
        float phaseInc  = 0.0f;   // (1 - pitchRatio) / grainSize
        int   grainSize = 2205;

        void init (int grain)
        {
            grainSize = std::max(grain, 64);
            buf.assign(grainSize * 2 + 8, 0.0f);
            writePos = 0;
            phase    = 0.5f;
        }
        void clear()
        {
            std::fill(buf.begin(), buf.end(), 0.0f);
            writePos = 0;
            phase    = 0.5f;
        }
        void setPitchRatio (float r)
        {
            phaseInc = (1.0f - r) / (float)grainSize;
        }
        float process (float in)
        {
            buf[writePos] = in;
            const int sz  = (int)buf.size();
            float out = 0.0f;
            for (int v = 0; v < 2; ++v)
            {
                float ph = phase + (float)v * 0.5f;
                if (ph >= 1.0f) ph -= 1.0f;
                const float delay = ph * (float)grainSize;
                const float w  = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * ph);
                const int   d0 = (int)delay;
                const float fr = delay - (float)d0;
                const int   i0 = (writePos - d0     + sz) % sz;
                const int   i1 = (writePos - d0 - 1 + sz) % sz;
                out += w * (buf[i0] * (1.0f - fr) + buf[i1] * fr);
            }
            if (++writePos >= sz) writePos = 0;
            phase += phaseInc;
            if (phase >  1.0f) phase -= 1.0f;
            if (phase <  0.0f) phase += 1.0f;
            return out;
        }
    };
    // ---------------------------------------------------------------

    static float processAP (float in, float g, DelayLine& dl, float delay);

    static constexpr double kRefRate = 29761.0;
    static constexpr int kD1 = 142, kD2 = 107, kD3 = 379, kD4 = 277;
    static constexpr int kAPA1 = 672,  kDA1 = 4453, kAPA2 = 1800, kDA2 = 3720;
    static constexpr int kAPB1 = 908,  kDB1 = 4217, kAPB2 = 2656, kDB2 = 3163;
    static constexpr float kModDepthRef = 16.0f;

    // ER tap times in ms — L/R use prime-spaced values for stereo decorrelation
    static constexpr float kErMsL[8] = {  7.f, 13.f, 23.f, 37.f, 53.f, 67.f, 83.f,  97.f };
    static constexpr float kErMsR[8] = { 11.f, 17.f, 29.f, 41.f, 59.f, 71.f, 89.f, 107.f };
    static constexpr float kErGains[8] = { 1.0f, 0.87f, 0.75f, 0.65f, 0.55f, 0.48f, 0.40f, 0.35f };

    // Diffusion allpasses (shared mono)
    DelayLine diff[4];
    // Tank A
    DelayLine apA1, delA1, apA2, delA2;
    // Tank B
    DelayLine apB1, delB1, apB2, delB2;
    // True-stereo pre-delays
    DelayLine preDelL, preDelR;
    // Early reflections buffer (mono)
    DelayLine erBuf;

    // Filter state
    float dampA = 0.0f, dampB = 0.0f;   // LP damping per tank
    float hpLpA = 0.0f, hpLpB = 0.0f;  // low-cut HP internal LP state

    float lfoPhase = 0.0f;

    PitchShifter psA, psB;   // shimmer, one per tank cross-feed

    float decay      = 0.5f;
    float damping    = 0.5f;
    float diffusion  = 0.75f;
    float preDelayMs = 20.0f;
    float modRate    = 0.5f;
    float erLevel    = 0.3f;
    float lowCutHz   = 80.0f;
    float shimmer    = 0.0f;
    float pitchSemi  = 12.0f;
    float wetDry     = 0.3f;
    double sr        = 44100.0;

    int scale (int n) const { return juce::roundToInt(n * sr / kRefRate); }
};
