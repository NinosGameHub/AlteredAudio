#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"

using APVTS  = juce::AudioProcessorValueTreeState;
using PID    = juce::ParameterID;
using NRange = juce::NormalisableRange<float>;

// ============================================================
// Parameter Layout
// ============================================================

APVTS::ParameterLayout AlteredAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    const juce::StringArray filterTypes  = { "LowPass","HighPass","BandPass","Notch","AllPass","Peak","LowShelf","HighShelf" };
    const juce::StringArray lfoShapes    = { "Sine","Triangle","Square","Sawtooth","Sample+Hold" };
    const juce::StringArray wsAlgos      = { "SoftClip","HardClip","TanhSat","BitCrush","FoldBack" };
    const juce::StringArray channelModes = { "Stereo","Mono","MidSide" };
    const juce::StringArray phaserStages = { "2","4","6","8" };

    using Grp = juce::AudioProcessorParameterGroup;

    // ---- Global ----
    {
        auto g = std::make_unique<Grp>("global", "Global", "|");
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::channelMode,1}, "Channel Mode", channelModes, 0));
        layout.add(std::move(g));
    }

    // ---- Filter ----
    {
        auto g = std::make_unique<Grp>("filter", "Filter", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::filterBypass,1}, "Bypass",     true));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::filterType,  1}, "Type",       filterTypes, 0));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterFreq,  1}, "Frequency",  NRange(20.0f, 20000.0f, 0.0f, 0.25f), 1000.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterQ,     1}, "Q",          NRange(0.1f, 10.0f, 0.0f, 0.5f),      0.707f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterGain,  1}, "Gain (dB)",  NRange(-24.0f, 24.0f),                 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::filterSlope, 1}, "Slope",      juce::StringArray{"12","24"},          0));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterDrive, 1}, "Drive (dB)", NRange(0.0f, 24.0f),                   0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterMix,   1}, "Mix",        NRange(0.0f, 1.0f),                    1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::filterOutput,1}, "Output (dB)",NRange(-24.0f, 24.0f),                 0.0f));
        layout.add(std::move(g));
    }

    // ---- EQ ----
    {
        auto g = std::make_unique<Grp>("eq", "EQ (8 Bands)", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>(PID{ParamID::eqBypass,1}, "Bypass", true));
        for (int n = 1; n <= EQModule::kMaxBands; ++n)
        {
            auto band = std::make_unique<Grp>("eq_band_"+juce::String(n), "Band "+juce::String(n), "|");
            band->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::eqEnabled(n),1}, "On",        false));
            band->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::eqType(n),   1}, "Type",      filterTypes, 5));
            band->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::eqFreq(n),   1}, "Frequency", NRange(20.0f, 20000.0f, 0.0f, 0.25f), 1000.0f));
            band->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::eqQ(n),      1}, "Q",         NRange(0.1f, 10.0f, 0.0f, 0.5f),      0.707f));
            band->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::eqGain(n),   1}, "Gain (dB)", NRange(-24.0f, 24.0f),                 0.0f));
            band->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::eqSlope(n),  1}, "Slope",     juce::StringArray{"6","12","18","24","48"}, 1));
            g->addChild(std::move(band));
        }
        layout.add(std::move(g));
    }

    // ---- Gain ----
    {
        auto g = std::make_unique<Grp>("gain", "Gain", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::gainBypass, 1}, "Bypass",   true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gainDb,     1}, "Gain (dB)",NRange(-60.0f, 24.0f, 0.0f, 1.5f), 0.0f));
        layout.add(std::move(g));
    }

    // ---- Delay ----
    {
        auto g = std::make_unique<Grp>("delay", "Delay", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::delayBypass,    1}, "Bypass",        true));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::delayTime,  1}, "Time L (ms)", NRange(0.0f, 2000.0f, 0.0f, 0.5f), 250.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::delayTimeR, 1}, "Time R (ms)", NRange(0.0f, 2000.0f, 0.0f, 0.5f), 250.0f));
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::delaySync,  1}, "Tempo Sync",  false));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::delayDivL,  1}, "Division L",  DelayModule::syncDivisionNames(), 4));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::delayDivR,  1}, "Division R",  DelayModule::syncDivisionNames(), 4));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::delayFeedback,  1}, "Feedback",      NRange(0.0f,   0.99f),              0.3f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayFbLPHz,    1}, "Tone LP Hz",    NRange(500.0f, 20000.0f, 0.0f, 0.4f), 6000.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayFbHPHz,    1}, "Tone HP Hz",    NRange(20.0f,   500.0f, 0.0f, 0.4f),    80.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayDucking,   1}, "Ducking",       NRange(0.0f,    1.0f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayDiffusion, 1}, "Diffusion",     NRange(0.0f,    1.0f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayWow,       1}, "Wow/Flutter",   NRange(0.0f,    1.0f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayModRate,   1}, "Mod Rate Hz",   NRange(0.01f,   5.0f),               1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayModDepth,  1}, "Mod Depth",     NRange(0.0f,    1.0f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::delayWetDry,    1}, "Wet/Dry",       NRange(0.0f,    1.0f),               0.5f));
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::delayPingPong,  1}, "Ping-Pong",     false));
        layout.add(std::move(g));
    }

    // ---- Reverb ----
    {
        auto g = std::make_unique<Grp>("reverb", "Reverb", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::reverbBypass,    1}, "Bypass",         true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbRoom,      1}, "Decay",          NRange(0.0f,   1.0f),   0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbDamp,      1}, "Damping",        NRange(0.0f,   1.0f),   0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbDiffusion, 1}, "Diffusion",      NRange(0.0f,   1.0f),   0.75f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbPreDelay,  1}, "Pre-Delay ms",   NRange(0.0f, 100.0f),  20.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbModRate,   1}, "Mod Rate Hz",    NRange(0.01f,  4.0f),   0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbErLevel,   1}, "ER Level",       NRange(0.0f,   1.0f),   0.3f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbLowCut,    1}, "Low Cut Hz",     NRange(20.0f, 500.0f), 80.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbShimmer,   1}, "Shimmer",        NRange(0.0f,   1.0f),   0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbPitchSemi, 1}, "Pitch Semitones",NRange(-24.0f,24.0f),  12.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::reverbWetDry,    1}, "Wet/Dry",        NRange(0.0f,   1.0f),   0.3f));
        layout.add(std::move(g));
    }

    // ---- Waveshaper ----
    {
        auto g = std::make_unique<Grp>("waveshaper", "Waveshaper", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::wsBypass,   1}, "Bypass",    true));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::wsAlgo,     1}, "Algorithm", wsAlgos, 2));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::wsDrive,    1}, "Drive",     NRange(1.0f, 100.0f, 0.0f, 0.3f), 1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::wsWetDry,   1}, "Wet/Dry",   NRange(0.0f, 1.0f),               1.0f));
        g->addChild(std::make_unique<juce::AudioParameterInt>   (PID{ParamID::wsBitDepth, 1}, "Bit Depth", 1, 24, 8));
        layout.add(std::move(g));
    }

    // ---- Compressor ----
    {
        auto g = std::make_unique<Grp>("compressor", "Compressor", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::compBypass,    1}, "Bypass",        true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compThreshold, 1}, "Threshold (dB)",NRange(-60.0f, 0.0f),             -12.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compRatio,     1}, "Ratio",         NRange(1.0f, 100.0f, 0.0f, 0.3f),  4.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compAttack,    1}, "Attack (ms)",   NRange(0.1f, 500.0f, 0.0f, 0.5f),  10.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compRelease,   1}, "Release (ms)",  NRange(1.0f, 5000.0f, 0.0f, 0.5f),100.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compKnee,       1}, "Knee (dB)",      NRange(0.0f, 24.0f),                 6.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compMakeup,    1}, "Makeup (dB)",    NRange(-12.0f, 12.0f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compRmsPeak,   1}, "Peak/RMS",       NRange(0.0f, 1.0f),                  0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compAutoRel,   1}, "Auto Release",   NRange(0.0f, 1.0f),                  0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compSaturation,1}, "Saturation",     NRange(0.0f, 1.0f),                  0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compScHP,      1}, "SC HP Hz",       NRange(20.0f, 500.0f, 0.0f, 0.4f),  20.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compScLP,      1}, "SC LP Hz",       NRange(2000.0f, 20000.0f, 0.0f, 0.4f), 20000.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compStereoLink,1}, "Stereo Link",    NRange(0.0f, 1.0f),                  1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compMix,       1}, "Mix",            NRange(0.0f, 1.0f),                  1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::compLookahead, 1}, "Lookahead (ms)", NRange(0.0f, 10.0f),                 0.0f));
        layout.add(std::move(g));
    }

    // ---- Transient Shaper ----
    {
        auto g = std::make_unique<Grp>("transient", "Transient Shaper", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::transBypass,  1}, "Bypass",         true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transAttack,  1}, "Attack (dB)",    NRange(-24.0f, 24.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transSustain, 1}, "Sustain (dB)",   NRange(-24.0f, 24.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transSpeed,   1}, "Speed",          NRange(0.0f, 1.0f),    0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transLink,    1}, "Stereo Link",    NRange(0.0f, 1.0f),    1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::transWetDry,  1}, "Wet/Dry",        NRange(0.0f, 1.0f),    1.0f));
        layout.add(std::move(g));
    }

    // ---- Clipper ----
    {
        const juce::StringArray clipModes    = { "Hard", "Soft", "Sine", "Tape" };
        const juce::StringArray clipOsQuals  = { "2x", "4x", "8x" };
        auto g = std::make_unique<Grp>("clipper", "Clipper", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::clipBypass,    1}, "Bypass",           true));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::clipMode,      1}, "Mode",             clipModes, 1));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipCeiling,   1}, "Ceiling (dB)",     NRange(-24.0f, 0.0f),              0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipMix,       1}, "Mix",              NRange(0.0f, 1.0f),                1.0f));
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::clipAutoGain,  1}, "Auto Gain",        false));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipBias,      1}, "Bias",             NRange(-0.5f, 0.5f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::clipOsQuality, 1}, "Oversampling",     clipOsQuals, 1));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipEmphFreq,  1}, "Emphasis Freq Hz", NRange(200.0f, 10000.0f, 0.0f, 0.4f), 3000.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipEmphGain,  1}, "Emphasis (dB)",    NRange(0.0f, 12.0f),               0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::clipKneeWidth, 1}, "Knee Width",       NRange(0.0f, 1.0f),                0.33f));
        layout.add(std::move(g));
    }

    // ---- Limiter ----
    {
        auto g = std::make_unique<Grp>("limiter", "Limiter", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::limBypass,  1}, "Bypass",      true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::limCeiling, 1}, "Ceiling (dB)",NRange(-24.0f, 0.0f),             -1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::limRelease, 1}, "Release (ms)",NRange(1.0f, 500.0f, 0.0f, 0.5f), 50.0f));
        layout.add(std::move(g));
    }

    // ---- Gate ----
    {
        auto g = std::make_unique<Grp>("gate", "Gate", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::gateBypass,    1}, "Bypass",        true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gateThreshold, 1}, "Threshold (dB)",NRange(-96.0f, 0.0f),             -40.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gateAttack,    1}, "Attack (ms)",   NRange(0.1f, 500.0f, 0.0f, 0.5f),   1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gateRelease,   1}, "Release (ms)",  NRange(1.0f, 5000.0f, 0.0f, 0.5f), 100.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::gateRange,     1}, "Range (dB)",    NRange(-96.0f, 0.0f),               -60.0f));
        layout.add(std::move(g));
    }

    // ---- Modulation (LFO 1 + LFO 2) ----
    {
        auto g = std::make_unique<Grp>("modulation", "Modulation", "|");
        auto lfo1 = std::make_unique<Grp>("lfo1", "LFO 1", "|");
        lfo1->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::lfo1Rate,  1}, "Rate (Hz)", NRange(0.01f, 20.0f, 0.0f, 0.4f), 1.0f));
        lfo1->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::lfo1Shape, 1}, "Shape",     lfoShapes, 0));
        lfo1->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::lfo1Depth, 1}, "Depth",     NRange(0.0f, 1.0f), 0.5f));
        auto lfo2 = std::make_unique<Grp>("lfo2", "LFO 2", "|");
        lfo2->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::lfo2Rate,  1}, "Rate (Hz)", NRange(0.01f, 20.0f, 0.0f, 0.4f), 2.0f));
        lfo2->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::lfo2Shape, 1}, "Shape",     lfoShapes, 0));
        lfo2->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::lfo2Depth, 1}, "Depth",     NRange(0.0f, 1.0f), 0.5f));
        g->addChild(std::move(lfo1));
        g->addChild(std::move(lfo2));
        layout.add(std::move(g));
    }

    // ---- Chorus ----
    {
        auto g = std::make_unique<Grp>("chorus", "Chorus", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::chorusBypass,  1}, "Bypass",      true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::chorusRate,    1}, "Rate (Hz)",   NRange(0.1f, 10.0f, 0.0f, 0.5f), 0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::chorusDepth,   1}, "Depth (ms)",  NRange(0.0f, 20.0f),              3.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::chorusCentre,  1}, "Centre (ms)", NRange(1.0f, 30.0f),             10.0f));
        g->addChild(std::make_unique<juce::AudioParameterInt>  (PID{ParamID::chorusVoices,  1}, "Voices",      1, 4, 2));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::chorusWetDry,  1}, "Wet/Dry",     NRange(0.0f, 1.0f),               0.5f));
        layout.add(std::move(g));
    }

    // ---- Flanger ----
    {
        auto g = std::make_unique<Grp>("flanger", "Flanger", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::flangerBypass,   1}, "Bypass",      true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerRate,     1}, "Rate (Hz)",   NRange(0.01f, 10.0f, 0.0f, 0.5f), 0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerDepth,    1}, "Depth (ms)",  NRange(0.0f, 4.5f),                2.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerCentre,   1}, "Centre (ms)", NRange(0.5f, 5.0f),                2.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerFeedback, 1}, "Feedback",    NRange(-0.95f, 0.95f),             0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::flangerWetDry,   1}, "Wet/Dry",     NRange(0.0f, 1.0f),                0.7f));
        layout.add(std::move(g));
    }

    // ---- Phaser ----
    {
        auto g = std::make_unique<Grp>("phaser", "Phaser", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool>  (PID{ParamID::phaserBypass,   1}, "Bypass",       true));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserRate,     1}, "Rate (Hz)",    NRange(0.01f, 10.0f, 0.0f, 0.5f),  0.5f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserDepth,    1}, "Depth",        NRange(0.0f, 1.0f),                 1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserBaseFreq, 1}, "Base Freq Hz", NRange(50.0f, 8000.0f, 0.0f, 0.3f), 500.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserFeedback, 1}, "Feedback",     NRange(-0.95f, 0.95f),              0.7f));
        g->addChild(std::make_unique<juce::AudioParameterChoice>(PID{ParamID::phaserStages,   1}, "Stages",       phaserStages, 1));
        g->addChild(std::make_unique<juce::AudioParameterFloat> (PID{ParamID::phaserWetDry,   1}, "Wet/Dry",      NRange(0.0f, 1.0f),                 0.7f));
        layout.add(std::move(g));
    }

    // ---- Spatial ----
    {
        auto g = std::make_unique<Grp>("spatial", "Spatial", "|");
        g->addChild(std::make_unique<juce::AudioParameterBool> (PID{ParamID::spatialBypass,   1}, "Bypass",            true));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialWidth,    1}, "Width",             NRange(0.0f, 2.0f), 1.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialPan,      1}, "Pan",               NRange(-1.0f, 1.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialRotation, 1}, "Rotation (deg)",    NRange(-180.0f, 180.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialMidGain,  1}, "Mid Gain (dB)",     NRange(-24.0f, 24.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialSideGain, 1}, "Side Gain (dB)",    NRange(-24.0f, 24.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialBassMono, 1}, "Bass Mono (Hz)",    NRange(0.0f, 500.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialHaasMs,   1}, "Haas Delay (ms)",   NRange(0.0f, 30.0f), 0.0f));
        g->addChild(std::make_unique<juce::AudioParameterFloat>(PID{ParamID::spatialWetDry,   1}, "Wet/Dry",           NRange(0.0f, 1.0f), 1.0f));
        layout.add(std::move(g));
    }

    // ---- Chain Order (each value = position 0..14; modules sorted by value) ----
    {
        auto g = std::make_unique<Grp>("chain_order", "Chain Order", "|");
        auto pos = [](const char* id, const char* name, int def)
        {
            return std::make_unique<juce::AudioParameterInt>(
                juce::ParameterID{id,1}, name, 0, 14, def);
        };
        g->addChild(pos(ParamID::posFilter,     "Filter Order",          0));
        g->addChild(pos(ParamID::posEq,         "EQ Order",              1));
        g->addChild(pos(ParamID::posGain,       "Gain Order",            2));
        g->addChild(pos(ParamID::posDelay,      "Delay Order",           3));
        g->addChild(pos(ParamID::posReverb,     "Reverb Order",          4));
        g->addChild(pos(ParamID::posWaveshaper, "Waveshaper Order",      5));
        g->addChild(pos(ParamID::posCompressor, "Compressor Order",      6));
        g->addChild(pos(ParamID::posClipper,    "Clipper Order",         7));
        g->addChild(pos(ParamID::posTransient,  "Transient Shaper Order",8));
        g->addChild(pos(ParamID::posLimiter,    "Limiter Order",         9));
        g->addChild(pos(ParamID::posGate,       "Gate Order",            10));
        g->addChild(pos(ParamID::posChorus,     "Chorus Order",          11));
        g->addChild(pos(ParamID::posFlanger,    "Flanger Order",         12));
        g->addChild(pos(ParamID::posPhaser,     "Phaser Order",          13));
        g->addChild(pos(ParamID::posSpatial,    "Spatial Order",         14));
        layout.add(std::move(g));
    }

    return layout;
}

