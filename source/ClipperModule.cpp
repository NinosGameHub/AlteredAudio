#include "ClipperModule.h"

// ============================================================
// Transfer functions (knee-aware, per-sample, oversampled domain)
// ============================================================

// Hard: cubic Hermite knee — C1 continuous, slope 1 at kneeStart, slope 0 at ceiling.
// knee=0 → pure hard clip; knee=1 → soft knee across full range.
float ClipperModule::clipHard(float x, float ceiling, float knee) noexcept
{
    const float a = std::abs(x);
    if (a >= ceiling) return std::copysign(ceiling, x);
    if (knee < 0.001f) return x;

    const float ks = ceiling * (1.0f - knee);
    if (a <= ks) return x;

    const float t    = (a - ks) / (ceiling - ks);
    const float aOut = ks + (ceiling - ks) * t * (1.0f + t - t * t);
    return std::copysign(aOut, x);
}

// Soft: quintic Hermite knee — C2 continuous (zero slope AND curvature at both endpoints).
// Derived: p(t) = (C-ks)*(3t^5 - 7t^4 + 4t^3 + t) + ks with p'(0)=1, p'(1)=0, p''(0)=p''(1)=0.
// Default knee=0.33 gives kneeStart ≈ 2/3 ceiling (matching the original fixed-knee behavior).
float ClipperModule::clipSoft(float x, float ceiling, float knee) noexcept
{
    const float a = std::abs(x);
    if (a >= ceiling) return std::copysign(ceiling, x);

    const float ks = ceiling * (1.0f - juce::jmax(knee, 0.001f));
    if (a <= ks) return x;

    const float t  = (a - ks) / (ceiling - ks);
    const float t2 = t * t;
    const float t3 = t2 * t;
    // 3t^5 = 3*t2*t3, 7t^4 = 7*t2*t2
    const float aOut = ks + (ceiling - ks) * (3.0f * t2 * t3 - 7.0f * t2 * t2 + 4.0f * t3 + t);
    return std::copysign(aOut, x);
}

// Sine: sine curve within the knee zone. knee=1 → full sine from 0 to ceiling (original behavior).
// Note: slope at kneeStart is π/2 (not 1), creating a slight kink for knee < 1.
float ClipperModule::clipSine(float x, float ceiling, float knee) noexcept
{
    const float a = std::abs(x);
    if (a >= ceiling) return std::copysign(ceiling, x);

    const float ks = ceiling * (1.0f - juce::jmax(knee, 0.001f));
    if (a <= ks) return x;

    // Map [ks, ceiling] → [0,1], apply sine, map back to [ks, ceiling].
    const float t    = (a - ks) / (ceiling - ks);
    const float aOut = ks + std::sin(juce::MathConstants<float>::halfPi * t) * (ceiling - ks);
    return std::copysign(aOut, x);
}

// Tape: tanh saturation with drive inversely scaled by knee.
// knee=0 → drive=4 (aggressive). knee=1 → drive=1.5 (warm, gradual).
float ClipperModule::clipTape(float x, float ceiling, float knee) noexcept
{
    const float drive = 1.5f + (1.0f - knee) * 2.5f;
    const float norm  = 1.0f / std::tanh(drive);
    return std::tanh(x * drive / ceiling) * ceiling * norm;
}

// ============================================================
// High-shelf biquad (Audio EQ Cookbook, S=1, direct form II transposed)
// ============================================================

ClipperModule::BiquadCoeffs
ClipperModule::highShelf(float freqHz, float gainDb, float sampleRate) noexcept
{
    const float A     = std::pow(10.0f, gainDb / 40.0f);  // sqrt(10^(dBgain/20))
    const float w0    = juce::MathConstants<float>::twoPi * freqHz / sampleRate;
    const float cs    = std::cos(w0);
    const float sn    = std::sin(w0);
    const float alpha = sn * juce::MathConstants<float>::sqrt2 * 0.5f;  // S=1: sin(w0)/sqrt(2)
    const float sqA2  = 2.0f * std::sqrt(A) * alpha;

    const float b0 =       A * ((A + 1.0f) + (A - 1.0f) * cs + sqA2);
    const float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cs);
    const float b2 =       A * ((A + 1.0f) + (A - 1.0f) * cs - sqA2);
    const float a0 =            (A + 1.0f) - (A - 1.0f) * cs + sqA2;
    const float a1 =  2.0f *   ((A - 1.0f) - (A + 1.0f) * cs);
    const float a2 =            (A + 1.0f) - (A - 1.0f) * cs - sqA2;

    return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
}

float ClipperModule::processBiquad(float x, const BiquadCoeffs& c, BiquadState& s) noexcept
{
    const float y = c.b0 * x + s.z1;
    s.z1 = c.b1 * x - c.a1 * y + s.z2;
    s.z2 = c.b2 * x - c.a2 * y;
    return y;
}

// ============================================================
// Module lifecycle
// ============================================================

