#pragma once
#include <JuceHeader.h>

class EnvelopeFollower
{
public:
    void prepare(double sampleRate, int blockSize);
    void reset();

    // Processes the buffer; updates internal smoothed level
    void processBlock(const juce::AudioBuffer<float>& buffer);

    // Returns current smoothed amplitude in [0, 1]
    float getLevel() const { return level; }

    void setAttackMs (float ms) { attackMs  = ms; updateCoeffs(); }
    void setReleaseMs(float ms) { releaseMs = ms; updateCoeffs(); }

private:
    void  updateCoeffs();
    float computeRMS(const juce::AudioBuffer<float>& buffer) const;

    float  attackMs    = 10.0f;
    float  releaseMs   = 100.0f;
    float  attackCoeff  = 0.0f;
    float  releaseCoeff = 0.0f;
    float  level        = 0.0f;
    double sampleRate_  = 44100.0;
};
