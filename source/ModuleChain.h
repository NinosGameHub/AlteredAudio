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

private:
    std::vector<std::unique_ptr<EffectModule>> modules;
    std::array<int, kMaxModules> processingOrder {};
    int numModules = 0;

    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;
};
