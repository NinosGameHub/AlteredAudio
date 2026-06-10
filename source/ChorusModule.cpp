#include "ChorusModule.h"

void ChorusModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    sampleRate_ = sampleRate;
    bufferSize  = (int)(kMaxDelayMs / 1000.0f * (float)sampleRate) + 1;

    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        delayBuf[ch].assign(bufferSize, 0.0f);
        writePos[ch] = 0;
    }

    for (int v = 0; v < kMaxVoices; ++v)
    {
        lfos[v].prepare(sampleRate);
        lfos[v].setPhaseOffset((float)v / (float)kMaxVoices);
    }
}

void ChorusModule::reset()
{
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        std::fill(delayBuf[ch].begin(), delayBuf[ch].end(), 0.0f);
        writePos[ch] = 0;
    }
    for (auto& l : lfos) l.reset();
}

float ChorusModule::readInterpolated(const std::vector<float>& buf, int wp, float delaySamples) const
{
    int   a    = wp - (int)delaySamples;
    float frac = delaySamples - (int)delaySamples;
    int   b    = a - 1;
    while (a < 0) a += bufferSize;
    while (b < 0) b += bufferSize;
    return buf[a % bufferSize] * (1.0f - frac) + buf[b % bufferSize] * frac;
}

void ChorusModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamples  = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        // One LFO tick per sample (advance all voices together)
        float lfoValues[kMaxVoices];
        for (int v = 0; v < numVoices; ++v)
            lfoValues[v] = lfos[v].tick();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float dry = buffer.getSample(ch, i);
            delayBuf[ch][writePos[ch]] = dry;

            float wet = 0.0f;
            for (int v = 0; v < numVoices; ++v)
            {
                const float modMs   = centreMs + lfoValues[v] * depthMs;
                const float modSamp = modMs * 0.001f * (float)sampleRate_;
                wet += readInterpolated(delayBuf[ch], writePos[ch], modSamp);
            }
            wet /= (float)numVoices;

            writePos[ch] = (writePos[ch] + 1) % bufferSize;
            buffer.setSample(ch, i, dry * (1.0f - wetDry) + wet * wetDry);
        }
    }
}
