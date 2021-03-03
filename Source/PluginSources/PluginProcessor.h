/*
    This file defines Imogen's internal audio processor as a whole, when Imogen is built as a plugin target
*/

#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#ifndef IMOGEN_ONLY_BUILDING_STANDALONE
  #define IMOGEN_ONLY_BUILDING_STANDALONE 0
#endif

#ifndef IMOGEN_NOT_BUILDING_STANDALONE
  #define IMOGEN_NOT_BUILDING_STANDALONE 0
#endif

#if ! IMOGEN_ONLY_BUILDING_STANDALONE
  #include <juce_audio_plugin_client/utility/juce_PluginHostType.h>
#endif


#include "bv_ImogenEngine/bv_ImogenEngine.h"


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
    
    // functions for custom preset management system ------------------------------------------------------------------------------------------------
    void savePreset  (juce::String presetName);
    bool loadPreset  (juce::String presetName);
    void deletePreset(juce::String presetName);
    juce::File getPresetsFolder() const;
    
    // functions to update parameters ---------------------------------------------------------------------------------------------------------------
    void updateAdsr();
    void updateLimiter();
    void updateStereoWidth();
    void updateQuickKillMs();
    void updateQuickAttackMs();
    void updateDryVoxPan();
    void updateMidiVelocitySensitivity();
    void updateNoteStealing();
    void updatePitchbendSettings();
    void updateDryWet();
    void updateConcertPitch();
    void updateMidiLatch();
    void updateIntervalLock();
    void updatePedalPitch();
    void updateDescant();
    void updatePitchDetectionSettings();
    void updateGains();
    void updateAftertouchGainToggle();
    void updateChannelPressureToggle();
    void updatePlayingButRelesedGain();
    
    void updateModulatorInputSource (const int newSource);
    
    // misc utility functions -----------------------------------------------------------------------------------------------------------------------
    
    void returnActivePitches(juce::Array<int>& outputArray) const;
    
    void killAllMidi();
    
    juce::AudioProcessorValueTreeState tree;
    
    juce::AudioProcessorParameter* getBypassParameter() const override;
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    void updateNumVoices (const int newNumVoices);
    
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
    juce::AudioParameterBool*  latchIsOn;
    juce::AudioParameterBool*  intervalLockIsOn;
    juce::AudioParameterFloat* inputGain;
    juce::AudioParameterFloat* outputGain;
    juce::AudioParameterBool*  limiterToggle;
    juce::AudioParameterFloat* limiterThresh;
    juce::AudioParameterInt*   limiterRelease;
    juce::AudioParameterFloat* dryGain;
    juce::AudioParameterFloat* wetGain;
    juce::AudioParameterFloat* softPedalGain;
    juce::AudioParameterInt*   minDetectedHz;
    juce::AudioParameterInt*   maxDetectedHz;
    juce::AudioParameterFloat* pitchDetectionConfidenceUpperThresh;
    juce::AudioParameterFloat* pitchDetectionConfidenceLowerThresh;
    juce::AudioParameterBool*  aftertouchGainToggle;
    juce::AudioParameterBool*  channelPressureToggle;
    juce::AudioParameterFloat* playingButReleasedGain;
    
    
private:
    
    template<typename SampleType>
    void updateAllParameters (bav::ImogenEngine<SampleType>& activeEngine);
    
    template <typename SampleType>
    void initialize (bav::ImogenEngine<SampleType>& activeEngine);
    
    template <typename SampleType1, typename SampleType2>
    void prepareToPlayWrapped (const double sampleRate, const int samplesPerBlock,
                               bav::ImogenEngine<SampleType1>& activeEngine,
                               bav::ImogenEngine<SampleType2>& idleEngine);
    
    
    template <typename SampleType>
    void processBlockWrapped (juce::AudioBuffer<SampleType>& buffer,
                              juce::MidiBuffer& midiMessages,
                              bav::ImogenEngine<SampleType>& engine,
                              const bool isBypassed = false);
    
    
    bav::ImogenEngine<float>  floatEngine;
    bav::ImogenEngine<double> doubleEngine;
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    juce::PluginHostType host;
    bool needsSidechain = false;
#endif
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters() const;
    
    juce::AudioProcessor::BusesProperties makeBusProperties() const;
    
    bool wasBypassedLastCallback; // used to activate a fade out instead of an instant kill when the bypass is activated
    
    template <typename SampleType>
    void updatePitchDetectionWrapped (bav::ImogenEngine<SampleType>& activeEngine);
    
    template <typename SampleType>
    void updateGainsPrivate (bav::ImogenEngine<SampleType>& activeEngine);
    
    template<typename SampleType>
    std::unique_ptr<juce::XmlElement> pluginStateToXml (bav::ImogenEngine<SampleType>& activeEngine);
    
    template<typename SampleType>
    bool updatePluginInternalState (juce::XmlElement& newState, bav::ImogenEngine<SampleType>& activeEngine);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
