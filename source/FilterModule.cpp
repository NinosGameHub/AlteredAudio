#include "FilterModule.h"

void FilterModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    for (auto& f : filters) f.prepare(sampleRate);
}

void FilterModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamples  = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] = filters[ch].processSample(data[i], 0);
    }
}

void FilterModule::reset()
{
    for (auto& f : filters) f.reset();
}
