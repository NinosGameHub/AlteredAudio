#include "GainModule.h"

void GainModule::setGainDb(float db)
{
    targetLinear = juce::Decibels::decibelsToGain(juce::jlimit(-60.0f, 24.0f, db));
    smoother.setTargetValue(targetLinear);
}

void GainModule::prepare(double sampleRate, int)
{
    smoother.reset(sampleRate, 0.02);
    smoother.setCurrentAndTargetValue(targetLinear);
}

void GainModule::reset()
{
    smoother.setCurrentAndTargetValue(targetLinear);
}

void GainModule::process(juce::AudioBuffer<float>& buffer)
{
    if (!smoother.isSmoothing())
    {
        buffer.applyGain(smoother.getCurrentValue());
        return;
    }

    const int numCh = buffer.getNumChannels();
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float g = smoother.getNextValue();
        for (int ch = 0; ch < numCh; ++ch)
            buffer.setSample(ch, i, buffer.getSample(ch, i) * g);
    }
}