// ============================================================
// Constructor / Destructor
// ============================================================

AlteredAudioProcessor::AlteredAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "AlteredAudioParams", createParameterLayout())
{
    cacheParameterPointers();

    // Add every module to the chain and save raw pointers for parameter updates.
    // All modules start bypassed (bypass params default to true).
    filterMod     = static_cast<FilterModule*>    (chain.addModule(std::make_unique<FilterModule>()));
    eqMod         = static_cast<EQModule*>        (chain.addModule(std::make_unique<EQModule>()));
    gainMod       = static_cast<GainModule*>      (chain.addModule(std::make_unique<GainModule>()));
    delayMod      = static_cast<DelayModule*>     (chain.addModule(std::make_unique<DelayModule>()));
    reverbMod     = static_cast<ReverbModule*>    (chain.addModule(std::make_unique<ReverbModule>()));
    waveshaperMod = static_cast<WaveshaperModule*>(chain.addModule(std::make_unique<WaveshaperModule>()));
    compressorMod = static_cast<CompressorModule*>(chain.addModule(std::make_unique<CompressorModule>()));
    clipperMod    = static_cast<ClipperModule*>          (chain.addModule(std::make_unique<ClipperModule>()));
    transientMod  = static_cast<TransientShaperModule*>  (chain.addModule(std::make_unique<TransientShaperModule>()));
    limiterMod    = static_cast<LimiterModule*>          (chain.addModule(std::make_unique<LimiterModule>()));
    gateMod       = static_cast<GateModule*>      (chain.addModule(std::make_unique<GateModule>()));
    chorusMod     = static_cast<ChorusModule*>    (chain.addModule(std::make_unique<ChorusModule>()));
    flangerMod    = static_cast<FlangerModule*>   (chain.addModule(std::make_unique<FlangerModule>()));
    phaserMod     = static_cast<PhaserModule*>    (chain.addModule(std::make_unique<PhaserModule>()));
    spatialMod    = static_cast<SpatialModule*>   (chain.addModule(std::make_unique<SpatialModule>()));
}

