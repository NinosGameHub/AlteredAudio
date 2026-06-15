# Claude Code Instructions — Gain76 UI Wiring

## What we're doing
Adding a custom vintage UI (AlteredAudioLookAndFeel) to Gain76. Three files are ready. 
Your job is to drop them in and fix any compile errors.

---

## Step 1 — Copy new files into Source/

Copy these two files into `Source/`:
- `AlteredAudioLookAndFeel.h`
- `PluginEditor.h` (replace the existing one)

---

## Step 2 — Update PluginProcessor.h

Add to the **public** section of the `Gain76AudioProcessor` class:

```cpp
float getInputLevel()  { return inputLevel.load();  }
float getOutputLevel() { return outputLevel.load(); }
```

Add to the **private** section:

```cpp
std::atomic<float> inputLevel  { 0.0f };
std::atomic<float> outputLevel { 0.0f };
float inputLevelSmoothed  = 0.0f;
float outputLevelSmoothed = 0.0f;
static constexpr float LEVEL_DECAY = 0.85f;
```

---

## Step 3 — Update PluginProcessor.cpp

Replace the body of `processBlock()` with this:

```cpp
void Gain76AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Measure input level
    float rawInputLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        rawInputLevel = juce::jmax (rawInputLevel,
                                    buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
    inputLevelSmoothed = rawInputLevel > inputLevelSmoothed
        ? rawInputLevel : inputLevelSmoothed * LEVEL_DECAY;
    inputLevel.store (inputLevelSmoothed);

    // Apply gain
    auto* gainParam = apvts.getRawParameterValue ("GAIN");
    float gainLinear = juce::Decibels::decibelsToGain (gainParam->load());
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* channelData = buffer.getWritePointer (ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
            channelData[s] *= gainLinear;
    }

    // Measure output level
    float rawOutputLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        rawOutputLevel = juce::jmax (rawOutputLevel,
                                     buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
    outputLevelSmoothed = rawOutputLevel > outputLevelSmoothed
        ? rawOutputLevel : outputLevelSmoothed * LEVEL_DECAY;
    outputLevel.store (outputLevelSmoothed);
}
```

Make sure `createParameterLayout()` includes the GAIN parameter:

```cpp
params.push_back (std::make_unique<juce::AudioParameterFloat> (
    juce::ParameterID { "GAIN", 1 },
    "Gain",
    juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
    0.0f,
    juce::AudioParameterFloatAttributes().withLabel ("dB")
));
```

---

## Step 4 — Build

```bash
cmake --build build --config Release
```

Fix any compile errors that come up — most likely missing includes or minor type mismatches.

---

## Step 5 — Deploy and test

```bash
xcopy /Y "build\Gain76_artefacts\Release\VST3\Gain76.vst3" "C:\Program Files\Common Files\VST3\" /E /I
```

Rescan plugins in Cubase or Bitwig and open the plugin. You should see the vintage cream UI with the large bakelite knob, amber VU meters, and stereo bar.
