#pragma once
#include <JuceHeader.h>

namespace ParamID
{
    // Global
    constexpr const char* channelMode     = "channel_mode";

    // Filter
    constexpr const char* filterBypass    = "filter_bypass";
    constexpr const char* filterType      = "filter_type";
    constexpr const char* filterFreq      = "filter_freq";
    constexpr const char* filterQ         = "filter_q";
    constexpr const char* filterGain      = "filter_gain";

    // EQ (band index 1..8, returned as juce::String)
    constexpr const char* eqBypass        = "eq_bypass";
    inline juce::String eqEnabled(int n)  { return "eq_" + juce::String(n) + "_en";   }
    inline juce::String eqType   (int n)  { return "eq_" + juce::String(n) + "_type"; }
    inline juce::String eqFreq   (int n)  { return "eq_" + juce::String(n) + "_freq"; }
    inline juce::String eqQ      (int n)  { return "eq_" + juce::String(n) + "_q";    }
    inline juce::String eqGain   (int n)  { return "eq_" + juce::String(n) + "_gain"; }

    // Delay
    constexpr const char* delayBypass     = "delay_bypass";
    constexpr const char* delayTime       = "delay_time";
    constexpr const char* delayFeedback   = "delay_feedback";
    constexpr const char* delaySpread     = "delay_spread";
    constexpr const char* delayFbLPHz     = "delay_fb_lp";
    constexpr const char* delayFbHPHz     = "delay_fb_hp";
    constexpr const char* delayDucking    = "delay_ducking";
    constexpr const char* delayDiffusion  = "delay_diffusion";
    constexpr const char* delayWow        = "delay_wow";
    constexpr const char* delayModRate    = "delay_mod_rate";
    constexpr const char* delayModDepth   = "delay_mod_depth";
    constexpr const char* delayWetDry     = "delay_wetdry";
    constexpr const char* delayPingPong   = "delay_pingpong";

    // Reverb
    constexpr const char* reverbBypass    = "reverb_bypass";
    constexpr const char* reverbRoom      = "reverb_room";
    constexpr const char* reverbDamp      = "reverb_damp";
    constexpr const char* reverbDiffusion = "reverb_diffusion";
    constexpr const char* reverbPreDelay  = "reverb_predelay";
    constexpr const char* reverbModRate   = "reverb_modrate";
    constexpr const char* reverbErLevel   = "reverb_erlevel";
    constexpr const char* reverbLowCut    = "reverb_lowcut";
    constexpr const char* reverbShimmer   = "reverb_shimmer";
    constexpr const char* reverbPitchSemi = "reverb_pitch";
    constexpr const char* reverbWetDry    = "reverb_wetdry";

    // Waveshaper
    constexpr const char* wsBypass        = "ws_bypass";
    constexpr const char* wsAlgo          = "ws_algo";
    constexpr const char* wsDrive         = "ws_drive";
    constexpr const char* wsWetDry        = "ws_wetdry";
    constexpr const char* wsBitDepth      = "ws_bitdepth";

    // Compressor
    constexpr const char* compBypass      = "comp_bypass";
    constexpr const char* compThreshold   = "comp_threshold";
    constexpr const char* compRatio       = "comp_ratio";
    constexpr const char* compAttack      = "comp_attack";
    constexpr const char* compRelease     = "comp_release";
    constexpr const char* compKnee        = "comp_knee";
    constexpr const char* compMakeup      = "comp_makeup";
    constexpr const char* compRmsPeak     = "comp_rms_peak";
    constexpr const char* compAutoRel     = "comp_auto_rel";
    constexpr const char* compSaturation  = "comp_saturation";
    constexpr const char* compScHP        = "comp_sc_hp";
    constexpr const char* compScLP        = "comp_sc_lp";
    constexpr const char* compStereoLink  = "comp_stereo_link";
    constexpr const char* compMix         = "comp_mix";
    constexpr const char* compLookahead   = "comp_lookahead";

