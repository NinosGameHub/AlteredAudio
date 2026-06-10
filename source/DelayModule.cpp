#include "DelayModule.h"

constexpr float DelayModule::kDiffMs[DelayModule::kDiffStages];

// Hermite cubic interpolation — smoother than linear for modulated reads
float DelayModule::readSample(int ch, float delaySamples) const
{
    const int d1  = (int)delaySamples;
    const float t = delaySamples - (float)d1;

    auto idx = [&](int offset) -> float {
        int i = writePos[ch] - d1 + offset;
        i = ((i % bufferSize) + bufferSize) % bufferSize;
        return delayBuffer[ch][i];
    };

    // Four-point Hermite: y(-1), y(0), y(1), y(2)
    const float ym1 = idx(1), y0 = idx(0), y1 = idx(-1), y2 = idx(-2);
    const float c0  = y0;
    const float c1  = 0.5f * (y1 - ym1);
    const float c2  = ym1 - 2.5f * y0 + 2.0f * y1 - 0.5f * y2;
    const float c3  = 0.5f * (y2 - ym1) + 1.5f * (y0 - y1);
    return ((c3 * t + c2) * t + c1) * t + c0;
}

void DelayModule::writeSample(int ch, float sample)
{
    delayBuffer[ch][writePos[ch]] = sample;
    writePos[ch] = (writePos[ch] + 1) % bufferSize;
}

void DelayModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    sr_ = sampleRate;

    bufferSize = (int)(kMaxDelayMs / 1000.0f * (float)sampleRate) + 4;
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        delayBuffer[ch].assign(bufferSize, 0.0f);
        writePos[ch] = 0;
        lpState[ch]  = 0.0f;
        hpLpSt[ch]   = 0.0f;
    }

    for (int s = 0; s < kDiffStages; ++s)
    {
        const int sz = (int)(kDiffMs[s] * 0.001f * sampleRate);
        for (int ch = 0; ch < kMaxChannels; ++ch)
            diffAP[s][ch].init(sz);
    }

    lfoPhase = wowPhase = flutterPhase = 0.0f;
    duckEnv = 0.0f;
}

void DelayModule::reset()
{
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        std::fill(delayBuffer[ch].begin(), delayBuffer[ch].end(), 0.0f);
        writePos[ch] = 0;
        lpState[ch]  = hpLpSt[ch] = 0.0f;
    }
    for (int s = 0; s < kDiffStages; ++s)
        for (int ch = 0; ch < kMaxChannels; ++ch)
            diffAP[s][ch].clear();

    lfoPhase = wowPhase = flutterPhase = duckEnv = 0.0f;
}

void DelayModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numCh   = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamp = buffer.getNumSamples();
    const float fsr   = (float)sr_;

    // ---- Coefficients ----
    // Tape darkens the LP frequency as wowFlutter increases
    const float effectiveLPHz = juce::jmax(500.0f, fbLPHz * (1.0f - wowFlutter * 0.65f));
    const float lpAlpha = 1.0f - std::exp(-(float)juce::MathConstants<double>::twoPi * effectiveLPHz / fsr);
    const float hpAlpha = 1.0f - std::exp(-(float)juce::MathConstants<double>::twoPi * fbHPHz       / fsr);

    const float lfoInc     = (float)juce::MathConstants<double>::twoPi * modRate     / fsr;
    const float wowInc     = (float)juce::MathConstants<double>::twoPi * 0.4f        / fsr;
    const float flutterInc = (float)juce::MathConstants<double>::twoPi * 7.3f        / fsr;
    const float diffG      = diffusion * 0.65f;

    // Ducking: 5ms attack, 150ms release
    const float duckAttCoeff = 1.0f - std::exp(-1.0f / (0.005f * fsr));
    const float duckRelCoeff = 1.0f - std::exp(-1.0f / (0.150f * fsr));

    // Tape wobble depth: ±0.8% of delay time per unit of wowFlutter
    const float baseSamples   = delayTimeMs * 0.001f * fsr;
    const float maxTapeSamp   = baseSamples * wowFlutter * 0.008f;
    // Modulation LFO depth: 0..20ms
    const float maxModSamp    = modDepth * 0.020f * fsr;
    const float spreadSamples = spreadMs * 0.001f * fsr;

    // Tape saturation drive
    const float satDrive  = 1.0f + wowFlutter * 2.5f;
    const float satNorm   = 1.0f / satDrive;
    const bool  doTape    = wowFlutter > 0.005f;
    const bool  doDiff    = diffusion  > 0.005f;
    const bool  doDuck    = ducking    > 0.005f;

    for (int i = 0; i < numSamp; ++i)
    {
        // ---- LFOs ----
        const float lfoVal     = std::sin(lfoPhase);
        lfoPhase += lfoInc;
        if (lfoPhase >= juce::MathConstants<float>::twoPi) lfoPhase -= juce::MathConstants<float>::twoPi;

        const float wowVal = std::sin(wowPhase);
        wowPhase += wowInc;
        if (wowPhase >= juce::MathConstants<float>::twoPi) wowPhase -= juce::MathConstants<float>::twoPi;

        // Flutter: two inharmonic sines → irregular pattern
        const float flutterVal = std::sin(flutterPhase) * 0.65f
                               + std::sin(flutterPhase * 1.73f) * 0.35f;
        flutterPhase += flutterInc;
        if (flutterPhase >= juce::MathConstants<float>::twoPi) flutterPhase -= juce::MathConstants<float>::twoPi;

        const float tapeWobble = (wowVal * 0.7f + flutterVal * 0.3f) * maxTapeSamp;
        const float modWobble  = lfoVal * maxModSamp;
        const float totalMod   = tapeWobble + modWobble;

        // ---- Effective delay times ----
        const float delL = juce::jlimit(2.0f, (float)(bufferSize - 2), baseSamples + totalMod);
        const float delR = juce::jlimit(2.0f, (float)(bufferSize - 2), baseSamples + spreadSamples + totalMod);

        // ---- Ducking envelope (tracks input level) ----
        const float dryL = numCh > 0 ? buffer.getSample(0, i) : 0.0f;
        const float dryR = numCh > 1 ? buffer.getSample(1, i) : dryL;

        float duckGain = 1.0f;
        if (doDuck)
        {
            const float inLevel = std::max(std::abs(dryL), std::abs(dryR));
            const float coeff   = inLevel > duckEnv ? duckAttCoeff : duckRelCoeff;
            duckEnv += (inLevel - duckEnv) * coeff;
            duckGain = juce::jlimit(0.05f, 1.0f, 1.0f - duckEnv * ducking * 3.0f);
        }

        // ---- Inline feedback processor ----
        auto processFB = [&](float delayed, int ch) -> float
        {
            float x = delayed * feedback;

            // Tone: LP then HP (bandpass character on repeats)
            lpState[ch] += lpAlpha * (x - lpState[ch]);
            x = lpState[ch];

            hpLpSt[ch] += hpAlpha * (x - hpLpSt[ch]);
            x -= hpLpSt[ch];

            // Diffusion: smear echoes into tails
            if (doDiff)
                for (int s = 0; s < kDiffStages; ++s)
                    x = diffAP[s][ch].process(x, diffG);

            // Tape saturation: warm distortion in feedback
            if (doTape)
                x = tanhApprox(x * satDrive) * satNorm;

            return x;
        };

        // ---- Write + output ----
        if (!pingPong)
        {
            for (int ch = 0; ch < numCh; ++ch)
            {
                const float dry      = (ch == 0) ? dryL : dryR;
                const float delSamp  = (ch == 0) ? delL : delR;
                const float delayed  = readSample(ch, delSamp);
                writeSample(ch, dry + processFB(delayed, ch));
                buffer.setSample(ch, i, dry * (1.0f - wetDry) + delayed * wetDry * duckGain);
            }
        }
        else if (numCh >= 2)
        {
            // Cross-feed: L reads from R buffer, R reads from L buffer
            const float delayed0 = readSample(0, delL);
            const float delayed1 = readSample(1, delR);
            writeSample(0, dryL + processFB(delayed1, 0));
            writeSample(1, dryR + processFB(delayed0, 1));
            buffer.setSample(0, i, dryL * (1.0f - wetDry) + delayed0 * wetDry * duckGain);
            buffer.setSample(1, i, dryR * (1.0f - wetDry) + delayed1 * wetDry * duckGain);
        }
    }
}
