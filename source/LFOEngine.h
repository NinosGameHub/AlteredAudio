#pragma once
#include <JuceHeader.h>

class LFOEngine
{
public:
    enum class Shape { Sine, Triangle, Square, Sawtooth, RandomSH };

    void prepare(double sampleRate);
    void reset();
    float tick(); // advance one sample, returns [-1, 1]

    void setRate(float hz)        { rate = hz; updateInc(); }
    void setShape(Shape s)        { shape = s; }
    void setPhaseOffset(float p)  { phaseOffset = p; } // 0..1

    // Advance phase by exactly numSamples steps — O(1), no inner loop
    float tickBlock(int numSamples) noexcept
    {
        const float advance  = phaseInc * (float)numSamples;
        const float newPhase = phase + advance;
        if (shape == Shape::RandomSH && newPhase >= 1.0f)
            randomValue = random.nextFloat() * 2.0f - 1.0f;
        phase = newPhase - std::floor(newPhase);
        return computeAt(std::fmod(phase + phaseOffset, 1.0f));
    }

private:
    void updateInc() noexcept { phaseInc = rate / (float)sampleRate_; }

    static constexpr float kTwoPi = 6.28318530718f;

    float computeAt(float p) const noexcept
    {
        switch (shape)
        {
            case Shape::Sine:     return std::sin(kTwoPi * p);
            case Shape::Triangle: return p < 0.5f ? 4.0f * p - 1.0f : 3.0f - 4.0f * p;
            case Shape::Square:   return p < 0.5f ? 1.0f : -1.0f;
            case Shape::Sawtooth: return 2.0f * p - 1.0f;
            case Shape::RandomSH: return randomValue;
            default:              return 0.0f;
        }
    }

    Shape  shape       = Shape::Sine;
    float  rate        = 1.0f;
    float  phase       = 0.0f;
    float  phaseInc    = 0.0f;
    float  phaseOffset = 0.0f;
    float  randomValue = 0.0f;
    double sampleRate_ = 44100.0;
    juce::Random random;
};
