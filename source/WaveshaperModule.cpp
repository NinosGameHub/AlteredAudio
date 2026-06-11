#include "WaveshaperModule.h"

void WaveshaperModule::prepare(double sampleRate, int blockSize)
{
    oversampling.initProcessing((size_t)blockSize);
    dryBuffer.setSize(2, blockSize, false, true, true);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)blockSize, 2 };
    dryDelay.prepare(spec);
    dryDelay.setDelay(oversampling.getLatencyInSamples());

    // Drive is consumed per-sample in the 2x oversampled domain
    driveSm.reset(sampleRate * 2.0, 0.02);
    driveSm.setCurrentAndTargetValue(drive);
    wetDrySm.reset(sampleRate, 0.02);
    wetDrySm.setCurrentAndTargetValue(wetDry);
}

void WaveshaperModule::reset()
{
    oversampling.reset();
    dryDelay.reset();
    driveSm.setCurrentAndTargetValue(drive);
    wetDrySm.setCurrentAndTargetValue(wetDry);
}

float WaveshaperModule::processSample(float x, float driveAmt) const
{
    const float driven = x * driveAmt;
    switch (algorithm)
    {
        case Algorithm::SoftClip:
            return driven / (1.0f + std::abs(driven));
        case Algorithm::HardClip:
            return juce::jlimit(-1.0f, 1.0f, driven);
        case Algorithm::TanhSat:
            return std::tanh(driven);
        case Algorithm::BitCrush:
        {
            const float steps = std::pow(2.0f, (float)bitDepth);
            return std::round(driven * steps) / steps;
        }
        case Algorithm::FoldBack:
        {
            float y = driven;
            while (y > 1.0f || y < -1.0f)
            {
                if (y >  1.0f) y =  2.0f - y;
                if (y < -1.0f) y = -2.0f - y;
            }
            return y;
        }
        default: return x;
    }
}

void WaveshaperModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    // Save dry signal (pre-allocated, no heap alloc here)
    dryBuffer.setSize(numChannels, numSamples, false, false, true);
    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    // Align dry path with the oversampler's group delay so the blend
    // below stays phase-coherent (no comb filtering at partial mixes).
    for (int ch = 0; ch < std::min(numChannels, 2); ++ch)
    {
        auto* d = dryBuffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            dryDelay.pushSample(ch, d[i]);
            d[i] = dryDelay.popSample(ch);
        }
    }

    // Upsample → process → downsample
    juce::dsp::AudioBlock<float> block(buffer);
    auto oversampledBlock = oversampling.processSamplesUp(block);

    const int osNumSamples  = (int)oversampledBlock.getNumSamples();
    const int osNumChannels = (int)oversampledBlock.getNumChannels();
    for (int i = 0; i < osNumSamples; ++i)
    {
        const float d = driveSm.getNextValue();
        for (int ch = 0; ch < osNumChannels; ++ch)
        {
            auto* data = oversampledBlock.getChannelPointer((size_t)ch);
            data[i] = processSample(data[i], d);
        }
    }

    oversampling.processSamplesDown(block);

    // Wet/dry blend — per-sample while the mix is moving, fast path when static
    if (wetDrySm.isSmoothing())
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const float w = wetDrySm.getNextValue();
            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto*       out = buffer.getWritePointer(ch);
                const auto* dry = dryBuffer.getReadPointer(ch);
                out[i] = out[i] * w + dry[i] * (1.0f - w);
            }
        }
    }
    else
    {
        const float w = wetDrySm.getCurrentValue();
        if (w < 0.999f)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                buffer.applyGain(ch, 0, numSamples, w);
                buffer.addFrom(ch, 0, dryBuffer, ch, 0, numSamples, 1.0f - w);
            }
        }
    }
}