AlteredAudioProcessor::~AlteredAudioProcessor() {}

// ============================================================
// Cache parameter pointers (called once from constructor)
// ============================================================

void AlteredAudioProcessor::cacheParameterPointers()
{
    auto* vts = &apvts;

    p_channelMode   = vts->getRawParameterValue(ParamID::channelMode);

    p_filterBypass  = vts->getRawParameterValue(ParamID::filterBypass);
    p_filterType    = vts->getRawParameterValue(ParamID::filterType);
    p_filterFreq    = vts->getRawParameterValue(ParamID::filterFreq);
    p_filterQ       = vts->getRawParameterValue(ParamID::filterQ);
    p_filterGain    = vts->getRawParameterValue(ParamID::filterGain);
    p_filterSlope   = vts->getRawParameterValue(ParamID::filterSlope);
    p_filterDrive   = vts->getRawParameterValue(ParamID::filterDrive);
    p_filterMix     = vts->getRawParameterValue(ParamID::filterMix);
    p_filterOutput  = vts->getRawParameterValue(ParamID::filterOutput);

    p_eqBypass      = vts->getRawParameterValue(ParamID::eqBypass);
    for (int n = 1; n <= EQModule::kMaxBands; ++n)
    {
        p_eqEnabled[n-1] = vts->getRawParameterValue(ParamID::eqEnabled(n));
        p_eqType   [n-1] = vts->getRawParameterValue(ParamID::eqType(n));
        p_eqFreq   [n-1] = vts->getRawParameterValue(ParamID::eqFreq(n));
        p_eqQ      [n-1] = vts->getRawParameterValue(ParamID::eqQ(n));
        p_eqGain   [n-1] = vts->getRawParameterValue(ParamID::eqGain(n));
        p_eqSlope  [n-1] = vts->getRawParameterValue(ParamID::eqSlope(n));
    }

    p_delayBypass    = vts->getRawParameterValue(ParamID::delayBypass);
    p_delayTime      = vts->getRawParameterValue(ParamID::delayTime);
    p_delayTimeR     = vts->getRawParameterValue(ParamID::delayTimeR);
    p_delaySync      = vts->getRawParameterValue(ParamID::delaySync);
    p_delayDivL      = vts->getRawParameterValue(ParamID::delayDivL);
    p_delayDivR      = vts->getRawParameterValue(ParamID::delayDivR);
    p_delayFeedback  = vts->getRawParameterValue(ParamID::delayFeedback);
    p_delayFbLPHz    = vts->getRawParameterValue(ParamID::delayFbLPHz);
    p_delayFbHPHz    = vts->getRawParameterValue(ParamID::delayFbHPHz);
    p_delayDucking   = vts->getRawParameterValue(ParamID::delayDucking);
    p_delayDiffusion = vts->getRawParameterValue(ParamID::delayDiffusion);
    p_delayWow       = vts->getRawParameterValue(ParamID::delayWow);
    p_delayModRate   = vts->getRawParameterValue(ParamID::delayModRate);
    p_delayModDepth  = vts->getRawParameterValue(ParamID::delayModDepth);
    p_delayWetDry    = vts->getRawParameterValue(ParamID::delayWetDry);
    p_delayPingPong  = vts->getRawParameterValue(ParamID::delayPingPong);

    p_reverbBypass    = vts->getRawParameterValue(ParamID::reverbBypass);
    p_reverbRoom      = vts->getRawParameterValue(ParamID::reverbRoom);
    p_reverbDamp      = vts->getRawParameterValue(ParamID::reverbDamp);
    p_reverbDiffusion = vts->getRawParameterValue(ParamID::reverbDiffusion);
    p_reverbPreDelay  = vts->getRawParameterValue(ParamID::reverbPreDelay);
    p_reverbModRate   = vts->getRawParameterValue(ParamID::reverbModRate);
    p_reverbErLevel   = vts->getRawParameterValue(ParamID::reverbErLevel);
    p_reverbLowCut    = vts->getRawParameterValue(ParamID::reverbLowCut);
    p_reverbShimmer   = vts->getRawParameterValue(ParamID::reverbShimmer);
    p_reverbPitchSemi = vts->getRawParameterValue(ParamID::reverbPitchSemi);
    p_reverbWetDry    = vts->getRawParameterValue(ParamID::reverbWetDry);

    p_wsBypass   = vts->getRawParameterValue(ParamID::wsBypass);
    p_wsAlgo     = vts->getRawParameterValue(ParamID::wsAlgo);
    p_wsDrive    = vts->getRawParameterValue(ParamID::wsDrive);
    p_wsWetDry   = vts->getRawParameterValue(ParamID::wsWetDry);
    p_wsBitDepth = vts->getRawParameterValue(ParamID::wsBitDepth);

    p_compBypass      = vts->getRawParameterValue(ParamID::compBypass);
    p_compThreshold   = vts->getRawParameterValue(ParamID::compThreshold);
    p_compRatio       = vts->getRawParameterValue(ParamID::compRatio);
    p_compAttack      = vts->getRawParameterValue(ParamID::compAttack);
    p_compRelease     = vts->getRawParameterValue(ParamID::compRelease);
    p_compKnee        = vts->getRawParameterValue(ParamID::compKnee);
    p_compMakeup      = vts->getRawParameterValue(ParamID::compMakeup);
    p_compRmsPeak     = vts->getRawParameterValue(ParamID::compRmsPeak);
    p_compAutoRel     = vts->getRawParameterValue(ParamID::compAutoRel);
    p_compSaturation  = vts->getRawParameterValue(ParamID::compSaturation);
    p_compScHP        = vts->getRawParameterValue(ParamID::compScHP);
    p_compScLP        = vts->getRawParameterValue(ParamID::compScLP);
    p_compStereoLink  = vts->getRawParameterValue(ParamID::compStereoLink);
    p_compMix         = vts->getRawParameterValue(ParamID::compMix);
    p_compLookahead   = vts->getRawParameterValue(ParamID::compLookahead);

    p_transBypass  = vts->getRawParameterValue(ParamID::transBypass);
    p_transAttack  = vts->getRawParameterValue(ParamID::transAttack);
    p_transSustain = vts->getRawParameterValue(ParamID::transSustain);
    p_transSpeed   = vts->getRawParameterValue(ParamID::transSpeed);
    p_transLink    = vts->getRawParameterValue(ParamID::transLink);
    p_transWetDry  = vts->getRawParameterValue(ParamID::transWetDry);

    p_clipBypass     = vts->getRawParameterValue(ParamID::clipBypass);
    p_clipMode       = vts->getRawParameterValue(ParamID::clipMode);
    p_clipCeiling    = vts->getRawParameterValue(ParamID::clipCeiling);
    p_clipMix        = vts->getRawParameterValue(ParamID::clipMix);
    p_clipAutoGain   = vts->getRawParameterValue(ParamID::clipAutoGain);
    p_clipBias       = vts->getRawParameterValue(ParamID::clipBias);
    p_clipOsQuality  = vts->getRawParameterValue(ParamID::clipOsQuality);
    p_clipEmphFreq   = vts->getRawParameterValue(ParamID::clipEmphFreq);
    p_clipEmphGain   = vts->getRawParameterValue(ParamID::clipEmphGain);
    p_clipKneeWidth  = vts->getRawParameterValue(ParamID::clipKneeWidth);

    p_limBypass  = vts->getRawParameterValue(ParamID::limBypass);
    p_limCeiling = vts->getRawParameterValue(ParamID::limCeiling);
    p_limRelease = vts->getRawParameterValue(ParamID::limRelease);

    p_gateBypass    = vts->getRawParameterValue(ParamID::gateBypass);
    p_gateThreshold = vts->getRawParameterValue(ParamID::gateThreshold);
    p_gateAttack    = vts->getRawParameterValue(ParamID::gateAttack);
    p_gateRelease   = vts->getRawParameterValue(ParamID::gateRelease);
    p_gateRange     = vts->getRawParameterValue(ParamID::gateRange);

    p_lfo1Rate  = vts->getRawParameterValue(ParamID::lfo1Rate);
    p_lfo1Shape = vts->getRawParameterValue(ParamID::lfo1Shape);
    p_lfo1Depth = vts->getRawParameterValue(ParamID::lfo1Depth);
    p_lfo2Rate  = vts->getRawParameterValue(ParamID::lfo2Rate);
    p_lfo2Shape = vts->getRawParameterValue(ParamID::lfo2Shape);
    p_lfo2Depth = vts->getRawParameterValue(ParamID::lfo2Depth);

    p_chorusBypass  = vts->getRawParameterValue(ParamID::chorusBypass);
    p_chorusRate    = vts->getRawParameterValue(ParamID::chorusRate);
    p_chorusDepth   = vts->getRawParameterValue(ParamID::chorusDepth);
    p_chorusCentre  = vts->getRawParameterValue(ParamID::chorusCentre);
    p_chorusVoices  = vts->getRawParameterValue(ParamID::chorusVoices);
    p_chorusWetDry  = vts->getRawParameterValue(ParamID::chorusWetDry);

    p_flangerBypass   = vts->getRawParameterValue(ParamID::flangerBypass);
    p_flangerRate     = vts->getRawParameterValue(ParamID::flangerRate);
    p_flangerDepth    = vts->getRawParameterValue(ParamID::flangerDepth);
    p_flangerCentre   = vts->getRawParameterValue(ParamID::flangerCentre);
    p_flangerFeedback = vts->getRawParameterValue(ParamID::flangerFeedback);
    p_flangerWetDry   = vts->getRawParameterValue(ParamID::flangerWetDry);

    p_phaserBypass   = vts->getRawParameterValue(ParamID::phaserBypass);
    p_phaserRate     = vts->getRawParameterValue(ParamID::phaserRate);
    p_phaserDepth    = vts->getRawParameterValue(ParamID::phaserDepth);
    p_phaserBaseFreq = vts->getRawParameterValue(ParamID::phaserBaseFreq);
    p_phaserFeedback = vts->getRawParameterValue(ParamID::phaserFeedback);
    p_phaserStages   = vts->getRawParameterValue(ParamID::phaserStages);
    p_phaserWetDry   = vts->getRawParameterValue(ParamID::phaserWetDry);

    p_gainBypass = vts->getRawParameterValue(ParamID::gainBypass);
    p_gainDb     = vts->getRawParameterValue(ParamID::gainDb);

    p_posFilter     = vts->getRawParameterValue(ParamID::posFilter);
    p_posEq         = vts->getRawParameterValue(ParamID::posEq);
    p_posGain       = vts->getRawParameterValue(ParamID::posGain);
    p_posDelay      = vts->getRawParameterValue(ParamID::posDelay);
    p_posReverb     = vts->getRawParameterValue(ParamID::posReverb);
    p_posWaveshaper = vts->getRawParameterValue(ParamID::posWaveshaper);
    p_posCompressor = vts->getRawParameterValue(ParamID::posCompressor);
    p_posLimiter    = vts->getRawParameterValue(ParamID::posLimiter);
    p_posGate       = vts->getRawParameterValue(ParamID::posGate);
    p_posChorus     = vts->getRawParameterValue(ParamID::posChorus);
    p_posFlanger    = vts->getRawParameterValue(ParamID::posFlanger);
    p_posPhaser     = vts->getRawParameterValue(ParamID::posPhaser);
    p_posClipper    = vts->getRawParameterValue(ParamID::posClipper);
    p_posTransient  = vts->getRawParameterValue(ParamID::posTransient);
    p_posSpatial    = vts->getRawParameterValue(ParamID::posSpatial);

    p_spatialBypass   = vts->getRawParameterValue(ParamID::spatialBypass);
    p_spatialWidth    = vts->getRawParameterValue(ParamID::spatialWidth);
    p_spatialPan      = vts->getRawParameterValue(ParamID::spatialPan);
    p_spatialRotation = vts->getRawParameterValue(ParamID::spatialRotation);
    p_spatialMidGain  = vts->getRawParameterValue(ParamID::spatialMidGain);
    p_spatialSideGain = vts->getRawParameterValue(ParamID::spatialSideGain);
    p_spatialBassMono = vts->getRawParameterValue(ParamID::spatialBassMono);
    p_spatialHaasMs   = vts->getRawParameterValue(ParamID::spatialHaasMs);
    p_spatialWetDry   = vts->getRawParameterValue(ParamID::spatialWetDry);
}

