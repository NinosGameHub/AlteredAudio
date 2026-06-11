#include "EQModule.h"

void EQModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    currentSampleRate = sampleRate;
    for (int b = 0; b < kMaxBands; ++b)
        for (int s = 0; s < kMaxSections; ++s)
            sections[b][s].prepare(sampleRate);
}

void EQModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamples  = buffer.getNumSamples();

    for (int b = 0; b < kMaxBands; ++b)
    {
        if (!params[b].enabled) continue;
        for (int s = 0; s < numSections[b]; ++s)
        {
            auto& f = sections[b][s];
            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto* data = buffer.getWritePointer(ch);
                for (int i = 0; i < numSamples; ++i)
                    data[i] = f.processSample(data[i], ch);
            }
        }
    }
}

void EQModule::reset()
{
    for (int b = 0; b < kMaxBands; ++b)
        for (int s = 0; s < kMaxSections; ++s)
            sections[b][s].reset();
}

int EQModule::planSections(const BandParams& p, Section out[kMaxSections])
{
    const bool isLP = (p.type == BiquadFilter::Type::LowPass);
    const bool isHP = (p.type == BiquadFilter::Type::HighPass);

    if (!isLP && !isHP)
    {
        out[0] = { p.type, p.q };
        return 1;
    }

    const auto t1 = isLP ? BiquadFilter::Type::LowPass1  : BiquadFilter::Type::HighPass1;
    const auto t2 = p.type;

    // Butterworth cascade Qs. The user Q scales the resonant (highest-Q)
    // section, so the Q knob adds an analog-style resonance peak at cutoff.
    const float qScale = juce::jmax(0.1f, p.q) / 0.7071f;

    switch (juce::jlimit(0, 4, p.slope))
    {
        case 0:  // 6 dB/oct — single first-order section, no resonance possible
            out[0] = { t1, 0.7071f };
            return 1;
        default:
        case 1:  // 12 dB/oct
            out[0] = { t2, 0.7071f * qScale };
            return 1;
        case 2:  // 18 dB/oct — 3rd order: first-order + biquad Q=1
            out[0] = { t1, 0.7071f };
            out[1] = { t2, 1.0f * qScale };
            return 2;
        case 3:  // 24 dB/oct — 4th order Butterworth
            out[0] = { t2, 0.5412f };
            out[1] = { t2, 1.3066f * qScale };
            return 2;
        case 4:  // 48 dB/oct — 8th order Butterworth
            out[0] = { t2, 0.5098f };
            out[1] = { t2, 0.6013f };
            out[2] = { t2, 0.9000f };
            out[3] = { t2, 2.5629f * qScale };
            return 4;
    }
}

void EQModule::setBand(int index, const BandParams& p)
{
    if (index < 0 || index >= kMaxBands) return;
    params[index] = p;

    Section plan[kMaxSections];
    const int n = planSections(p, plan);

    // Topology changed (slope/type switch) — clear state so stale samples
    // from a previous configuration can't leak into the new cascade.
    if (n != numSections[index])
        for (int s = 0; s < kMaxSections; ++s)
            sections[index][s].reset();

    numSections[index] = n;
    for (int s = 0; s < n; ++s)
    {
        auto& f = sections[index][s];
        f.setType(plan[s].type);
        f.setFrequency(p.frequency);
        f.setQ(juce::jmax(0.05f, plan[s].q));
        f.setGainDb(p.gainDb);
    }
}

float EQModule::bandResponseDb(const BandParams& p, float freqHz, double sampleRate)
{
    if (!p.enabled) return 0.0f;

    Section plan[kMaxSections];
    const int n = planSections(p, plan);

    double db = 0.0;
    for (int s = 0; s < n; ++s)
    {
        double c[5];
        BiquadFilter::calcCoefficients(plan[s].type, sampleRate, p.frequency,
                                       juce::jmax(0.05f, plan[s].q), p.gainDb, c);
        db += BiquadFilter::magnitudeDb(c, (double)freqHz, sampleRate);
    }
    return (float)db;
}
