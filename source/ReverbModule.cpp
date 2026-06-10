#include "ReverbModule.h"

// constexpr definitions (required in C++14/17 when address is taken)
constexpr float ReverbModule::kErMsL[8];
constexpr float ReverbModule::kErMsR[8];
constexpr float ReverbModule::kErGains[8];

float ReverbModule::processAP (float in, float g, DelayLine& dl, float delay)
{
    const float dN = dl.readInterp(delay);
    const float d  = in - g * dN;
    dl.write(d);
    return g * d + dN;
}

void ReverbModule::prepare (double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    sr = sampleRate;

    diff[0].init(scale(kD1) + 2);
    diff[1].init(scale(kD2) + 2);
    diff[2].init(scale(kD3) + 2);
    diff[3].init(scale(kD4) + 2);

    apA1.init(scale(kAPA1) + 32);
    delA1.init(scale(kDA1)  + 2);
    // apA2 buffer must cover both the AP delay AND the output tap at 1913 ref samples
    apA2.init(juce::jmax(scale(kAPA2), scale(1913)) + 4);
    delA2.init(scale(kDA2) + 2);

    apB1.init(scale(kAPB1) + 32);
    delB1.init(scale(kDB1) + 2);
    apB2.init(scale(kAPB2) + 4);
    delB2.init(scale(kDB2) + 2);

    preDelL.init((int)(0.101 * sampleRate) + 2);
    preDelR.init((int)(0.101 * sampleRate) + 2);

    // ER buffer must reach the longest tap (107 ms)
    erBuf.init((int)(0.12 * sampleRate) + 2);

    const int grainSamples = (int)(0.05 * sampleRate);  // 50 ms grains
    psA.init(grainSamples);
    psB.init(grainSamples);

    dampA = dampB = hpLpA = hpLpB = 0.0f;
    lfoPhase = 0.0f;
}

void ReverbModule::reset()
{
    for (auto& d : diff) d.clear();
    apA1.clear(); delA1.clear(); apA2.clear(); delA2.clear();
    apB1.clear(); delB1.clear(); apB2.clear(); delB2.clear();
    preDelL.clear(); preDelR.clear();
    erBuf.clear();
    psA.clear(); psB.clear();
    dampA = dampB = hpLpA = hpLpB = 0.0f;
    lfoPhase = 0.0f;
}