// ============================================================
// Update module parameters (called per block, audio thread)
// ============================================================

void AlteredAudioProcessor::updateModuleParameters()
{
    // ---- LFOs (advance by full block, get value at end of block) ----
    lfo1.setRate (*p_lfo1Rate);
    lfo1.setShape(static_cast<LFOEngine::Shape>((int)*p_lfo1Shape));
    const float lfo1Val = lfo1.tickBlock(currentBlockSize) * (*p_lfo1Depth);

    lfo2.setRate (*p_lfo2Rate);
    lfo2.setShape(static_cast<LFOEngine::Shape>((int)*p_lfo2Shape));
    const float lfo2Val = lfo2.tickBlock(currentBlockSize) * (*p_lfo2Depth);

    // Feed modulation matrix with current source values
    modMatrix.update(lfo1Val, lfo2Val, envFollower.getLevel());

    // ---- Channel mode ----
    channelMode = static_cast<ChannelMode>((int)*p_channelMode);

    // ---- Filter ----
    if (filterMod != nullptr)
    {
        const bool bp = *p_filterBypass > 0.5f;
        filterMod->setBypassed(bp);
        if (!bp)
        {
            filterMod->setType(static_cast<BiquadFilter::Type>((int)*p_filterType));
            const float freqBase = *p_filterFreq;
            const float freqMod  = modMatrix.getModulation(ParamID::filterFreq);
            filterMod->setFrequency(juce::jlimit(20.0f, 20000.0f,
                freqMod != 0.0f ? freqBase * std::pow(2.0f, freqMod * 3.0f) : freqBase));
            filterMod->setQ(*p_filterQ);
            filterMod->setGainDb(*p_filterGain);
            filterMod->setSlope((int)*p_filterSlope);
            filterMod->setDriveDb(*p_filterDrive);
            filterMod->setMix(*p_filterMix);
            filterMod->setOutputDb(*p_filterOutput);
        }
    }

    // ---- EQ ----
    if (eqMod != nullptr)
    {
        const bool bp = *p_eqBypass > 0.5f;
        eqMod->setBypassed(bp);
        if (!bp)
        {
            for (int n = 0; n < EQModule::kMaxBands; ++n)
            {
                EQModule::BandParams band;
                band.enabled   = *p_eqEnabled[n] > 0.5f;
                band.type      = static_cast<BiquadFilter::Type>((int)*p_eqType[n]);
                band.frequency = juce::jlimit(20.0f, 20000.0f, (float)*p_eqFreq[n]);
                band.q         = juce::jlimit(0.1f,  10.0f,    (float)*p_eqQ[n]);
                band.gainDb    = *p_eqGain[n];
                band.slope     = p_eqSlope[n] ? (int)*p_eqSlope[n] : 1;
                eqMod->setBand(n, band);
            }
        }
    }

    // ---- Delay ----
    if (delayMod != nullptr)
    {
        const bool bp = *p_delayBypass > 0.5f;
        delayMod->setBypassed(bp);
        if (!bp)
        {
            const float timeMod = modMatrix.getModulation(ParamID::delayTime) * 1000.0f;
            float msL, msR;
            if (*p_delaySync > 0.5f)
            {
                msL = DelayModule::syncDivisionToMs((int)*p_delayDivL, currentBpm);
                msR = DelayModule::syncDivisionToMs((int)*p_delayDivR, currentBpm);
            }
            else
            {
                msL = *p_delayTime;
                msR = *p_delayTimeR;
            }
            delayMod->setDelayTimesMs(msL + timeMod, msR + timeMod);
            delayMod->setFeedback  (*p_delayFeedback);
            delayMod->setFbLPHz    (*p_delayFbLPHz);
            delayMod->setFbHPHz    (*p_delayFbHPHz);
            delayMod->setDucking   (*p_delayDucking);
            delayMod->setDiffusion (*p_delayDiffusion);
            delayMod->setWowFlutter(*p_delayWow);
            delayMod->setModRate   (*p_delayModRate);
            delayMod->setModDepth  (*p_delayModDepth);
            delayMod->setWetDry    (*p_delayWetDry);
            delayMod->setPingPong  (*p_delayPingPong > 0.5f);
        }
    }

    // ---- Reverb ----
    if (reverbMod != nullptr)
    {
        const bool bp = *p_reverbBypass > 0.5f;
        reverbMod->setBypassed(bp);
        if (!bp)
        {
            const float decayMod = modMatrix.getModulation(ParamID::reverbRoom);
            reverbMod->setDecay      (juce::jlimit(0.0f, 1.0f, (float)*p_reverbRoom + decayMod * 0.5f));
            reverbMod->setDamping    (*p_reverbDamp);
            reverbMod->setDiffusion  (*p_reverbDiffusion);
            reverbMod->setPreDelayMs (*p_reverbPreDelay);
            reverbMod->setModRate    (*p_reverbModRate);
            reverbMod->setErLevel    (*p_reverbErLevel);
            reverbMod->setLowCutHz   (*p_reverbLowCut);
            reverbMod->setShimmer    (*p_reverbShimmer);
            reverbMod->setPitchSemi  (*p_reverbPitchSemi);
            reverbMod->setWetDry     (*p_reverbWetDry);
        }
    }

    // ---- Waveshaper ----
    if (waveshaperMod != nullptr)
    {
        const bool bp = *p_wsBypass > 0.5f;
        waveshaperMod->setBypassed(bp);
        if (!bp)
        {
            waveshaperMod->setAlgorithm(static_cast<WaveshaperModule::Algorithm>((int)*p_wsAlgo));
            const float driveMod = modMatrix.getModulation(ParamID::wsDrive);
            waveshaperMod->setDrive(juce::jlimit(1.0f, 100.0f,
                (float)*p_wsDrive + driveMod * 49.5f));
            waveshaperMod->setWetDry  (*p_wsWetDry);
            waveshaperMod->setBitDepth((int)*p_wsBitDepth);
        }
    }

    // ---- Compressor ----
    if (compressorMod != nullptr)
    {
        const bool bp = *p_compBypass > 0.5f;
        compressorMod->setBypassed(bp);
        if (!bp)
        {
            compressorMod->setThresholdDb  (*p_compThreshold);
            compressorMod->setRatio        (*p_compRatio);
            compressorMod->setAttackMs     (*p_compAttack);
            compressorMod->setReleaseMs    (*p_compRelease);
            compressorMod->setKneeDb       (*p_compKnee);
            compressorMod->setMakeupGainDb (*p_compMakeup);
            compressorMod->setRmsPeak      (*p_compRmsPeak);
            compressorMod->setAutoRelease  (*p_compAutoRel);
            compressorMod->setSaturation   (*p_compSaturation);
            compressorMod->setScHPHz       (*p_compScHP);
            compressorMod->setScLPHz       (*p_compScLP);
            compressorMod->setStereoLink   (*p_compStereoLink);
            compressorMod->setMix          (*p_compMix);
            compressorMod->setLookaheadMs  (*p_compLookahead);
        }
    }

    // ---- Transient Shaper ----
    if (transientMod != nullptr)
    {
        const bool bp = *p_transBypass > 0.5f;
        transientMod->setBypassed(bp);
        if (!bp)
        {
            transientMod->setAttackGainDb (*p_transAttack);
            transientMod->setSustainGainDb(*p_transSustain);
            transientMod->setSpeed        (*p_transSpeed);
            transientMod->setStereoLink   (*p_transLink);
            transientMod->setWetDry       (*p_transWetDry);
        }
    }

    // ---- Clipper ----
    if (clipperMod != nullptr)
    {
        const bool bp = *p_clipBypass > 0.5f;
        clipperMod->setBypassed(bp);
        if (!bp)
        {
            clipperMod->setMode      (static_cast<ClipperModule::Mode>((int)*p_clipMode));
            clipperMod->setCeilingDb (*p_clipCeiling);
            clipperMod->setMix       (*p_clipMix);
            clipperMod->setAutoGain  (*p_clipAutoGain > 0.5f);
            clipperMod->setBias      (*p_clipBias);
            clipperMod->setOsQuality ((int)*p_clipOsQuality);
            clipperMod->setEmphFreqHz(*p_clipEmphFreq);
            clipperMod->setEmphGainDb(*p_clipEmphGain);
            clipperMod->setKneeWidth (*p_clipKneeWidth);
        }
    }

    // ---- Limiter ----
    if (limiterMod != nullptr)
    {
        const bool bp = *p_limBypass > 0.5f;
        limiterMod->setBypassed(bp);
        if (!bp)
        {
            limiterMod->setCeiling  (*p_limCeiling);
            limiterMod->setReleaseMs(*p_limRelease); // exp() only on change
        }
    }

    // ---- Gate ----
    if (gateMod != nullptr)
    {
        const bool bp = *p_gateBypass > 0.5f;
        gateMod->setBypassed(bp);
        if (!bp)
        {
            gateMod->setThresholdDb(*p_gateThreshold);
            gateMod->setAttackMs   (*p_gateAttack);  // exp() only on change
            gateMod->setReleaseMs  (*p_gateRelease); // exp() only on change
            gateMod->setRangeDb    (*p_gateRange);
        }
    }

    // ---- Chorus ----
    if (chorusMod != nullptr)
    {
        const bool bp = *p_chorusBypass > 0.5f;
        chorusMod->setBypassed(bp);
        if (!bp)
        {
            chorusMod->setRate     (*p_chorusRate);
            chorusMod->setDepthMs  (*p_chorusDepth);
            chorusMod->setCentreMs (*p_chorusCentre);
            chorusMod->setNumVoices((int)*p_chorusVoices);
            chorusMod->setWetDry   (*p_chorusWetDry);
        }
    }

    // ---- Flanger ----
    if (flangerMod != nullptr)
    {
        const bool bp = *p_flangerBypass > 0.5f;
        flangerMod->setBypassed(bp);
        if (!bp)
        {
            flangerMod->setRate    (*p_flangerRate);
            flangerMod->setDepthMs (*p_flangerDepth);
            flangerMod->setCentreMs(*p_flangerCentre);
            flangerMod->setFeedback(*p_flangerFeedback);
            flangerMod->setWetDry  (*p_flangerWetDry);
        }
    }

    // ---- Phaser ----
    if (phaserMod != nullptr)
    {
        const bool bp = *p_phaserBypass > 0.5f;
        phaserMod->setBypassed(bp);
        if (!bp)
        {
            phaserMod->setRate     (*p_phaserRate);
            phaserMod->setDepth    (*p_phaserDepth);
            phaserMod->setBaseFreq (*p_phaserBaseFreq);
            phaserMod->setFeedback (*p_phaserFeedback);
            phaserMod->setNumStages(((int)*p_phaserStages + 1) * 2);
            phaserMod->setWetDry   (*p_phaserWetDry);
        }
    }

    // ---- Gain ----
    if (gainMod != nullptr)
    {
        const bool bp = *p_gainBypass > 0.5f;
        gainMod->setBypassed(bp);
        if (!bp)
            gainMod->setGainDb(*p_gainDb);
    }

    // ---- Spatial ----
    if (spatialMod != nullptr)
    {
        const bool bp = *p_spatialBypass > 0.5f;
        spatialMod->setBypassed(bp);
        if (!bp)
        {
            spatialMod->setWidth      (*p_spatialWidth);
            spatialMod->setPan        (*p_spatialPan);
            spatialMod->setRotationDeg(*p_spatialRotation);
            spatialMod->setMidGainDb  (*p_spatialMidGain);
            spatialMod->setSideGainDb (*p_spatialSideGain);
            spatialMod->setBassMono   (*p_spatialBassMono);
            spatialMod->setHaasMs     (*p_spatialHaasMs);
            spatialMod->setWetDry     (*p_spatialWetDry);
        }
    }

    // ---- Module order ----
    {
        std::array<std::pair<int,int>, 15> pairs {{
            {0,  (int)*p_posFilter},
            {1,  (int)*p_posEq},
            {2,  (int)*p_posGain},
            {3,  (int)*p_posDelay},
            {4,  (int)*p_posReverb},
            {5,  (int)*p_posWaveshaper},
            {6,  (int)*p_posCompressor},
            {7,  (int)*p_posClipper},
            {8,  (int)*p_posTransient},
            {9,  (int)*p_posLimiter},
            {10, (int)*p_posGate},
            {11, (int)*p_posChorus},
            {12, (int)*p_posFlanger},
            {13, (int)*p_posPhaser},
            {14, (int)*p_posSpatial},
        }};
        std::stable_sort(pairs.begin(), pairs.end(),
                         [](const auto& a, const auto& b){ return a.second < b.second; });
        std::array<int, 15> order;
        for (int i = 0; i < 15; ++i)
            order[i] = pairs[i].first;
        chain.setProcessingOrder(order.data(), 15);
    }
}

