#include "TransientShaperModule.h"

void TransientShaperModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    sampleRate_ = sampleRate;
    reset();
}

void TransientShaperModule::reset()
{
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        envFast[ch]    = 0.0f;
        envSlow[ch]    = 0.0f;
        gainSmooth[ch] = 1.0f;
    }
}

void TransientShaperModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numCh   = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamp = buffer.getNumSamples();
    const float fsr   = (float)sampleRate_;

    // ---- Time constants from speed param (computed once per block) ----
    // speed=1 = fast/snappy (tight drums), speed=0 = slow/smooth (pads, bass)
    const float fastRelMs  =  2.0f + (1.0f - speed) * 23.0f;   // 2ms..25ms
    const float slowAttMs  =  8.0f + (1.0f - speed) * 72.0f;   // 8ms..80ms
    const float slowRelMs  = 60.0f + (1.0f - speed) * 540.0f;  // 60ms..600ms

    const float attFast = 1.0f - std::exp(-1.0f / (0.1f   * 0.001f * fsr)); // fixed 0.1ms
    const float relFast = 1.0f - std::exp(-1.0f / (fastRelMs * 0.001f * fsr));
    const float attSlow = 1.0f - std::exp(-1.0f / (slowAttMs * 0.001f * fsr));
    const float relSlow = 1.0f - std::exp(-1.0f / (slowRelMs * 0.001f * fsr));

    // Gain smoother: fast attack (0.5ms) so shaping tracks transients,
    // slow release (15ms) to avoid clicks on the tail.
    const float gainAttCoeff = 1.0f - std::exp(-1.0f / (0.5f  * 0.001f * fsr));
    const float gainRelCoeff = 1.0f - std::exp(-1.0f / (15.0f * 0.001f * fsr));

    for (int i = 0; i < numSamp; ++i)
    {
        // ---- Update per-channel envelope followers ----
        for (int ch = 0; ch < numCh; ++ch)
        {
            const float level = std::abs(buffer.getSample(ch, i));
            const float df = level > envFast[ch] ? attFast : relFast;
            const float ds = level > envSlow[ch] ? attSlow : relSlow;
            envFast[ch] += df * (level - envFast[ch]);
            envSlow[ch] += ds * (level - envSlow[ch]);
        }

        // ---- Stereo link: blend per-channel levels toward the max ----
        float detFast[kMaxChannels], detSlow[kMaxChannels];
        for (int ch = 0; ch < numCh; ++ch)
        {
            detFast[ch] = envFast[ch];
            detSlow[ch] = envSlow[ch];
        }
        if (numCh >= 2 && stereoLink > 0.005f)
        {
            const float mxF = std::max(detFast[0], detFast[1]);
            const float mxS = std::max(detSlow[0], detSlow[1]);
            for (int ch = 0; ch < numCh; ++ch)
            {
                detFast[ch] += (mxF - detFast[ch]) * stereoLink;
                detSlow[ch] += (mxS - detSlow[ch]) * stereoLink;
            }
        }

        // ---- Per-channel gain compute + smooth + output ----
        for (int ch = 0; ch < numCh; ++ch)
        {
            // Transient activity in dB: how far the fast envelope sits above the slow one.
            // Positive = attack phase. Negative = sustain/decay phase.
            const float transDb =
                (detFast[ch] > 1e-10f && detSlow[ch] > 1e-10f)
                ? 20.0f * std::log10(detFast[ch] / detSlow[ch])
                : 0.0f;

            // Normalize against a ±10 dB reference so both controls reach full scale
            // on typical transients (0 dB = no transient / full sustain; 10 dB = sharp hit).
            constexpr float kRef = 10.0f;
            const float atkMult = juce::jlimit(0.0f, 1.0f,  transDb / kRef);
            const float susMult = juce::jlimit(0.0f, 1.0f, -transDb / kRef);

            const float gainDb     = attackGainDb * atkMult + sustainGainDb * susMult;
            const float targetGain = juce::Decibels::decibelsToGain(gainDb);

            // Asymmetric smoother: follow attack fast, smooth off the release
            const float coeff = targetGain > gainSmooth[ch] ? gainAttCoeff : gainRelCoeff;
            gainSmooth[ch] += coeff * (targetGain - gainSmooth[ch]);

            const float dry = buffer.getSample(ch, i);
            const float wet = dry * gainSmooth[ch];
            buffer.setSample(ch, i, dry + (wet - dry) * wetDry);
        }
    }
}
