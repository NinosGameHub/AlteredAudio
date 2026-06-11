#pragma once
#include "EffectModule.h"
#include <vector>
#include <memory>
#include <array>

class ModuleChain
{
public:
    static constexpr int kMaxModules = 16;

    void prepare(double sampleRate, int blockSize);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    EffectModule* addModule(std::unique_ptr<EffectModule> module);
    void removeModule(int index);
    int getNumModules() const          { return (int) modules.size(); }
    EffectModule* getModule(int index) { return modules[index].get(); }

    // Pass a sorted array of module indices to set the processing order.
    // Called from the audio thread — no heap allocation.
    void setProcessingOrder(const int* order, int count) noexcept;

    // Total latency of all active (non-bypassed) modules, for host PDC.
    int getLatencySamples() const noexcept;

private:
    static constexpr int kRampSamples = 64;   // ~1.4 ms at 44.1 kHz

    std::vector<std::unique_ptr<EffectModule>> modules;
    std::array<int, kMaxModules> processingOrder {};
    int numModules = 0;

    // Per-module bypass gain: 0.0 = fully bypassed, 1.0 = fully active.
    // Ramped over kRampSamples when bypass state changes to avoid clicks.
    float bypassGain[kMaxModules] {};

    // Scratch buffer for capturing dry signal during bypass crossfade.
    // Allocated in prepare() — never allocates on the audio thread.
    juce::AudioBuffer<float> scratchBuffer;

    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;
};
