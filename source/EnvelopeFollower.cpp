#include "EnvelopeFollower.h"
#include <cmath>

void EnvelopeFollower::prepare(double sampleRate, int)
{
    sampleRate_ = sampleRate;
    updateCoeffs();
    reset();
}

void EnvelopeFollower::reset()
{
    level = 0.0f;
}

void EnvelopeFollower::updateCoeffs()
{
    const float a = attackMs  * 0.001f;
    const float r = releaseMs * 0.001f;
    attackCoeff  = (a > 0.0f) ? std::exp(-1.0f / (float)(sampleRate_ * a)) : 0.0f;
    releaseCoeff = (r > 0.0f) ? std::exp(-1.0f / (float)(sampleRate_ * r)) : 0.0f;
}

float EnvelopeFollower::computeRMS(const juce::AudioBuffer<float>& buffer) const
{
    const int ch = buffer.getNumChannels();
    const int n  = buffer.getNumSamples();
    if (ch == 0 || n == 0) return 0.0f;

    double sum = 0.0;
    for (int c = 0; c < ch; ++c)
    {
        const float* d = buffer.getReadPointer(c);
        for (int i = 0; i < n; ++i)
            sum += (double)(d[i] * d[i]);
    }
    return std::sqrt((float)(sum / (ch * n)));
}

void EnvelopeFollower::processBlock(const juce::AudioBuffer<float>& buffer)
{
    const float rms   = computeRMS(buffer);
    const float coeff = (rms > level) ? attackCoeff : releaseCoeff;
    level = coeff * level + (1.0f - coeff) * rms;
}
