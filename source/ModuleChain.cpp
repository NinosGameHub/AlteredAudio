#include "ModuleChain.h"

void ModuleChain::prepare(double sampleRate, int blockSize)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = blockSize;

    for (auto& module : modules)
        module->prepare(sampleRate, blockSize);
}

void ModuleChain::process(juce::AudioBuffer<float>& buffer)
{
    for (int i = 0; i < numModules; ++i)
    {
        const int idx = processingOrder[i];
        if (idx >= 0 && idx < (int)modules.size() && !modules[idx]->isBypassed())
            modules[idx]->process(buffer);
    }
}

void ModuleChain::reset()
{
    for (auto& module : modules)
        module->reset();
}

EffectModule* ModuleChain::addModule(std::unique_ptr<EffectModule> module)
{
    module->prepare(currentSampleRate, currentBlockSize);
    auto* raw = module.get();
    const int idx = (int)modules.size();
    modules.push_back(std::move(module));
    if (numModules < kMaxModules)
        processingOrder[numModules++] = idx;
    return raw;
}

void ModuleChain::setProcessingOrder(const int* order, int count) noexcept
{
    const int n = std::min(count, kMaxModules);
    for (int i = 0; i < n; ++i)
        processingOrder[i] = order[i];
    numModules = n;
}

void ModuleChain::removeModule(int index)
{
    if (index >= 0 && index < (int) modules.size())
        modules.erase(modules.begin() + index);
}