void ClipperModule::prepare(double sampleRate, int blockSize)
{
    sampleRate_ = sampleRate;

    using FilterType = OsType::FilterType;
    for (int i = 0; i < 3; ++i)
    {
        os[i] = std::make_unique<OsType>(
            kMaxChannels, i + 1,                            // stages: 1=2x, 2=4x, 3=8x
            FilterType::filterHalfBandPolyphaseIIR,
            true, false);                                    // normalizeGain, !useIntegerLatency
        os[i]->initProcessing(static_cast<size_t>(blockSize));
    }

    currentOsQuality = juce::jlimit(0, 2, pendingOsQuality);
    dryBuffer.setSize(kMaxChannels, blockSize, false, true, true);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)blockSize, kMaxChannels };
    dryDelay.prepare(spec);
    dryDelay.setDelay(os[currentOsQuality]->getLatencyInSamples());
    wasMixing = false;

    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        emphPreState [ch] = {};
        emphPostState[ch] = {};
    }
}

void ClipperModule::reset()
{
    for (int i = 0; i < 3; ++i)
        if (os[i] != nullptr)
            os[i]->reset();

    dryDelay.reset();
    wasMixing = false;

    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        emphPreState [ch] = {};
        emphPostState[ch] = {};
    }
}

// ============================================================
// Process
// ============================================================

void ClipperModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numCh      = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamples = buffer.getNumSamples();

    const float ceiling     = juce::Decibels::decibelsToGain(ceilingDb);
    const float autoGainLin = autoGain ? (1.0f / ceiling) : 1.0f;
    const float biasLin     = bias * ceiling;
    const bool  doMix       = mix < 0.999f;
    const bool  doEmph      = emphGainDb > 0.1f;

    // ---- 0. Save pre-processing dry signal for parallel mix ----
    if (doMix)
    {
        dryBuffer.setSize(numCh, numSamples, false, false, true);
        for (int ch = 0; ch < numCh; ++ch)
            dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }

    // ---- 1. Pre-emphasis: high-shelf boost before clipping (original SR) ----
    if (doEmph)
    {
        const BiquadCoeffs pre = highShelf(emphFreqHz, emphGainDb, (float)sampleRate_);
        for (int ch = 0; ch < numCh; ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                data[i] = processBiquad(data[i], pre, emphPreState[ch]);
        }
    }

    // ---- 2. Switch oversampling quality without reallocating ----
    {
        const int newQ = juce::jlimit(0, 2, pendingOsQuality);
        if (newQ != currentOsQuality)
        {
            os[newQ]->reset();  // flush state before first use at this quality
            currentOsQuality = newQ;
        }
    }

    // ---- 3. Upsample → clip → downsample ----
    juce::dsp::AudioBlock<float> block(buffer);
    auto osBlock = os[currentOsQuality]->processSamplesUp(block);

    const int osNumSamples = (int)osBlock.getNumSamples();
    for (int ch = 0; ch < (int)osBlock.getNumChannels(); ++ch)
    {
        float* data = osBlock.getChannelPointer(ch);
        for (int i = 0; i < osNumSamples; ++i)
        {
            float s = data[i] + biasLin;  // shift operating point for asymmetric clipping

            switch (mode)
            {
                case Mode::Hard: s = clipHard(s, ceiling, kneeWidth); break;
                case Mode::Soft: s = clipSoft(s, ceiling, kneeWidth); break;
                case Mode::Sine: s = clipSine(s, ceiling, kneeWidth); break;
                case Mode::Tape: s = clipTape(s, ceiling, kneeWidth); break;
            }

            s -= biasLin;                             // restore DC center
            s  = juce::jlimit(-ceiling, ceiling, s);  // brickwall after asymmetric de-bias

            data[i] = s * autoGainLin;
        }
    }

    os[currentOsQuality]->processSamplesDown(block);

    // ---- 4. Post-emphasis: exact inverse shelf to restore frequency balance ----
    if (doEmph)
    {
        const BiquadCoeffs post = highShelf(emphFreqHz, -emphGainDb, (float)sampleRate_);
        for (int ch = 0; ch < numCh; ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                data[i] = processBiquad(data[i], post, emphPostState[ch]);
        }
    }

    // ---- 5. Parallel mix (dry aligned to the oversampler's group delay) ----
    if (doMix)
    {
        if (!wasMixing)
            dryDelay.reset();   // flush stale samples from when mix was 100%
        dryDelay.setDelay(os[currentOsQuality]->getLatencyInSamples());

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* d = dryBuffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                dryDelay.pushSample(ch, d[i]);
                d[i] = dryDelay.popSample(ch);
            }
        }

        for (int ch = 0; ch < numCh; ++ch)
        {
            buffer.applyGain(ch, 0, numSamples, mix);
            buffer.addFrom(ch, 0, dryBuffer, ch, 0, numSamples, 1.0f - mix);
        }
    }
    wasMixing = doMix;
}
