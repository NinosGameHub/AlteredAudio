#pragma once
#include "CompressorModule.h"

class LimiterModule : public CompressorModule
{
public:
    LimiterModule()
    {
        ratio     = 1000.0f;
        attackMs  = 0.5f;
        releaseMs = 50.0f;
        kneeDb    = 0.0f;
    }

    juce::String getName() const override { return "Limiter"; }

    void setCeiling(float dB) { thresholdDb = dB; }
};
