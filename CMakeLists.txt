# ============================================================
#  Filter 76 — fully independent plugin project.
#  Own JUCE copy (./JUCE), own build dir (./build), outside
#  OneDrive. Nothing here depends on the AlteredAudio repo.
#
#  First-time:  cmake -B build -S .
#  Build:       cmake --build build --config Release --target Filter76_VST3
#  Install:     AlteredAudio\tools\install-vst3.ps1 -Only "AlteredAudio Filter 76.vst3"
#
#  PLUGIN_CODE stays AFlt (the code DAW sessions already know) —
#  do not change it. LTO is off for fast iteration links.
# ============================================================
cmake_minimum_required(VERSION 3.24)
project(Filter76 VERSION 1.0.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Local JUCE — no external dependency
add_subdirectory(JUCE)

juce_add_plugin(Filter76
    COMPANY_NAME            "AlteredAudio"
    PLUGIN_MANUFACTURER_CODE Aalt
    PLUGIN_CODE             AFlt
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS                 VST3
    PLUGIN_VERSION          "${PROJECT_VERSION}"
    PRODUCT_NAME            "AlteredAudio Filter 76"
    VST3_CATEGORIES         "Fx Filter"
)

juce_generate_juce_header(Filter76)

juce_add_binary_data(Filter76BinaryData SOURCES
    Resources/knob.png
)

target_sources(Filter76 PRIVATE
    Source/FilterPlugin.cpp
    Source/AuroraFilterEditor.cpp
    Source/FilterModule.cpp
    Source/BiquadFilter.cpp
    Source/SingleModuleProcessor.cpp
)

target_compile_definitions(Filter76 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Filter76 PRIVATE
    Filter76BinaryData
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