void ReverbModule::process (juce::AudioBuffer<float>& buffer)
{
    const int numCh   = std::min(buffer.getNumChannels(), 2);
    const int numSamp = buffer.getNumSamples();

    // --- Coefficients ---
    const float decayCoeff = 0.5f + decay * 0.48f;
    const float dampCoeff  = damping * 0.9f;
    const float g12        = 0.75f  * diffusion;
    const float g34        = 0.625f * diffusion;
    const float modDepth   = kModDepthRef * (float)(sr / kRefRate);
    const float lfoInc     = (float)(juce::MathConstants<double>::twoPi * modRate / sr);
    const int   pdSamp     = (int)(preDelayMs * 0.001f * (float)sr);

    // 1-pole HP low-cut: α = 1 - exp(-2π·fc/sr)
    const float hpAlpha = 1.0f - std::exp(
        -(float)juce::MathConstants<double>::twoPi * lowCutHz / (float)sr);

    const float pitchRatio = std::pow(2.0f, pitchSemi / 12.0f);
    psA.setPitchRatio(pitchRatio);
    psB.setPitchRatio(pitchRatio);

    // --- Pre-scaled delay lengths ---
    const float fAPA1 = (float)scale(kAPA1), fAPB1 = (float)scale(kAPB1);
    const float fAPA2 = (float)scale(kAPA2), fAPB2 = (float)scale(kAPB2);
    const int   iDA1  = scale(kDA1), iDA2 = scale(kDA2);
    const int   iDB1  = scale(kDB1), iDB2 = scale(kDB2);
    const int   iD1   = scale(kD1),  iD2  = scale(kD2);
    const int   iD3   = scale(kD3),  iD4  = scale(kD4);

    // --- ER tap positions ---
    int erTL[8], erTR[8];
    for (int t = 0; t < 8; ++t)
    {
        erTL[t] = (int)(kErMsL[t] * 0.001f * (float)sr);
        erTR[t] = (int)(kErMsR[t] * 0.001f * (float)sr);
    }
    // Normalise ER to unity sum (sum of gains ≈ 5.05)
    static constexpr float kErNorm = 0.198f;

    // --- Output tap positions (complete 7-tap Dattorro + apA2/apB2 internal taps) ---
    const int tA1a = scale(266),  tA1b = scale(2974), tA1c = scale(335);
    const int tA2a = scale(2673), tA2b = scale(1519);
    const int tB1a = scale(353),  tB1b = scale(3627), tB1c = scale(2664);
    const int tB2a = scale(1228), tB2b = scale(187);
    const int tAP_A2 = scale(1913);   // reads from apA2 internal buffer (enlarged in prepare)
    const int tAP_B2 = scale(2111);   // reads from apB2 internal buffer

    for (int i = 0; i < numSamp; ++i)
    {
        const float inL  = numCh > 0 ? buffer.getSample(0, i) : 0.0f;
        const float inR  = numCh > 1 ? buffer.getSample(1, i) : inL;

        // --- Early reflections (from dry signal, before pre-delay) ---
        erBuf.write((inL + inR) * 0.5f);
        float erL = 0.0f, erR = 0.0f;
        for (int t = 0; t < 8; ++t)
        {
            const float s = erBuf.read(erTL[t]);
            erL += s * kErGains[t];
            erR += erBuf.read(erTR[t]) * kErGains[t];
        }
        erL *= kErNorm;
        erR *= kErNorm;

        // --- True-stereo pre-delay ---
        preDelL.write(inL);
        preDelR.write(inR);
        const float pdL = preDelL.read(pdSamp);
        const float pdR = preDelR.read(pdSamp);

        // --- Input diffusion (mono mid, 4 series allpass) ---
        float s = (pdL + pdR) * 0.5f;
        s = processAP(s, g12, diff[0], (float)iD1);
        s = processAP(s, g12, diff[1], (float)iD2);
        s = processAP(s, g34, diff[2], (float)iD3);
        s = processAP(s, g34, diff[3], (float)iD4);

        // Side component injects stereo spread into the tanks
        const float side = (pdL - pdR) * 0.35f;

        // --- LFO ---
        const float lfoVal = std::sin(lfoPhase);
        lfoPhase += lfoInc;
        if (lfoPhase > juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;

        // --- Cross-feedback with optional shimmer pitch shift ---
        const float rawFbA = delA2.read(iDA2);
        const float rawFbB = delB2.read(iDB2);
        // Always run pitch shifters so they stay warm (shimmer lerps the output)
        const float pitchedA = psB.process(rawFbA);
        const float pitchedB = psA.process(rawFbB);
        const float fbA = (rawFbA + (pitchedA - rawFbA) * shimmer) * decayCoeff;
        const float fbB = (rawFbB + (pitchedB - rawFbB) * shimmer) * decayCoeff;

        // --- Tank A (left-biased input: mid + side) ---
        {
            float v = processAP(s + side + fbB, 0.7f, apA1, fAPA1 + modDepth * lfoVal);
            delA1.write(v);
            const float dA = delA1.read(iDA1);
            // LP damping
            dampA = dA * (1.0f - dampCoeff) + dampA * dampCoeff;
            // HP low-cut (1-pole, removes mud from feedback)
            hpLpA += hpAlpha * (dampA - hpLpA);
            const float filtA = (dampA - hpLpA) * decayCoeff;
            delA2.write(processAP(filtA, 0.5f, apA2, fAPA2));
        }

        // --- Tank B (right-biased input: mid - side) ---
        {
            float v = processAP(s - side + fbA, 0.7f, apB1, fAPB1 - modDepth * lfoVal);
            delB1.write(v);
            const float dB = delB1.read(iDB1);
            dampB = dB * (1.0f - dampCoeff) + dampB * dampCoeff;
            hpLpB += hpAlpha * (dampB - hpLpB);
            const float filtB = (dampB - hpLpB) * decayCoeff;
            delB2.write(processAP(filtB, 0.5f, apB2, fAPB2));
        }

        // --- Stereo output: 6-tap Dattorro (delay lines + AP internal reads) ---
        const float wetL = ( delA1.read(tA1a) + delA1.read(tA1b)
                           - apA2.read(tAP_A2) + delA2.read(tA2a)
                           - delB1.read(tB1c)  - delB2.read(tB2b) ) * (1.0f / 6.0f);

        const float wetR = ( delB1.read(tB1a) + delB1.read(tB1b)
                           - apB2.read(tAP_B2) + delB2.read(tB2a)
                           - delA1.read(tA1c)  - delA2.read(tA2b) ) * (1.0f / 6.0f);

        // --- Mix ER with late reverb, then wet/dry ---
        const float mixL = wetL * (1.0f - erLevel) + erL * erLevel;
        const float mixR = wetR * (1.0f - erLevel) + erR * erLevel;

        if (numCh > 0) buffer.setSample(0, i, inL * (1.0f - wetDry) + mixL * wetDry);
        if (numCh > 1) buffer.setSample(1, i, inR * (1.0f - wetDry) + mixR * wetDry);
    }
}
