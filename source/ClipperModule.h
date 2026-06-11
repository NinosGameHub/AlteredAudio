#pragma once
#include "EffectModule.h"
#include <memory>

class ClipperModule : public EffectModule
{
public:
    enum class Mode { Hard = 0, Soft, Sine, Tape };

    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Clipper"; }

    void setMode       (Mode m)    { mode             = m; }
    void setCeilingDb  (float dB)  { ceilingDb        = juce::jlimit(-24.0f,  0.0f,     dB); }
    void setMix        (float v)   { mix              = juce::jlimit( 0.0f,   1.0f,      v); }
    void setAutoGain   (bool b)    { autoGain         = b; }
    void setBias       (float v)   { bias             = juce::jlimit(-0.5f,   0.5f,      v); }
    void setOsQuality  (int q)     { pendingOsQuality = juce::jlimit( 0,      2,          q); }
    void setEmphFreqHz (float hz)  { emphFreqHz       = juce::jlimit(200.0f, 10000.0f, hz); }
    void setEmphGainDb (float dB)  { emphGainDb       = juce::jlimit(0.0f,  12.0f,     dB); }
    void setKneeWidth  (float v)   { kneeWidth        = juce::jlimit(0.0f,   1.0f,      v); }

    int getLatencySamples() const override
    {
        const int q = currentOsQuality;
        return (os[q] != nullptr) ? (int)os[q]->getLatencyInSamples() : 0;
    }

private:
    // Transfer functions — applied per-sample in the oversampled domain.
    // knee=0: mode starts at ceiling (hardest). knee=1: mode operates over full range (softest).
    static float clipHard(float x, float ceiling, float knee) noexcept;
    static float clipSoft(float x, float ceiling, float knee) noexcept; // quintic, C2-continuous
    static float clipSine(float x, float ceiling, float knee) noexcept;
    static float clipTape(float x, float ceiling, float knee) noexcept;

    // Simple high-shelf biquad (Audio EQ Cookbook, shelf slope S=1)
    struct BiquadCoeffs { float b0, b1, b2, a1, a2; };
    struct BiquadState  { float z1 = 0.0f, z2 = 0.0f; };

    static BiquadCoeffs highShelf  (float freqHz, float gainDb, float sampleRate) noexcept;
    static float        processBiquad(float x, const BiquadCoeffs& c, BiquadState& s) noexcept;

    // Parameters
    Mode  mode            = Mode::Soft;
    float ceilingDb       = 0.0f;
    float mix             = 1.0f;
    bool  autoGain        = false;
    float bias            = 0.0f;    // DC bias: shifts clipping asymmetry, creates even harmonics
    float emphFreqHz      = 3000.0f;
    float emphGainDb      = 0.0f;
    float kneeWidth       = 0.33f;

    // 0 = 2x (1 stage), 1 = 4x (2 stages), 2 = 8x (3 stages)
    int pendingOsQuality  = 1;
    int currentOsQuality  = 1;

    static constexpr int kMaxChannels = 2;

    using OsType = juce::dsp::Oversampling<float>;
    std::unique_ptr<OsType> os[3]; // pre-allocated at prepare(); switched with zero reallocation

    // Per-channel biquad states for pre/post emphasis filters
    BiquadState emphPreState [kMaxChannels];
    BiquadState emphPostState[kMaxChannels];

    juce::AudioBuffer<float> dryBuffer;

    // Aligns the dry path with the active oversampler's group delay
    // so the parallel mix stays phase-coherent.
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd>
        dryDelay { 128 };
    bool wasMixing = false;

    double sampleRate_ = 44100.0;
};
