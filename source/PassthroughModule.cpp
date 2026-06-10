#include "PassthroughModule.h"

void PassthroughModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(sampleRate, blockSize);
}

void PassthroughModule::process(juce::AudioBuffer<float>& buffer)
{
    juce::ignoreUnused(buffer); // audio passes through unchanged
}

void PassthroughModule::reset() {}

juce::String PassthroughModule::getName() const { return "Passthrough"; }
