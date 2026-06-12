#pragma once
#include <JuceHeader.h>

class EffectModule
{
public:
    virtual ~EffectModule() = default;

    virtual void prepare(double sampleRate, int blockSize) = 0;
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;
    virtual void reset() = 0;
    virtual juce::String getName() const = 0;

    // Samples of delay this module introduces (oversampling, lookahead...).
    // The processor sums these for host latency compensation.
    virtual int getLatencySamples() const { return 0; }

    void setBypassed(bool bypassed) { bypassed_ = bypassed; }
    bool isBypassed() const         { return bypassed_; }

private:
    bool bypassed_ = false;
};
