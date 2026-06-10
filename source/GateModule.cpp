#include "GateModule.h"

void GateModule::updateCoeffs()
{
    attackCoeff  = std::exp(-1.0f / (attackMs  * 0.001f * (float)sampleRate_));
    releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * (float)sampleRate_));
}

void GateModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    sampleRate_ = sampleRate;
    smoothedGain = 1.0f;
    updateCoeffs();
}

void GateModule::reset()
{
    smoothedGain = 1.0f;
}

void GateModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();
    const float threshold = juce::Decibels::decibelsToGain(thresholdDb);
    const float rangeGain = juce::Decibels::decibelsToGain(rangeDb);

    for (int i = 0; i < numSamples; ++i)
    {
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            peak = std::max(peak, std::abs(buffer.getSample(ch, i)));

        const float targetGain = peak >= threshold ? 1.0f : rangeGain;
        const float coeff = targetGain > smoothedGain ? attackCoeff : releaseCoeff;
        smoothedGain = targetGain + coeff * (smoothedGain - targetGain);

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.setSample(ch, i, buffer.getSample(ch, i) * smoothedGain);
    }
}
