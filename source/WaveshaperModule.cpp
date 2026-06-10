#include "WaveshaperModule.h"

void WaveshaperModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(sampleRate);
    oversampling.initProcessing((size_t)blockSize);
    dryBuffer.setSize(2, blockSize, false, true, true);
}

void WaveshaperModule::reset()
{
    oversampling.reset();
}

float WaveshaperModule::processSample(float x) const
{
    const float driven = x * drive;
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

    // Upsample → process → downsample
    juce::dsp::AudioBlock<float> block(buffer);
    auto oversampledBlock = oversampling.processSamplesUp(block);

    for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
    {
        auto* data = oversampledBlock.getChannelPointer(ch);
        for (size_t i = 0; i < oversampledBlock.getNumSamples(); ++i)
            data[i] = processSample(data[i]);
    }

    oversampling.processSamplesDown(block);

    // Wet/dry blend
    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.applyGain(ch, 0, numSamples, wetDry);
        buffer.addFrom(ch, 0, dryBuffer, ch, 0, numSamples, 1.0f - wetDry);
    }
}