    // Transient Shaper
    constexpr const char* transBypass     = "trans_bypass";
    constexpr const char* transAttack     = "trans_attack";
    constexpr const char* transSustain    = "trans_sustain";
    constexpr const char* transSpeed      = "trans_speed";
    constexpr const char* transLink       = "trans_link";
    constexpr const char* transWetDry     = "trans_wetdry";

    // Clipper
    constexpr const char* clipBypass      = "clip_bypass";
    constexpr const char* clipMode        = "clip_mode";
    constexpr const char* clipCeiling     = "clip_ceiling";
    constexpr const char* clipMix         = "clip_mix";
    constexpr const char* clipAutoGain    = "clip_auto_gain";
    constexpr const char* clipBias        = "clip_bias";
    constexpr const char* clipOsQuality   = "clip_os_quality";
    constexpr const char* clipEmphFreq    = "clip_emph_freq";
    constexpr const char* clipEmphGain    = "clip_emph_gain";
    constexpr const char* clipKneeWidth   = "clip_knee_width";

    // Limiter
    constexpr const char* limBypass       = "lim_bypass";
    constexpr const char* limCeiling      = "lim_ceiling";
    constexpr const char* limRelease      = "lim_release";

    // Gate
    constexpr const char* gateBypass      = "gate_bypass";
    constexpr const char* gateThreshold   = "gate_threshold";
    constexpr const char* gateAttack      = "gate_attack";
    constexpr const char* gateRelease     = "gate_release";
    constexpr const char* gateRange       = "gate_range";

    // LFO 1 / LFO 2
    constexpr const char* lfo1Rate        = "lfo1_rate";
    constexpr const char* lfo1Shape       = "lfo1_shape";
    constexpr const char* lfo1Depth       = "lfo1_depth";
    constexpr const char* lfo2Rate        = "lfo2_rate";
    constexpr const char* lfo2Shape       = "lfo2_shape";
    constexpr const char* lfo2Depth       = "lfo2_depth";

    // Chorus
    constexpr const char* chorusBypass    = "chorus_bypass";
    constexpr const char* chorusRate      = "chorus_rate";
    constexpr const char* chorusDepth     = "chorus_depth";
    constexpr const char* chorusCentre    = "chorus_centre";
    constexpr const char* chorusVoices    = "chorus_voices";
    constexpr const char* chorusWetDry    = "chorus_wetdry";

    // Flanger
    constexpr const char* flangerBypass   = "flanger_bypass";
    constexpr const char* flangerRate     = "flanger_rate";
    constexpr const char* flangerDepth    = "flanger_depth";
    constexpr const char* flangerCentre   = "flanger_centre";
    constexpr const char* flangerFeedback = "flanger_feedback";
    constexpr const char* flangerWetDry   = "flanger_wetdry";

    // Gain
    constexpr const char* gainBypass      = "gain_bypass";
    constexpr const char* gainDb          = "gain_db";

    // Chain order (0..11 — sorted to determine processing order)
    constexpr const char* posFilter       = "pos_filter";
    constexpr const char* posEq           = "pos_eq";
    constexpr const char* posGain         = "pos_gain";
    constexpr const char* posDelay        = "pos_delay";
    constexpr const char* posReverb       = "pos_reverb";
    constexpr const char* posWaveshaper   = "pos_ws";
    constexpr const char* posCompressor   = "pos_comp";
    constexpr const char* posLimiter      = "pos_lim";
    constexpr const char* posGate         = "pos_gate";
    constexpr const char* posChorus       = "pos_chorus";
    constexpr const char* posFlanger      = "pos_flanger";
    constexpr const char* posPhaser       = "pos_phaser";
    constexpr const char* posClipper      = "pos_clipper";
    constexpr const char* posTransient    = "pos_transient";

    // Phaser
    constexpr const char* phaserBypass    = "phaser_bypass";
    constexpr const char* phaserRate      = "phaser_rate";
    constexpr const char* phaserDepth     = "phaser_depth";
    constexpr const char* phaserBaseFreq  = "phaser_base_freq";
    constexpr const char* phaserFeedback  = "phaser_feedback";
    constexpr const char* phaserStages    = "phaser_stages";
    constexpr const char* phaserWetDry    = "phaser_wetdry";
}
