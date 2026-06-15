//==============================================================================
// ADD THESE TO YOUR PluginProcessor.h
// Inside the public section of your Gain76AudioProcessor class
//==============================================================================

// --- Paste these method declarations into PluginProcessor.h (public section) ---

float getInputLevel()  { return inputLevel.load();  }
float getOutputLevel() { return outputLevel.load(); }

// --- Paste these member variables into PluginProcessor.h (private section) ---

std::atomic<float> inputLevel  { 0.0f };
std::atomic<float> outputLevel { 0.0f };

// Smoothing — levels decay slowly so meters don't snap to zero
float inputLevelSmoothed  = 0.0f;
float outputLevelSmoothed = 0.0f;
static constexpr float LEVEL_DECAY = 0.85f; // per buffer, tweak to taste


//==============================================================================
// ADD THIS TO YOUR PluginProcessor.cpp
// Inside your processBlock() method
//==============================================================================

void Gain76AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // --- Measure input level BEFORE applying gain ---
    float rawInputLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        rawInputLevel = juce::jmax (rawInputLevel,
                                    buffer.getMagnitude (ch, 0, buffer.getNumSamples()));

    // Smooth input level
    inputLevelSmoothed = rawInputLevel > inputLevelSmoothed
        ? rawInputLevel
        : inputLevelSmoothed * LEVEL_DECAY;
    inputLevel.store (inputLevelSmoothed);

    // --- Apply gain ---
    auto* gainParam = apvts.getRawParameterValue ("GAIN");
    float gainLinear = juce::Decibels::decibelsToGain (gainParam->load());

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* channelData = buffer.getWritePointer (ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
            channelData[s] *= gainLinear;
    }

    // --- Measure output level AFTER applying gain ---
    float rawOutputLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        rawOutputLevel = juce::jmax (rawOutputLevel,
                                     buffer.getMagnitude (ch, 0, buffer.getNumSamples()));

    // Smooth output level
    outputLevelSmoothed = rawOutputLevel > outputLevelSmoothed
        ? rawOutputLevel
        : outputLevelSmoothed * LEVEL_DECAY;
    outputLevel.store (outputLevelSmoothed);
}


//==============================================================================
// ADD THIS TO YOUR PluginProcessor.cpp
// Inside createParameterLayout() — or wherever you define your APVTS parameters
//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout Gain76AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "GAIN", 1 },
        "Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")
    ));

    return { params.begin(), params.end() };
}
