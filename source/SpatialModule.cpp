#include "SpatialModule.h"

static constexpr float kPi   = juce::MathConstants<float>::pi;
static constexpr float kSqrt2 = 1.41421356237f;

void SpatialModule::setBassMono(float hz)
{
    bassMonoHz_ = hz;
    updateBassLp();
}

void SpatialModule::updateBassLp()
{
    if (bassMonoHz_ >= 20.0f && sampleRate_ > 0.0)
    {
        const float omega = 2.0f * kPi * bassMonoHz_ / (float)sampleRate_;
        bassLpAlpha = 1.0f - std::exp(-omega);
    }
    else
    {
        bassLpAlpha = 0.0f;   // disabled — LP output will stay at 0
    }
}

void SpatialModule::prepare(double sampleRate, int blockSize)
{
    sampleRate_ = sampleRate;

    const double rampSec = 0.02;  // 20 ms ramp (same as GainModule)
    widthSmooth .reset(sampleRate, rampSec);
    midSmooth   .reset(sampleRate, rampSec);
    sideSmooth  .reset(sampleRate, rampSec);
    wetDrySmooth.reset(sampleRate, rampSec);

    widthSmooth .setCurrentAndTargetValue(1.0f);
    midSmooth   .setCurrentAndTargetValue(1.0f);
    sideSmooth  .setCurrentAndTargetValue(1.0f);
    wetDrySmooth.setCurrentAndTargetValue(1.0f);

    juce::ignoreUnused(blockSize);
    updateBassLp();
    reset();
}

void SpatialModule::reset()
{
    bassLpState  = 0.0f;
    haasWritePos = 0;
    std::fill(std::begin(haasBuffer), std::end(haasBuffer), 0.0f);

    widthSmooth .setCurrentAndTargetValue(widthSmooth .getTargetValue());
    midSmooth   .setCurrentAndTargetValue(midSmooth   .getTargetValue());
    sideSmooth  .setCurrentAndTargetValue(sideSmooth  .getTargetValue());
    wetDrySmooth.setCurrentAndTargetValue(wetDrySmooth.getTargetValue());
}

void SpatialModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numCh = buffer.getNumChannels();
    const int N     = buffer.getNumSamples();
    if (numCh < 2 || N == 0) return;

    float* chL = buffer.getWritePointer(0);
    float* chR = buffer.getWritePointer(1);

    // ---- Block-rate: compute rotation matrix and pan gains ----

    const float rotRad   = targetRotDeg * (kPi / 180.0f);
    const float cosRot   = std::cos(rotRad);
    const float sinRot   = std::sin(rotRad);
    const bool  doRot    = (std::abs(rotRad) > 1e-5f);

    // Balance pan: constant-power law.
    // pan = 0 → both channels unity.
    // pan = +1 → hard right: L = 0, R = 1.
    // pan = -1 → hard left: L = 1, R = 0.
    const float panAng = (targetPan + 1.0f) * (kPi * 0.25f);
    const float panL   = std::cos(panAng) * kSqrt2;
    const float panR   = std::sin(panAng) * kSqrt2;

    // Haas delay: how many whole samples to delay R
    const int   haasSmp  = std::min((int)(targetHaasMs * (float)sampleRate_ * 0.001f + 0.5f),
                                    kMaxHaasSamples - 1);
    const bool  doHaas   = (haasSmp > 0);
    const bool  doBass   = (bassLpAlpha > 0.0f);

    const bool smoothing = widthSmooth .isSmoothing() || midSmooth.isSmoothing()
                        || sideSmooth  .isSmoothing() || wetDrySmooth.isSmoothing();

    // ---- Fast path: no per-sample smoothing needed ----
    if (!smoothing)
    {
        const float width  = widthSmooth .getCurrentValue();
        const float midG   = midSmooth   .getCurrentValue();
        const float sideG  = sideSmooth  .getCurrentValue();
        const float wetDry = wetDrySmooth.getCurrentValue();
        const float dry    = 1.0f - wetDry;

        for (int i = 0; i < N; ++i)
        {
            const float l0 = chL[i], r0 = chR[i];

            // 1. M/S encode
            float mid = (l0 + r0) * 0.5f;
            float sid = (l0 - r0) * 0.5f;

            // 2. Mid / Side gain
            mid *= midG;
            sid *= sideG;

            // 3. Width
            sid *= width;

            // 4. Bass Mono — subtract bass from Side (below cut, image collapses to mono)
            if (doBass)
            {
                bassLpState += bassLpAlpha * (sid - bassLpState);
                sid -= bassLpState;
            }

            // 5. M/S decode
            float l = mid + sid;
            float r = mid - sid;

            // 6. Rotation
            if (doRot)
            {
                const float rl = l * cosRot - r * sinRot;
                const float rr = l * sinRot + r * cosRot;
                l = rl; r = rr;
            }

            // 7. Pan
            l *= panL;
            r *= panR;

            // 8. Haas spread (delay R)
            if (doHaas)
            {
                haasBuffer[haasWritePos] = r;
                const int rp = (haasWritePos - haasSmp + kMaxHaasSamples) % kMaxHaasSamples;
                r = haasBuffer[rp];
                haasWritePos = (haasWritePos + 1) % kMaxHaasSamples;
            }

            // 9. Wet/dry
            chL[i] = l0 * dry + l * wetDry;
            chR[i] = r0 * dry + r * wetDry;
        }
        return;
    }

    // ---- Per-sample path: advance all smoothers ----
    for (int i = 0; i < N; ++i)
    {
        const float width  = widthSmooth .getNextValue();
        const float midG   = midSmooth   .getNextValue();
        const float sideG  = sideSmooth  .getNextValue();
        const float wetDry = wetDrySmooth.getNextValue();
        const float dry    = 1.0f - wetDry;

        const float l0 = chL[i], r0 = chR[i];

        // 1. M/S encode
        float mid = (l0 + r0) * 0.5f;
        float sid = (l0 - r0) * 0.5f;

        // 2. Mid / Side gain
        mid *= midG;
        sid *= sideG;

        // 3. Width
        sid *= width;

        // 4. Bass Mono
        if (doBass)
        {
            bassLpState += bassLpAlpha * (sid - bassLpState);
            sid -= bassLpState;
        }

        // 5. M/S decode
        float l = mid + sid;
        float r = mid - sid;

        // 6. Rotation
        if (doRot)
        {
            const float rl = l * cosRot - r * sinRot;
            const float rr = l * sinRot + r * cosRot;
            l = rl; r = rr;
        }

        // 7. Pan
        l *= panL;
        r *= panR;

        // 8. Haas spread
        if (doHaas)
        {
            haasBuffer[haasWritePos] = r;
            const int rp = (haasWritePos - haasSmp + kMaxHaasSamples) % kMaxHaasSamples;
            r = haasBuffer[rp];
            haasWritePos = (haasWritePos + 1) % kMaxHaasSamples;
        }

        // 9. Wet/dry
        chL[i] = l0 * dry + l * wetDry;
        chR[i] = r0 * dry + r * wetDry;
    }
}
