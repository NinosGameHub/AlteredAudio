#pragma once
#include <JuceHeader.h>
#include "ModuleChain.h"
#include "FilterModule.h"
#include "EQModule.h"
#include "DelayModule.h"
#include "ReverbModule.h"
#include "WaveshaperModule.h"
#include "CompressorModule.h"
#include "ClipperModule.h"
#include "TransientShaperModule.h"
#include "LimiterModule.h"
#include "GateModule.h"
#include "GainModule.h"
#include "ChorusModule.h"
#include "FlangerModule.h"
#include "PhaserModule.h"
#include "SpatialModule.h"
#include "LFOEngine.h"
#include "EnvelopeFollower.h"
#include "ModulationMatrix.h"

enum class ChannelMode { Stereo, Mono, MidSide };

class AlteredAudioProcessor : public juce::AudioProcessor
{
public:
    AlteredAudioProcessor();
    ~AlteredAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    ModuleChain& getChain()            { return chain; }
    void setChannelMode(ChannelMode m) { channelMode = m; }
    ChannelMode getChannelMode() const { return channelMode; }

    juce::AudioProcessorValueTreeState& getAPVTS()   { return apvts; }
    ModulationMatrix&  getModulationMatrix()          { return modMatrix; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    ModuleChain chain;
    ChannelMode channelMode    = ChannelMode::Stereo;
    double      currentSampleRate = 44100.0;
    int         currentBlockSize  = 512;

    juce::AudioProcessorValueTreeState apvts;

    LFOEngine        lfo1, lfo2;
    EnvelopeFollower envFollower;
    ModulationMatrix modMatrix;

    // Non-owning raw pointers into chain — valid for the processor's lifetime
    FilterModule*     filterMod     = nullptr;
    EQModule*         eqMod         = nullptr;
    GainModule*       gainMod       = nullptr;
    DelayModule*      delayMod      = nullptr;
    ReverbModule*     reverbMod     = nullptr;
    WaveshaperModule* waveshaperMod = nullptr;
    CompressorModule* compressorMod = nullptr;
    ClipperModule*          clipperMod    = nullptr;
    TransientShaperModule*  transientMod  = nullptr;
    LimiterModule*          limiterMod    = nullptr;
    GateModule*       gateMod       = nullptr;
    ChorusModule*     chorusMod     = nullptr;
    FlangerModule*    flangerMod    = nullptr;
    PhaserModule*     phaserMod     = nullptr;
    SpatialModule*    spatialMod    = nullptr;

    // Cached APVTS raw parameter pointers — set in constructor, zero-cost to dereference on audio thread
    std::atomic<float>* p_channelMode   = nullptr;

    std::atomic<float>* p_filterBypass  = nullptr;
    std::atomic<float>* p_filterType    = nullptr;
    std::atomic<float>* p_filterFreq    = nullptr;
    std::atomic<float>* p_filterQ       = nullptr;
    std::atomic<float>* p_filterGain    = nullptr;

    std::atomic<float>* p_eqBypass                          = nullptr;
    std::atomic<float>* p_eqEnabled[EQModule::kMaxBands]    = {};
    std::atomic<float>* p_eqType   [EQModule::kMaxBands]    = {};
    std::atomic<float>* p_eqFreq   [EQModule::kMaxBands]    = {};
    std::atomic<float>* p_eqQ      [EQModule::kMaxBands]    = {};
    std::atomic<float>* p_eqGain   [EQModule::kMaxBands]    = {};

    std::atomic<float>* p_delayBypass    = nullptr;
    std::atomic<float>* p_delayTime      = nullptr;
    std::atomic<float>* p_delayFeedback  = nullptr;
    std::atomic<float>* p_delaySpread    = nullptr;
    std::atomic<float>* p_delayFbLPHz    = nullptr;
    std::atomic<float>* p_delayFbHPHz    = nullptr;
    std::atomic<float>* p_delayDucking   = nullptr;
    std::atomic<float>* p_delayDiffusion = nullptr;
    std::atomic<float>* p_delayWow       = nullptr;
    std::atomic<float>* p_delayModRate   = nullptr;
    std::atomic<float>* p_delayModDepth  = nullptr;
    std::atomic<float>* p_delayWetDry    = nullptr;
    std::atomic<float>* p_delayPingPong  = nullptr;

    std::atomic<float>* p_reverbBypass    = nullptr;
    std::atomic<float>* p_reverbRoom      = nullptr;
    std::atomic<float>* p_reverbDamp      = nullptr;
    std::atomic<float>* p_reverbDiffusion = nullptr;
    std::atomic<float>* p_reverbPreDelay  = nullptr;
    std::atomic<float>* p_reverbModRate   = nullptr;
    std::atomic<float>* p_reverbErLevel   = nullptr;
    std::atomic<float>* p_reverbLowCut    = nullptr;
    std::atomic<float>* p_reverbShimmer   = nullptr;
    std::atomic<float>* p_reverbPitchSemi = nullptr;
    std::atomic<float>* p_reverbWetDry    = nullptr;

    std::atomic<float>* p_wsBypass   = nullptr;
    std::atomic<float>* p_wsAlgo     = nullptr;
    std::atomic<float>* p_wsDrive    = nullptr;
    std::atomic<float>* p_wsWetDry   = nullptr;
    std::atomic<float>* p_wsBitDepth = nullptr;

    std::atomic<float>* p_compBypass      = nullptr;
    std::atomic<float>* p_compThreshold   = nullptr;
    std::atomic<float>* p_compRatio       = nullptr;
    std::atomic<float>* p_compAttack      = nullptr;
    std::atomic<float>* p_compRelease     = nullptr;
    std::atomic<float>* p_compKnee        = nullptr;
    std::atomic<float>* p_compMakeup      = nullptr;
    std::atomic<float>* p_compRmsPeak     = nullptr;
    std::atomic<float>* p_compAutoRel     = nullptr;
    std::atomic<float>* p_compSaturation  = nullptr;
    std::atomic<float>* p_compScHP        = nullptr;
    std::atomic<float>* p_compScLP        = nullptr;
    std::atomic<float>* p_compStereoLink  = nullptr;
    std::atomic<float>* p_compMix         = nullptr;
    std::atomic<float>* p_compLookahead   = nullptr;

    std::atomic<float>* p_clipBypass     = nullptr;
    std::atomic<float>* p_clipMode       = nullptr;
    std::atomic<float>* p_clipCeiling    = nullptr;
    std::atomic<float>* p_clipMix        = nullptr;
    std::atomic<float>* p_clipAutoGain   = nullptr;
    std::atomic<float>* p_clipBias       = nullptr;
    std::atomic<float>* p_clipOsQuality  = nullptr;
    std::atomic<float>* p_clipEmphFreq   = nullptr;
    std::atomic<float>* p_clipEmphGain   = nullptr;
    std::atomic<float>* p_clipKneeWidth  = nullptr;

    std::atomic<float>* p_transBypass  = nullptr;
    std::atomic<float>* p_transAttack  = nullptr;
    std::atomic<float>* p_transSustain = nullptr;
    std::atomic<float>* p_transSpeed   = nullptr;
    std::atomic<float>* p_transLink    = nullptr;
    std::atomic<float>* p_transWetDry  = nullptr;

    std::atomic<float>* p_limBypass  = nullptr;
    std::atomic<float>* p_limCeiling = nullptr;
    std::atomic<float>* p_limRelease = nullptr;

    std::atomic<float>* p_gateBypass    = nullptr;
    std::atomic<float>* p_gateThreshold = nullptr;
    std::atomic<float>* p_gateAttack    = nullptr;
    std::atomic<float>* p_gateRelease   = nullptr;
    std::atomic<float>* p_gateRange     = nullptr;

    std::atomic<float>* p_lfo1Rate  = nullptr;
    std::atomic<float>* p_lfo1Shape = nullptr;
    std::atomic<float>* p_lfo1Depth = nullptr;
    std::atomic<float>* p_lfo2Rate  = nullptr;
    std::atomic<float>* p_lfo2Shape = nullptr;
    std::atomic<float>* p_lfo2Depth = nullptr;

    std::atomic<float>* p_chorusBypass  = nullptr;
    std::atomic<float>* p_chorusRate    = nullptr;
    std::atomic<float>* p_chorusDepth   = nullptr;
    std::atomic<float>* p_chorusCentre  = nullptr;
    std::atomic<float>* p_chorusVoices  = nullptr;
    std::atomic<float>* p_chorusWetDry  = nullptr;

    std::atomic<float>* p_flangerBypass   = nullptr;
    std::atomic<float>* p_flangerRate     = nullptr;
    std::atomic<float>* p_flangerDepth    = nullptr;
    std::atomic<float>* p_flangerCentre   = nullptr;
    std::atomic<float>* p_flangerFeedback = nullptr;
    std::atomic<float>* p_flangerWetDry   = nullptr;

    std::atomic<float>* p_phaserBypass   = nullptr;
    std::atomic<float>* p_phaserRate     = nullptr;
    std::atomic<float>* p_phaserDepth    = nullptr;
    std::atomic<float>* p_phaserBaseFreq = nullptr;
    std::atomic<float>* p_phaserFeedback = nullptr;
    std::atomic<float>* p_phaserStages   = nullptr;
    std::atomic<float>* p_phaserWetDry   = nullptr;

    std::atomic<float>* p_gainBypass = nullptr;
    std::atomic<float>* p_gainDb     = nullptr;

    std::atomic<float>* p_posFilter     = nullptr;
    std::atomic<float>* p_posEq         = nullptr;
    std::atomic<float>* p_posGain       = nullptr;
    std::atomic<float>* p_posDelay      = nullptr;
    std::atomic<float>* p_posReverb     = nullptr;
    std::atomic<float>* p_posWaveshaper = nullptr;
    std::atomic<float>* p_posCompressor = nullptr;
    std::atomic<float>* p_posLimiter    = nullptr;
    std::atomic<float>* p_posGate       = nullptr;
    std::atomic<float>* p_posChorus     = nullptr;
    std::atomic<float>* p_posFlanger    = nullptr;
    std::atomic<float>* p_posPhaser      = nullptr;
    std::atomic<float>* p_posClipper     = nullptr;
    std::atomic<float>* p_posTransient   = nullptr;
    std::atomic<float>* p_posSpatial     = nullptr;

    std::atomic<float>* p_spatialBypass   = nullptr;
    std::atomic<float>* p_spatialWidth    = nullptr;
    std::atomic<float>* p_spatialPan      = nullptr;
    std::atomic<float>* p_spatialRotation = nullptr;
    std::atomic<float>* p_spatialMidGain  = nullptr;
    std::atomic<float>* p_spatialSideGain = nullptr;
    std::atomic<float>* p_spatialBassMono = nullptr;
    std::atomic<float>* p_spatialHaasMs   = nullptr;
    std::atomic<float>* p_spatialWetDry   = nullptr;

    void cacheParameterPointers();
    void updateModuleParameters();

    void encodeMidSide(juce::AudioBuffer<float>& buffer);
    void decodeMidSide(juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlteredAudioProcessor)
};
