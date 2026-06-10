cmake_minimum_required(VERSION 3.24)
project(AlteredAudio VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("C:/JUCE" "${CMAKE_BINARY_DIR}/JUCE")

juce_add_plugin(AlteredAudio
    COMPANY_NAME "AlteredAudio"
    PLUGIN_MANUFACTURER_CODE Aalt
    PLUGIN_CODE Arsp
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS VST3
    PRODUCT_NAME "AlteredAudio"
    VST3_CATEGORIES "Fx"
)

juce_generate_juce_header(AlteredAudio)

target_sources(AlteredAudio PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/PassthroughModule.cpp
    source/ModuleChain.cpp
    source/EnvelopeFollower.cpp
    source/ModulationMatrix.cpp
    source/BiquadFilter.cpp
    source/FilterModule.cpp
    source/EQModule.cpp
    source/DelayModule.cpp
    source/ReverbModule.cpp
    source/WaveshaperModule.cpp
    source/CompressorModule.cpp
    source/ClipperModule.cpp
    source/TransientShaperModule.cpp
    source/LimiterModule.cpp
    source/GateModule.cpp
    source/GainModule.cpp
    source/LFOEngine.cpp
    source/ChorusModule.cpp
    source/FlangerModule.cpp
    source/PhaserModule.cpp
    source/ChainStrip.cpp
    source/ModulePanels.cpp
)

target_compile_definitions(AlteredAudio PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AlteredAudio PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ── Latency Tester standalone tool ──────────────────────────────────────────

juce_add_gui_app(LatencyTester
    PRODUCT_NAME "Latency Tester"
)

juce_generate_juce_header(LatencyTester)

target_sources(LatencyTester PRIVATE
    tools/LatencyTester.cpp
)

target_compile_definitions(LatencyTester PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(LatencyTester PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_basics
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
