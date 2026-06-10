#include "LFOEngine.h"

void LFOEngine::prepare(double sampleRate)
{
    sampleRate_ = sampleRate;
    updateInc();
    reset();
}

void LFOEngine::reset()
{
    phase = 0.0f;
    randomValue = 0.0f;
}

float LFOEngine::tick()
{
    const float p = std::fmod(phase + phaseOffset, 1.0f);

    if (shape == Shape::RandomSH && phase + phaseInc >= 1.0f)
        randomValue = random.nextFloat() * 2.0f - 1.0f;

    const float output = computeAt(p);

    phase += phaseInc;
    if (phase >= 1.0f) phase -= 1.0f;

    return output;
}
