#include "FilterModule.h"

void FilterModule::prepare(double sampleRate, int blockSize)
{
    for (auto& s : sections) s.prepare(sampleRate);
    dryBuffer.setSize(kMaxChannels, blockSize, false, true, true);

    // Control smoothing: fast enough to track LFO modulation, slow enough
    // to kill coefficient-step transients (the low-frequency "cracks").
    freqSm .reset(sampleRate, 0.012);
    qSm    .reset(sampleRate, 0.020);
    gainSm .reset(sampleRate, 0.020);
    driveSm.reset(sampleRate, 0.020);
    mixSm  .reset(sampleRate, 0.020);
    outSm  .reset(sampleRate, 0.020);

    freqSm .setCurrentAndTargetValue(freqSm.getTargetValue());
    qSm    .setCurrentAndTargetValue(qSm.getTargetValue());
    gainSm .setCurrentAndTargetValue(gainSm.getTargetValue());
    driveSm.setCurrentAndTargetValue(driveSm.getTargetValue());
    mixSm  .setCurrentAndTargetValue(mixSm.getTargetValue());
    outSm  .setCurrentAndTargetValue(outSm.getTargetValue());

    applyControl(freqSm.getCurrentValue(), qSm.getCurrentValue(), gainSm.getCurrentValue());
}

void FilterModule::reset()
{
    for (auto& s : sections) s.reset();
    freqSm .setCurrentAndTargetValue(freqSm.getTargetValue());
    qSm    .setCurrentAndTargetValue(qSm.getTargetValue());
    gainSm .setCurrentAndTargetValue(gainSm.getTargetValue());
    driveSm.setCurrentAndTargetValue(driveSm.getTargetValue());
    mixSm  .setCurrentAndTargetValue(mixSm.getTargetValue());
    outSm  .setCurrentAndTargetValue(outSm.getTargetValue());
}

int FilterModule::planSections(FilterType type, int slope, float q, Section out[2])
{
    const bool isLPHP = (type == FilterType::LowPass || type == FilterType::HighPass);

    if (!isLPHP || slope == 0)
    {
        out[0] = { type, q };
        return 1;
    }

    // 24 dB/oct — 4th-order Butterworth; user Q scales the resonant section
    const float qScale = juce::jmax(0.1f, q) / 0.7071f;
    out[0] = { type, 0.5412f };
    out[1] = { type, 1.3066f * qScale };
    return 2;
}

void FilterModule::applyControl(float freq, float q, float gainDb)
{
    Section plan[2];
    const int n = planSections(type, slope, q, plan);

    // Topology change (slope/type switch) — clear state so stale samples
    // from the previous configuration can't leak into the new cascade.
    if (n != numSections)
        for (auto& s : sections) s.reset();

    numSections = n;
    for (int i = 0; i < n; ++i)
    {
        sections[i].setType(plan[i].type);
        sections[i].setFrequency(freq);
        sections[i].setQ(juce::jmax(0.05f, plan[i].q));
        sections[i].setGainDb(gainDb);
    }
}

void FilterModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamples  = buffer.getNumSamples();

    // Dry copy for the mix blend
    dryBuffer.setSize(numChannels, numSamples, false, false, true);
    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    int i = 0;
    while (i < numSamples)
    {
        const int n = std::min(kCtrlInterval, numSamples - i);

        // Advance control-rate values and refresh coefficients in small,
        // inaudible steps instead of one jump per block.
        const float fCur = freqSm.skip(n);
        const float qCur = qSm.skip(n);
        const float gCur = gainSm.skip(n);
        applyControl(fCur, qCur, gCur);

        for (int s = i; s < i + n; ++s)
        {
            // Drive: tanh input saturation, amount-crossfaded so 0 dB is fully
            // clean, ceiling-normalized so peaks stay put as drive increases.
            const float dDb  = driveSm.getNextValue();
            const float amt  = dDb / 24.0f;
            const float dLin = juce::Decibels::decibelsToGain(dDb);
            const float norm = 1.0f / tanhApprox(dLin);

            const float w   = mixSm.getNextValue();
            const float out = outSm.getNextValue();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float x = buffer.getSample(ch, s);

                if (amt > 0.0001f)
                    x += amt * (tanhApprox(x * dLin) * norm - x);

                for (int sec = 0; sec < numSections; ++sec)
                    x = sections[sec].processSample(x, ch);

                const float dry = dryBuffer.getSample(ch, s);
                buffer.setSample(ch, s, (dry + w * (x - dry)) * out);
            }
        }
        i += n;
    }
}

float FilterModule::responseDb(FilterType type, int slope, float freq, float q,
                               float gainDb, float atFreqHz, double sampleRate)
{
    Section plan[2];
    const int n = planSections(type, juce::jlimit(0, 1, slope), q, plan);

    double db = 0.0;
    for (int i = 0; i < n; ++i)
    {
        double c[5];
        BiquadFilter::calcCoefficients(plan[i].type, sampleRate, freq,
                                       juce::jmax(0.05f, plan[i].q), gainDb, c);
        db += BiquadFilter::magnitudeDb(c, (double)atFreqHz, sampleRate);
    }
    return (float)db;
}