// ============================================================
// Audio lifecycle
// ============================================================

const juce::String AlteredAudioProcessor::getName() const { return JucePlugin_Name; }
bool AlteredAudioProcessor::acceptsMidi() const  { return false; }
bool AlteredAudioProcessor::producesMidi() const { return false; }
bool AlteredAudioProcessor::isMidiEffect() const { return false; }
double AlteredAudioProcessor::getTailLengthSeconds() const { return 2.0; } // reverb tail

int AlteredAudioProcessor::getNumPrograms()                             { return 1; }
int AlteredAudioProcessor::getCurrentProgram()                          { return 0; }
void AlteredAudioProcessor::setCurrentProgram(int)                      {}
const juce::String AlteredAudioProcessor::getProgramName(int)           { return {}; }
void AlteredAudioProcessor::changeProgramName(int, const juce::String&) {}

void AlteredAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;
    chain.prepare(sampleRate, samplesPerBlock);
    lfo1.prepare(sampleRate);
    lfo2.prepare(sampleRate);
    envFollower.prepare(sampleRate, samplesPerBlock);

    // Report chain latency (oversampling + lookahead) so the host can compensate
    setLatencySamples(chain.getLatencySamples());
}

void AlteredAudioProcessor::releaseResources()
{
    chain.reset();
    lfo1.reset();
    lfo2.reset();
    envFollower.reset();
}

void AlteredAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    // Envelope follower processes input before any DSP
    envFollower.processBlock(buffer);

    // Host tempo for synced delay times (falls back to last known / 120)
    if (auto* playHead = getPlayHead())
        if (auto pos = playHead->getPosition())
            if (auto bpm = pos->getBpm())
                currentBpm = *bpm;

    // Read all APVTS params, run LFOs, apply modulation → update every module's setters
    updateModuleParameters();

    // Keep host PDC current — bypass toggles and lookahead changes move the total
    const int chainLatency = chain.getLatencySamples();
    if (chainLatency != getLatencySamples())
        setLatencySamples(chainLatency);

    // Channel encoding
    if (channelMode == ChannelMode::Mono && buffer.getNumChannels() >= 2)
    {
        auto* L = buffer.getWritePointer(0);
        const auto* R = buffer.getReadPointer(1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            L[i] = (L[i] + R[i]) * 0.5f;
    }

    if (channelMode == ChannelMode::MidSide)
        encodeMidSide(buffer);

    chain.process(buffer);

    if (channelMode == ChannelMode::MidSide)
        decodeMidSide(buffer);

    if (channelMode == ChannelMode::Mono && buffer.getNumChannels() >= 2)
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
}

// ============================================================
// Mid/Side encode/decode
// ============================================================

void AlteredAudioProcessor::encodeMidSide(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 2) return;
    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getWritePointer(1);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float m = (L[i] + R[i]) * 0.5f;
        const float s = (L[i] - R[i]) * 0.5f;
        L[i] = m; R[i] = s;
    }
}

void AlteredAudioProcessor::decodeMidSide(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 2) return;
    auto* M = buffer.getWritePointer(0);
    auto* S = buffer.getWritePointer(1);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float L = M[i] + S[i];
        const float R = M[i] - S[i];
        M[i] = L; S[i] = R;
    }
}

// ============================================================
// Editor
// ============================================================

bool AlteredAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AlteredAudioProcessor::createEditor()
{
    return new AlteredAudioEditor(*this);
}

// ============================================================
// State serialisation (APVTS handles everything)
// ============================================================

void AlteredAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AlteredAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// ============================================================
// Plugin entry point
// ============================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AlteredAudioProcessor();
}
