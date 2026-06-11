#include "ModuleChain.h"

void ModuleChain::prepare(double sampleRate, int blockSize)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = blockSize;

    // Pre-allocate scratch buffer — stereo, worst-case block size.
    // Done here (not in process) so the audio thread never allocates.
    scratchBuffer.setSize(2, blockSize, false, true, false);

    for (auto& module : modules)
        module->prepare(sampleRate, blockSize);
}

void ModuleChain::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = std::min(buffer.getNumChannels(),
                                     scratchBuffer.getNumChannels());

    for (int i = 0; i < numModules; ++i)
    {
        const int idx = processingOrder[i];
        if (idx < 0 || idx >= (int)modules.size()) continue;

        auto*       mod         = modules[idx].get();
        const float targetGain  = mod->isBypassed() ? 0.0f : 1.0f;
        float&      gain        = bypassGain[idx];
        const bool  needsRamp   = std::abs(gain - targetGain) > 1e-6f;

        // Fully bypassed and stable — skip processing entirely.
        if (!needsRamp && targetGain == 0.0f)
            continue;

        // Fully active and stable — process normally, no crossfade overhead.
        if (!needsRamp)
        {
            mod->process(buffer);
            continue;
        }

        // Bypass state is changing: crossfade dry ↔ wet over kRampSamples.
        // Capture dry signal into scratch (no allocation — scratch pre-allocated in prepare).
        const int safeSamples = std::min(numSamples, scratchBuffer.getNumSamples());
        for (int ch = 0; ch < numChannels; ++ch)
            scratchBuffer.copyFrom(ch, 0, buffer, ch, 0, safeSamples);

        // Run the module on the full buffer to get the wet signal.
        mod->process(buffer);

        // Blend dry and wet sample-by-sample as the gain ramps.
        const float step = (targetGain > gain ? 1.0f : -1.0f) / (float)kRampSamples;
        for (int s = 0; s < safeSamples; ++s)
        {
            gain = juce::jlimit(0.0f, 1.0f, gain + step);
            const float dryAmt = 1.0f - gain;
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float*       out = buffer.getWritePointer(ch);
                const float* dry = scratchBuffer.getReadPointer(ch);
                out[s] = dry[s] * dryAmt + out[s] * gain;
            }
        }
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

int ModuleChain::getLatencySamples() const noexcept
{
    int total = 0;
    for (int i = 0; i < numModules; ++i)
    {
        const int idx = processingOrder[i];
        if (idx < 0 || idx >= (int)modules.size()) continue;
        const auto* mod = modules[idx].get();
        if (!mod->isBypassed())
            total += mod->getLatencySamples();
    }
    return total;
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
