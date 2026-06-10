#pragma once
#include "EffectModule.h"
#include <vector>
#include <cmath>

class CompressorModule : public EffectModule
{
public:
    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Compressor"; }

    // Original params
    void setThresholdDb  (float dB) { thresholdDb  = dB; }
    void setRatio        (float r)  { ratio        = juce::jlimit(1.0f, 100.0f, r); }
    void setAttackMs     (float ms) { if (ms != attackMs)  { attackMs  = ms; updateCoeffs(); } }
    void setReleaseMs    (float ms) { if (ms != releaseMs) { releaseMs = ms; updateCoeffs(); } }
    void setKneeDb       (float dB) { kneeDb       = juce::jlimit(0.0f, 24.0f, dB); }
    void setMakeupGainDb (float dB) { makeupGainDb = dB; }

    // Option 1: Analog character
    void setRmsPeak    (float v)  { rmsPeak     = juce::jlimit(0.0f, 1.0f, v); }
    void setAutoRelease(float v)  { autoRelease = juce::jlimit(0.0f, 1.0f, v); }
    void setSaturation (float v)  { saturation  = juce::jlimit(0.0f, 1.0f, v); }

    // Option 2: Sidechain detection shaping
    void setScHPHz    (float hz) { scHPHz     = juce::jlimit(20.0f, 500.0f, hz); }
    void setScLPHz    (float hz) { scLPHz     = juce::jlimit(2000.0f, 20000.0f, hz); }
    void setStereoLink(float v)  { stereoLink = juce::jlimit(0.0f, 1.0f, v); }

    // Option 3: Parallel / New York compression
    void setMix(float v) { mix = juce::jlimit(0.0f, 1.0f, v); }

    // Option 4: Lookahead
    void setLookaheadMs(float ms) { lookaheadMs = juce::jlimit(0.0f, 10.0f, ms); }

    float getGainReductionDb() const { return gainReductionDb; }

protected:
    float computeGainDb(float levelDb) const;

    static constexpr int   kMaxChannels    = 2;
    static constexpr float kMaxLookaheadMs = 10.0f;
    static constexpr float kRmsTimeMs      = 20.0f;  // fixed RMS integration window

    // Original
    float thresholdDb  = -12.0f;
    float ratio        =   4.0f;
    float attackMs     =  10.0f;
    float releaseMs    = 100.0f;
    float kneeDb       =   6.0f;
    float makeupGainDb =   0.0f;

    // New
    float rmsPeak      =  0.0f;     // 0=peak, 1=RMS blend
    float autoRelease  =  0.0f;     // program-dependent release amount
    float saturation   =  0.0f;     // output saturation drive
    float scHPHz       = 20.0f;     // sidechain HP cutoff
    float scLPHz       = 20000.0f;  // sidechain LP cutoff
    float stereoLink   =  1.0f;     // 0=dual-mono, 1=fully-linked
    float mix          =  1.0f;     // parallel mix: 0=dry, 1=compressed
    float lookaheadMs  =  0.0f;

    float attackCoeff  = 0.0f;
    float releaseCoeff = 0.0f;
    float gainReductionDb = 0.0f;
    double sampleRate_ = 44100.0;

    // Per-channel state
    float smoothedGain_[kMaxChannels] = { 1.0f, 1.0f };
    float rmsState     [kMaxChannels] = {};
    float scHpState    [kMaxChannels] = {};
    float scLpState    [kMaxChannels] = {};

    // Lookahead circular buffers
    std::vector<float> lookaheadBuf[kMaxChannels];
    int lookaheadWritePos[kMaxChannels] = {};
    int lookaheadBufSize = 0;

    void updateCoeffs();

    static float tanhApprox(float x) noexcept
    {
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }
};
