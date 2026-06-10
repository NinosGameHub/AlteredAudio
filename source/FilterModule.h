#pragma once
#include "EffectModule.h"
#include "BiquadFilter.h"

class FilterModule : public EffectModule
{
public:
    using FilterType = BiquadFilter::Type;

    void prepare(double sampleRate, int blockSize) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;
    juce::String getName() const override { return "Filter"; }

    void setType(FilterType t)  { for (auto& f : filters) f.setType(t); }
    void setFrequency(float hz) { for (auto& f : filters) f.setFrequency(hz); }
    void setQ(float q)          { for (auto& f : filters) f.setQ(q); }
    void setGainDb(float dB)    { for (auto& f : filters) f.setGainDb(dB); }

private:
    static constexpr int kMaxChannels = 2;
    BiquadFilter filters[kMaxChannels];
};
