#include "SingleModuleProcessor.h"

SingleModuleProcessor::SingleModuleProcessor(
    std::unique_ptr<EffectModule> module,
    juce::AudioProcessorValueTreeState::ParameterLayout layout)
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      module_(std::move(module)),
      apvts_(*this, nullptr, "Params", std::move(layout))
{}

void SingleModuleProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    module_->prepare(sampleRate, samplesPerBlock);
    dryBuffer_.setSize(2, samplesPerBlock, false, true, false);
}

void SingleModuleProcessor::releaseResources()
{
    module_->reset();
}

void SingleModuleProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    updateModuleParameters();

    const float targetGain = module_->isBypassed() ? 0.0f : 1.0f;
    const bool  needsRamp  = std::abs(bypassGain_ - targetGain) > 1e-6f;

    if (!needsRamp && targetGain == 0.0f)
        return;  // fully bypassed, nothing to do

    if (!needsRamp)
    {
        module_->process(buffer);
        return;
    }

    // Bypass state is changing — crossfade dry <-> wet over kRampSamples
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = std::min(buffer.getNumChannels(), dryBuffer_.getNumChannels());
    const int safe        = std::min(numSamples, dryBuffer_.getNumSamples());

    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer_.copyFrom(ch, 0, buffer, ch, 0, safe);

    module_->process(buffer);

    const float step = (targetGain > bypassGain_ ? 1.0f : -1.0f) / (float)kRampSamples;
    for (int s = 0; s < safe; ++s)
    {
        bypassGain_ = juce::jlimit(0.0f, 1.0f, bypassGain_ + step);
        const float dryAmt = 1.0f - bypassGain_;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float*       out = buffer.getWritePointer(ch);
            const float* dry = dryBuffer_.getReadPointer(ch);
            out[s] = dry[s] * dryAmt + out[s] * bypassGain_;
        }
    }
}

juce::AudioProcessorEditor* SingleModuleProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void SingleModuleProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SingleModuleProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts_.state.getType()))
        apvts_.replaceState(juce::ValueTree::fromXml(*xml));
}
