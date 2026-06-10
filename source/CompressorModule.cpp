#include "CompressorModule.h"

void CompressorModule::updateCoeffs()
{
    attackCoeff  = std::exp(-1.0f / (attackMs  * 0.001f * (float)sampleRate_));
    releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * (float)sampleRate_));
}

void CompressorModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    sampleRate_ = sampleRate;

    lookaheadBufSize = (int)(kMaxLookaheadMs * 0.001 * sampleRate) + 4;
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        lookaheadBuf[ch].assign(lookaheadBufSize, 0.0f);
        lookaheadWritePos[ch] = 0;
        smoothedGain_[ch] = 1.0f;
        rmsState[ch] = scHpState[ch] = scLpState[ch] = 0.0f;
    }
    updateCoeffs();
}

void CompressorModule::reset()
{
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        smoothedGain_[ch] = 1.0f;
        rmsState[ch] = scHpState[ch] = scLpState[ch] = 0.0f;
        std::fill(lookaheadBuf[ch].begin(), lookaheadBuf[ch].end(), 0.0f);
        lookaheadWritePos[ch] = 0;
    }
    gainReductionDb = 0.0f;
}

float CompressorModule::computeGainDb(float levelDb) const
{
    const float halfKnee = kneeDb / 2.0f;
    const float delta    = levelDb - thresholdDb;

    if (delta < -halfKnee) return 0.0f;
    if (delta > halfKnee)  return delta * (1.0f / ratio - 1.0f);

    const float t = (delta + halfKnee) / kneeDb;
    return (t * t) * (kneeDb / 2.0f) * (1.0f / ratio - 1.0f);
}

void CompressorModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();
    const int numCh       = std::min(numChannels, kMaxChannels);
    const float fsr       = (float)sampleRate_;
    const float makeupLin = juce::Decibels::decibelsToGain(makeupGainDb);

    // Sidechain filter coefficients (1-pole)
    const float hpAlpha  = 1.0f - std::exp(-(float)juce::MathConstants<double>::twoPi * scHPHz / fsr);
    const float lpAlpha  = 1.0f - std::exp(-(float)juce::MathConstants<double>::twoPi * scLPHz / fsr);
    // RMS integration
    const float rmsAlpha = 1.0f - std::exp(-1.0f / (kRmsTimeMs * 0.001f * fsr));

    // Lookahead delay in samples
    const int lookaheadSamp = juce::jlimit(0, lookaheadBufSize - 1,
                                            (int)(lookaheadMs * 0.001f * fsr));
    const bool doLook = lookaheadSamp > 0;

    // Saturation
    const float satDrive = 1.0f + saturation * 4.0f;
    const float satNorm  = 1.0f / satDrive;
    const bool  doSat    = saturation > 0.005f;

    // Program-dependent release: per-dB-below-threshold speedup
    constexpr float kAutoRelScale = 0.15f;

    for (int i = 0; i < numSamples; ++i)
    {
        float dry[kMaxChannels] = {};
        for (int ch = 0; ch < numCh; ++ch)
            dry[ch] = buffer.getSample(ch, i);

        // ---- Sidechain detection: HP + LP filter, then peak/RMS blend ----
        float detLevel[kMaxChannels] = {};
        for (int ch = 0; ch < numCh; ++ch)
        {
            float sc = dry[ch];
            // HP: remove sub-bass from detector
            scHpState[ch] += hpAlpha * (sc - scHpState[ch]);
            sc -= scHpState[ch];
            // LP: remove ultra-highs from detector
            scLpState[ch] += lpAlpha * (sc - scLpState[ch]);
            sc = scLpState[ch];

            const float peak = std::abs(sc);
            rmsState[ch] += rmsAlpha * (sc * sc - rmsState[ch]);
            const float rms = std::sqrt(juce::jmax(0.0f, rmsState[ch]));
            detLevel[ch] = peak + (rms - peak) * rmsPeak;
        }

        // ---- Stereo link ----
        if (numCh >= 2 && stereoLink > 0.005f)
        {
            const float linked = std::max(detLevel[0], detLevel[1]);
            detLevel[0] += (linked - detLevel[0]) * stereoLink;
            detLevel[1] += (linked - detLevel[1]) * stereoLink;
        }

        // ---- Per-channel gain computer ----
        for (int ch = 0; ch < numCh; ++ch)
        {
            const float levelDb = detLevel[ch] > 1e-6f
                                  ? juce::Decibels::gainToDecibels(detLevel[ch]) : -120.0f;
            const float gainDb  = computeGainDb(levelDb);
            const float target  = juce::Decibels::decibelsToGain(gainDb) * makeupLin;

            // Program-dependent release: speeds up when signal is well below threshold
            float relCoeff = releaseCoeff;
            if (autoRelease > 0.005f)
            {
                const float belowThresh = juce::jmax(0.0f, thresholdDb - levelDb);
                const float factor      = juce::jmin(8.0f, 1.0f + autoRelease * belowThresh * kAutoRelScale);
                relCoeff = std::exp(-factor / (releaseMs * 0.001f * fsr));
            }

            const float coeff    = target < smoothedGain_[ch] ? attackCoeff : relCoeff;
            smoothedGain_[ch]    = target + coeff * (smoothedGain_[ch] - target);
        }

        // ---- Lookahead: write current dry, read N-samples-ago ----
        float audioIn[kMaxChannels];
        for (int ch = 0; ch < numCh; ++ch)
        {
            if (doLook)
            {
                lookaheadBuf[ch][lookaheadWritePos[ch]] = dry[ch];
                int readPos = lookaheadWritePos[ch] - lookaheadSamp;
                if (readPos < 0) readPos += lookaheadBufSize;
                audioIn[ch] = lookaheadBuf[ch][readPos];
                if (++lookaheadWritePos[ch] >= lookaheadBufSize) lookaheadWritePos[ch] = 0;
            }
            else
            {
                audioIn[ch] = dry[ch];
            }
        }

        // ---- Output: parallel blend then optional saturation ----
        for (int ch = 0; ch < numCh; ++ch)
        {
            const float compressed = audioIn[ch] * smoothedGain_[ch];
            float out = audioIn[ch] + (compressed - audioIn[ch]) * mix;

            if (doSat)
                out = tanhApprox(out * satDrive) * satNorm;

            buffer.setSample(ch, i, out);
        }
    }

    // GR meter: strip makeup so it reads pure gain reduction
    gainReductionDb = juce::Decibels::gainToDecibels(smoothedGain_[0] / makeupLin);
}
