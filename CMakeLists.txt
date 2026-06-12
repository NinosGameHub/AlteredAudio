cmake_minimum_required(VERSION 3.24)
project(AlteredAudio VERSION 0.7.2)

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
    COPY_PLUGIN_AFTER_BUILD TRUE
    FORMATS VST3
    PLUGIN_VERSION "0.7.2"
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
    source/SpatialModule.cpp
    source/ChainStrip.cpp
    source/ModulePanels.cpp
    source/CRTDisplay.cpp
    source/ModuleTileList.cpp
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

# ── Helper macro — avoids repeating boilerplate for every single-module plugin ──
macro(aa_add_plugin TARGET_NAME PLUGIN_CODE_VAL PRODUCT_NAME_VAL VST3_CAT)
    juce_add_plugin(${TARGET_NAME}
        COMPANY_NAME            "AlteredAudio"
        PLUGIN_MANUFACTURER_CODE Aalt
        PLUGIN_CODE             ${PLUGIN_CODE_VAL}
        IS_SYNTH                FALSE
        NEEDS_MIDI_INPUT        FALSE
        NEEDS_MIDI_OUTPUT       FALSE
        IS_MIDI_EFFECT          FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        FORMATS                 VST3
        PRODUCT_NAME            ${PRODUCT_NAME_VAL}
        VST3_CATEGORIES         ${VST3_CAT}
    )
    juce_generate_juce_header(${TARGET_NAME})
    target_compile_definitions(${TARGET_NAME} PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
    )
    target_link_libraries(${TARGET_NAME} PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )
endmacro()

# Shared sources compiled into every single-module plugin
set(AA_SINGLE_MODULE_BASE
    source/SingleModuleProcessor.cpp
)

# ── Single-module plugins ────────────────────────────────────────────────────

aa_add_plugin(AlteredAudio_EQ             AEq1 "AlteredAudio EQ"              "Fx EQ")
target_sources(AlteredAudio_EQ PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/BiquadFilter.cpp
    source/EQModule.cpp
    source/plugins/EQPlugin.cpp
)

aa_add_plugin(AlteredAudio_Filter         AFlt "AlteredAudio Filter 76"        "Fx Filter")
target_sources(AlteredAudio_Filter PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/BiquadFilter.cpp
    source/FilterModule.cpp
    source/plugins/FilterPlugin.cpp
    source/plugins/AuroraFilterEditor.cpp
)

aa_add_plugin(AlteredAudio_Gain           AGn1 "AlteredAudio Gain"             "Fx")
target_sources(AlteredAudio_Gain PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/GainModule.cpp
    source/plugins/GainPlugin.cpp
)

aa_add_plugin(AlteredAudio_Delay          ADly "AlteredAudio Delay"            "Fx Delay")
target_sources(AlteredAudio_Delay PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/DelayModule.cpp
    source/plugins/DelayPlugin.cpp
)

aa_add_plugin(AlteredAudio_Reverb         ARvb "AlteredAudio Reverb"           "Fx Reverb")
target_sources(AlteredAudio_Reverb PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/ReverbModule.cpp
    source/plugins/ReverbPlugin.cpp
)

aa_add_plugin(AlteredAudio_Waveshaper     AWvs "AlteredAudio Waveshaper"       "Fx Distortion")
target_sources(AlteredAudio_Waveshaper PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/WaveshaperModule.cpp
    source/plugins/WaveshaperPlugin.cpp
)

aa_add_plugin(AlteredAudio_Compressor     ACmp "AlteredAudio Compressor"       "Fx Dynamics Compressor")
target_sources(AlteredAudio_Compressor PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/CompressorModule.cpp
    source/plugins/CompressorPlugin.cpp
)

aa_add_plugin(AlteredAudio_Clipper        AClp "AlteredAudio Clipper"          "Fx Distortion Dynamics")
target_sources(AlteredAudio_Clipper PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/ClipperModule.cpp
    source/plugins/ClipperPlugin.cpp
)

aa_add_plugin(AlteredAudio_TransientShaper ATs1 "AlteredAudio Transient Shaper" "Fx Dynamics")
target_sources(AlteredAudio_TransientShaper PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/TransientShaperModule.cpp
    source/plugins/TransientShaperPlugin.cpp
)

aa_add_plugin(AlteredAudio_Limiter        ALim "AlteredAudio Limiter"          "Fx Dynamics Limiter")
target_sources(AlteredAudio_Limiter PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/LimiterModule.cpp
    source/plugins/LimiterPlugin.cpp
)

aa_add_plugin(AlteredAudio_Gate           AGte "AlteredAudio Gate"             "Fx Dynamics")
target_sources(AlteredAudio_Gate PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/GateModule.cpp
    source/plugins/GatePlugin.cpp
)

aa_add_plugin(AlteredAudio_Chorus         AChr "AlteredAudio Chorus"           "Fx Modulation")
target_sources(AlteredAudio_Chorus PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/ChorusModule.cpp
    source/plugins/ChorusPlugin.cpp
)

aa_add_plugin(AlteredAudio_Flanger        AFln "AlteredAudio Flanger"          "Fx Modulation")
target_sources(AlteredAudio_Flanger PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/FlangerModule.cpp
    source/plugins/FlangerPlugin.cpp
)

aa_add_plugin(AlteredAudio_Phaser         APhs "AlteredAudio Phaser"           "Fx Modulation")
target_sources(AlteredAudio_Phaser PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/PhaserModule.cpp
    source/plugins/PhaserPlugin.cpp
)

aa_add_plugin(AlteredAudio_Spatial        ASpt "AlteredAudio Spatial"          "Fx Spatial")
target_sources(AlteredAudio_Spatial PRIVATE
    ${AA_SINGLE_MODULE_BASE}
    source/SpatialModule.cpp
    source/plugins/SpatialPlugin.cpp
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
