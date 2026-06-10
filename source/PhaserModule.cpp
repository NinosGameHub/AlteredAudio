#include "PhaserModule.h"

static constexpr double kPi = 3.14159265358979323846;

void PhaserModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    sampleRate_ = sampleRate;
    lfo.prepare(sampleRate);
    reset();
}

void PhaserModule::reset()
{
    for (int s = 0; s < kMaxStages; ++s)
        for (int ch = 0; ch < kMaxChannels; ++ch)
            apState[s][ch] = 0.0f;
    for (int ch = 0; ch < kMaxChannels; ++ch)
        feedbackSample[ch] = 0.0f;
    lfo.reset();
}


void PhaserModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamples  = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        const float lfoVal = lfo.tick();
        // LFO modulates frequency from baseFreq to baseFreq * (1 + depth * 8)
        const float modFreq = baseFreq * (1.0f + depth * 7.0f * (lfoVal * 0.5f + 0.5f));
        const float omega   = (float)(kPi * (double)modFreq / sampleRate_);
        const float c       = (std::tan(omega) - 1.0f) / (std::tan(omega) + 1.0f);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float dry  = buffer.getSample(ch, i);
            float x = dry + feedbackSample[ch] * feedback;

            for (int s = 0; s < numStages; ++s)
            {
                // Direct form II: H(z) = (c + z^-1) / (1 + c*z^-1)
                const float w  = x - c * apState[s][ch];
                x              = c * w + apState[s][ch];
                apState[s][ch] = w;
            }

            feedbackSample[ch] = x;
            buffer.setSample(ch, i, dry * (1.0f - wetDry) + x * wetDry);
        }
    }
}
