#pragma once
#include <JuceHeader.h>
#include "EffectModule.h"

// Base AudioProcessor for single-module standalone plugins.
// Subclasses provide a module instance, a parameter layout, and
// override updateModuleParameters() to push APVTS values into the module.

class SingleModuleProcessor : public juce::AudioProcessor
{
public:
    SingleModuleProcessor(std::unique_ptr<EffectModule> module,
                          juce::AudioProcessorValueTreeState::ParameterLayout layout);

    // ---- AudioProcessor interface ----
    void prepareToPlay  (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock   (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return module_->getName(); }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms()                              override { return 1; }
    int getCurrentProgram()                           override { return 0; }
    void setCurrentProgram(int)                       override {}
    const juce::String getProgramName(int)            override { return {}; }
    void changeProgramName(int, const juce::String&)  override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts_; }

protected:
    // Subclasses read from apvts_ and call module setters here.
    // Called once per processBlock, before the module runs.
    virtual void updateModuleParameters() = 0;

    // Additional fixed latency a subclass introduces around the module
    // (e.g. oversampling filters) — included in host PDC reporting.
    virtual int getExtraLatencySamples() const { return 0; }

    std::unique_ptr<EffectModule>           module_;
    juce::AudioProcessorValueTreeState      apvts_;

private:
    // Bypass crossfade — same approach as ModuleChain to avoid clicks.
    static constexpr int kRampSamples = 64;
    float bypassGain_ = 0.0f;
    juce::AudioBuffer<float> dryBuffer_;   // pre-allocated in prepareToPlay

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SingleModuleProcessor)
};
