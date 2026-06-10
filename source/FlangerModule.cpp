#include "FlangerModule.h"

void FlangerModule::prepare(double sampleRate, int blockSize)
{
    juce::ignoreUnused(blockSize);
    sampleRate_ = sampleRate;
    bufferSize  = (int)(kMaxDelayMs / 1000.0f * (float)sampleRate) + 2;

    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        delayBuf[ch].assign(bufferSize, 0.0f);
        writePos[ch] = 0;
    }
    lfo.prepare(sampleRate);
}

void FlangerModule::reset()
{
    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        std::fill(delayBuf[ch].begin(), delayBuf[ch].end(), 0.0f);
        writePos[ch] = 0;
    }
    lfo.reset();
}

float FlangerModule::readInterpolated(const std::vector<float>& buf, int wp, float delaySamples) const
{
    int   a    = wp - (int)delaySamples;
    float frac = delaySamples - (int)delaySamples;
    int   b    = a - 1;
    while (a < 0) a += bufferSize;
    while (b < 0) b += bufferSize;
    return buf[a % bufferSize] * (1.0f - frac) + buf[b % bufferSize] * frac;
}

void FlangerModule::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = std::min(buffer.getNumChannels(), kMaxChannels);
    const int numSamples  = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        const float lfoVal   = lfo.tick();
        const float modMs    = centreMs + lfoVal * depthMs;
        const float modSamp  = juce::jlimit(0.1f, (float)(bufferSize - 2),
                                            modMs * 0.001f * (float)sampleRate_);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float dry     = buffer.getSample(ch, i);
            const float delayed = readInterpolated(delayBuf[ch], writePos[ch], modSamp);

            delayBuf[ch][writePos[ch]] = dry + delayed * feedback;
            writePos[ch] = (writePos[ch] + 1) % bufferSize;

            buffer.setSample(ch, i, dry * (1.0f - wetDry) + delayed * wetDry);
        }
    }
}
