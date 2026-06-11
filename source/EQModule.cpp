#include "EQModule.h"

void EQModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    currentSampleRate = sampleRate;
    for (int b = 0; b < kMaxBands; ++b)
        for (int ch = 0; ch < kMaxChannels; ++ch)
            bands[b][ch].prepare(sampleRate);
}

void EQModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamples  = buffer.getNumSamples();

    for (int b = 0; b < kMaxBands; ++b)
    {
        if (!params[b].enabled) continue;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                data[i] = bands[b][ch].processSample(data[i], 0);
        }
    }
}

void EQModule::reset()
{
    for (int b = 0; b < kMaxBands; ++b)
        for (int ch = 0; ch < kMaxChannels; ++ch)
            bands[b][ch].reset();
}

void EQModule::setBand(int index, const BandParams& p)
{
    if (index < 0 || index >= kMaxBands) return;
    params[index] = p;
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        bands[index][ch].setType(p.type);
        bands[index][ch].setFrequency(p.frequency);
        bands[index][ch].setQ(p.q);
        bands[index][ch].setGainDb(p.gainDb);
    }
}
