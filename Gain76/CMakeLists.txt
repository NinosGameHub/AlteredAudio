# ============================================================
#  Gain 76 — fully self-contained plugin project.
#  Shares NOTHING with the AlteredAudio rack / other modules:
#  all sources live in Gain76/Source/.
#
#  Configure once:  cmake -S Gain76 -B build-gain76
#  Build:           cmake --build build-gain76 --config Release --target Gain76_VST3
#  Install:         tools\install-vst3.ps1 -Only "AlteredAudio Gain 76.vst3"
#
#  LTO is intentionally off for fast iteration links.
# ============================================================
cmake_minimum_required(VERSION 3.24)
project(Gain76 VERSION 0.8.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("C:/JUCE" "${CMAKE_BINARY_DIR}/JUCE")

juce_add_plugin(Gain76
    COMPANY_NAME            "AlteredAudio"
    PLUGIN_MANUFACTURER_CODE Aalt
    PLUGIN_CODE             AGn1
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS                 VST3
    PLUGIN_VERSION          "${PROJECT_VERSION}"
    PRODUCT_NAME            "AlteredAudio Gain 76"
    VST3_CATEGORIES         "Fx"
)

juce_generate_juce_header(Gain76)

target_sources(Gain76 PRIVATE
    Source/GainProcessor.cpp
    Source/GainEditor.cpp
)

target_compile_definitions(Gain76 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Gain76 PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
