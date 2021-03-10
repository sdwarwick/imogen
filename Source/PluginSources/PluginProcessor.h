/*
    This file defines Imogen's internal audio processor as a whole, when Imogen is built as a plugin target
*/

#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "bv_ImogenEngine/bv_ImogenEngine.h"


#ifndef IMOGEN_ONLY_BUILDING_STANDALONE
  #define IMOGEN_ONLY_BUILDING_STANDALONE 0
#endif

#ifndef IMOGEN_NOT_BUILDING_STANDALONE
  #define IMOGEN_NOT_BUILDING_STANDALONE 0
#endif

#if ! IMOGEN_ONLY_BUILDING_STANDALONE
  #include <juce_audio_plugin_client/utility/juce_PluginHostType.h>
#endif


class ImogenAudioProcessorEditor; // forward declaration...

///////////

class ImogenAudioProcessor    : public juce::AudioProcessor

{
    
public:
    
    // standard & general-purpose functions ---------------------------------------------------------------------------------------------------------
    ImogenAudioProcessor();
    ~ImogenAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    
    void releaseResources() override;
    
    void reset() override;
    
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void processBlockBypassed (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override;
    
    bool canAddBus (bool isInput) const override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    bool shouldWarnUserToEnableSidechain() const;
#endif
    
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    
    const juce::String getName() const override { return JucePlugin_Name; }
    
    bool acceptsMidi()  const override { return true;  }
    bool producesMidi() const override { return true;  }
    bool supportsMPE()  const override { return false; }
    bool isMidiEffect() const override { return false; }
    
    double getTailLengthSeconds() const override;
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void savePreset  (juce::String presetName);
    bool loadPreset  (juce::String presetName);
    void deletePreset(juce::String presetName);
    juce::File getPresetsFolder() const;
    
    void updateModulatorInputSource (const int newSource);
    
    void returnActivePitches(juce::Array<int>& outputArray) const;
    
    void killAllMidi();
    
    void pitchbendFromEditor (const int pitchbend);
    
    juce::AudioProcessorValueTreeState tree;
    
    juce::AudioProcessorParameter* getBypassParameter() const override;
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    void updateNumVoices (const int newNumVoices);
    
    void setMidiLatch (const bool isOn);
    void setIntervalLock (const bool isOn);
    
    // these functions return parameters' current state/value
    int getDryPan() const { return dryPan->get(); }
    int getDryWet() const { return dryWet->get(); }
    float getInputGain() const { return inputGain->get(); }
    float getOutputGain() const { return outputGain->get(); }
    bool getIsLimiterOn() const { return limiterToggle->get(); }
    float getAdsrAttack() const { return adsrAttack->get(); }
    float getAdsrDecay() const { return adsrDecay->get(); }
    float getAdsrSustain() const { return adsrSustain->get(); }
    float getAdsrRelease() const { return adsrRelease->get(); }
    bool getIsAdsrOn() const { return adsrToggle->get(); }
    bool getIsMidiLatchOn() const { return latchIsOn.load(); }
    bool getIsIntervalLockOn() const { return intervalLockIsOn.load(); }
    int getQuickKillMs() const { return quickKillMs->get(); }
    int getQuickAttackMs() const { return quickAttackMs->get(); }
    int getStereoWidth() const { return stereoWidth->get(); }
    int getLowestPannedNote() const { return lowestPanned->get(); }
    int getMidiVelocitySensitivity() const { return velocitySens->get(); }
    int getPitchbendRangeUp() const { return pitchBendUp->get(); }
    int getPitchbendRangeDown() const { return pitchBendDown->get(); }
    bool getIsVoiceStealingEnabled() const { return voiceStealing->get(); }
    int getConcertPitchHz() const { return concertPitchHz->get(); }
    bool getIsPedalPitchOn() const { return pedalPitchIsOn->get(); }
    int getPedalPitchThresh() const { return pedalPitchThresh->get(); }
    int getPedalPitchInterval() const { return pedalPitchInterval->get(); }
    bool getIsDescantOn() const { return descantIsOn->get(); }
    int getDescantThresh() const { return descantThresh->get(); }
    int getDescantInterval() const { return descantInterval->get(); }
    
    
    // these functions return the default values for each parameter, according to the most recently loaded state from the host, or user-selected preset.
    int getDefaultDryPan() const { return defaultDryPan.load(); }
    int getDefaultDryWet() const { return defaultDryWet.load(); }
    float getDefaultInputGain() const { return defaultInputGain.load(); }
    float getDefaultOutputGain() const { return defaultOutputGain.load(); }
    float getDefaultAdsrAttack() const { return defaultAdsrAttack.load(); }
    float getDefaultAdsrDecay() const { return defaultAdsrDecay.load(); }
    float getDefaultAdsrSustain() const { return defaultAdsrSustain.load(); }
    float getDefaultAdsrRelease() const { return defaultAdsrRelease.load(); }
    int getDefaultQuickKillMs() const { return defaultQuickKillMs.load(); }
    int getDefaultStereoWidth() const { return defaultStereoWidth.load(); }
    int getDefaultLowestPannedNote() const { return defaultLowestPannedNote.load(); }
    int getDefaultMidiVelocitySensitivity() const { return defaultVelocitySensitivity.load(); }
    int getDefaultConcertPitchHz() const { return defaultConcertPitchHz.load(); }
    int getDefaultPedalPitchThresh() const { return defaultPedalPitchThresh.load(); }
    int getDefaultDescantThresh() const { return defaultDescantThresh.load(); }
    
    bool hasUpdatedParamDefaults() const { return parameterDefaultsAreDirty.load(); }
    
    
private:
    
    template<typename SampleType>
    void updateAllParameters (bav::ImogenEngine<SampleType>& activeEngine);
    
    void updateVocalRangeType (int rangeTypeIndex);
    
    void updatePitchDetectionHzRange (int minHz, int maxHz);
    
    template <typename SampleType>
    void initialize (bav::ImogenEngine<SampleType>& activeEngine);
    
    template <typename SampleType1, typename SampleType2>
    void prepareToPlayWrapped (const double sampleRate, const int samplesPerBlock,
                               bav::ImogenEngine<SampleType1>& activeEngine,
                               bav::ImogenEngine<SampleType2>& idleEngine);
    
    
    template <typename SampleType>
    inline void processBlockWrapped (juce::AudioBuffer<SampleType>& buffer,
                                     juce::MidiBuffer& midiMessages,
                                     bav::ImogenEngine<SampleType>& engine,
                                     const bool isBypassed = false);
    
    
    bav::ImogenEngine<float>  floatEngine;
    bav::ImogenEngine<double> doubleEngine;
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    juce::PluginHostType host;
    bool needsSidechain = false;
#endif
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void initializeParameterPointers();
    
    juce::AudioProcessor::BusesProperties makeBusProperties() const;
    
    template<typename SampleType>
    std::unique_ptr<juce::XmlElement> pluginStateToXml (bav::ImogenEngine<SampleType>& activeEngine);
    
    template<typename SampleType>
    bool updatePluginInternalState (juce::XmlElement& newState, bav::ImogenEngine<SampleType>& activeEngine);
    
    std::atomic<bool> latchIsOn, intervalLockIsOn;
    
    // listener variables linked to AudioProcessorValueTreeState parameters:
    juce::AudioParameterBool*  isBypassed;
    juce::AudioParameterInt*   dryPan;
    juce::AudioParameterInt*   dryWet;
    juce::AudioParameterFloat* adsrAttack;
    juce::AudioParameterFloat* adsrDecay;
    juce::AudioParameterFloat* adsrSustain;
    juce::AudioParameterFloat* adsrRelease;
    juce::AudioParameterBool*  adsrToggle;
    juce::AudioParameterInt*   quickKillMs;
    juce::AudioParameterInt*   quickAttackMs;
    juce::AudioParameterInt*   stereoWidth;
    juce::AudioParameterInt*   lowestPanned;
    juce::AudioParameterInt*   velocitySens;
    juce::AudioParameterInt*   pitchBendUp;
    juce::AudioParameterInt*   pitchBendDown;
    juce::AudioParameterBool*  pedalPitchIsOn;
    juce::AudioParameterInt*   pedalPitchThresh;
    juce::AudioParameterInt*   pedalPitchInterval;
    juce::AudioParameterBool*  descantIsOn;
    juce::AudioParameterInt*   descantThresh;
    juce::AudioParameterInt*   descantInterval;
    juce::AudioParameterInt*   concertPitchHz;
    juce::AudioParameterBool*  voiceStealing;
    juce::AudioParameterFloat* inputGain;
    juce::AudioParameterFloat* outputGain;
    juce::AudioParameterBool*  limiterToggle;
    juce::AudioParameterFloat* softPedalGain;
    juce::AudioParameterFloat* pitchDetectionConfidenceUpperThresh;
    juce::AudioParameterFloat* pitchDetectionConfidenceLowerThresh;
    juce::AudioParameterChoice* vocalRangeType;
    juce::AudioParameterBool*  aftertouchGainToggle;
    juce::AudioParameterBool*  channelPressureToggle;
    juce::AudioParameterFloat* playingButReleasedGain;
    
    void updateParameterDefaults();
    
    std::atomic<bool> parameterDefaultsAreDirty;
    
    std::atomic<int> defaultDryPan, defaultDryWet, defaultQuickKillMs, defaultQuickAttackMs, defaultStereoWidth, defaultLowestPannedNote, defaultVelocitySensitivity, defaultPitchbendUp, defaultPitchbendDown, defaultPedalPitchThresh, defaultPedalPitchInterval, defaultDescantThresh, defaultDescantInterval, defaultConcertPitchHz;
    std::atomic<float> defaultAdsrAttack, defaultAdsrDecay, defaultAdsrSustain, defaultAdsrRelease, defaultInputGain, defaultOutputGain, defaultSoftPedalGain, defaultPitchUpperConfidenceThresh, defaultPitchLowerConfidenceThresh, defaultPlayingButReleasedGain;
    
    int prevRangeTypeIndex;
    
    juce::StringArray vocalRangeTypes;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
