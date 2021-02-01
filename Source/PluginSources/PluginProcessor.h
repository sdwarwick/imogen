#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <juce_audio_plugin_client/utility/juce_PluginHostType.h>

#include "bv_ImogenEngine/bv_ImogenEngine.h"


class ImogenAudioProcessorEditor; // forward declaration...


//==============================================================================
/**
 */

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
    
    
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    
    const juce::String getName() const override { return JucePlugin_Name; }
    
    bool  acceptsMidi()  const override { return true;  }
    bool  producesMidi() const override { return true;  }
    bool  supportsMPE()  const override { return false; }
    bool  isMidiEffect() const override { return false; }
    
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
    void loadPreset  (juce::String presetName);
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
    
    // misc utility functions -----------------------------------------------------------------------------------------------------------------------
    
    void returnActivePitches(juce::Array<int>& outputArray) const;
    
    void killAllMidi();
    
    juce::AudioProcessorValueTreeState tree;
    
    bool canAddBus(bool isInput) const override;
    
    bool shouldWarnUserToEnableSidechain() const;
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    void updateTrackProperties (const TrackProperties& properties) override; // informs the plugin about the properties of the DAW mixer track it's loaded on
    
    void updateNumVoices(const int newNumVoices);
    
    //==============================================================================
    
    
    template<typename SampleType>
    void updateAllParameters (ImogenEngine<SampleType>& activeEngine);
    
    // listener variables linked to AudioProcessorValueTreeState parameters:
    juce::AudioParameterInt*   dryPan             = nullptr;
    juce::AudioParameterInt*   dryWet             = nullptr;
    juce::AudioParameterInt*   inputChan          = nullptr;
    juce::AudioParameterFloat* adsrAttack         = nullptr;
    juce::AudioParameterFloat* adsrDecay          = nullptr;
    juce::AudioParameterFloat* adsrSustain        = nullptr;
    juce::AudioParameterFloat* adsrRelease        = nullptr;
    juce::AudioParameterBool*  adsrToggle         = nullptr;
    juce::AudioParameterInt*   quickKillMs        = nullptr;
    juce::AudioParameterInt*   quickAttackMs      = nullptr;
    juce::AudioParameterInt*   stereoWidth        = nullptr;
    juce::AudioParameterInt*   lowestPanned       = nullptr;
    juce::AudioParameterInt*   velocitySens       = nullptr;
    juce::AudioParameterInt*   pitchBendUp        = nullptr;
    juce::AudioParameterInt*   pitchBendDown      = nullptr;
    juce::AudioParameterBool*  pedalPitchIsOn     = nullptr;
    juce::AudioParameterInt*   pedalPitchThresh   = nullptr;
    juce::AudioParameterInt*   pedalPitchInterval = nullptr;
    juce::AudioParameterBool*  descantIsOn        = nullptr;
    juce::AudioParameterInt*   descantThresh      = nullptr;
    juce::AudioParameterInt*   descantInterval    = nullptr;
    juce::AudioParameterInt*   concertPitchHz     = nullptr;
    juce::AudioParameterBool*  voiceStealing      = nullptr;
    juce::AudioParameterBool*  latchIsOn          = nullptr;
    juce::AudioParameterBool*  intervalLockIsOn   = nullptr;
    juce::AudioParameterFloat* inputGain          = nullptr;
    juce::AudioParameterFloat* outputGain         = nullptr;
    juce::AudioParameterBool*  limiterToggle      = nullptr;
    juce::AudioParameterFloat* limiterThresh      = nullptr;
    juce::AudioParameterInt*   limiterRelease     = nullptr;
    juce::AudioParameterFloat* dryGain            = nullptr;
    juce::AudioParameterFloat* wetGain            = nullptr;
    juce::AudioParameterFloat* softPedalGain      = nullptr;
    juce::AudioParameterInt*   minDetectedHz      = nullptr;
    juce::AudioParameterInt*   maxDetectedHz      = nullptr;
    juce::AudioParameterFloat* confidenceThresh   = nullptr;
    
    
private:
    
    template <typename SampleType>
    void initialize (ImogenEngine<SampleType>& activeEngine);
    
    template <typename SampleType>
    void processBlockWrapped (juce::AudioBuffer<SampleType>& buffer, juce::MidiBuffer& midiMessages, ImogenEngine<SampleType>& engine);
    
    template <typename SampleType>
    void processBlockBypassedWrapped (juce::AudioBuffer<SampleType>& buffer, juce::MidiBuffer& midiMessages, ImogenEngine<SampleType>& engine);
    
    template <typename SampleType1, typename SampleType2>
    void prepareToPlayWrapped (const double sampleRate, const int samplesPerBlock,
                               ImogenEngine<SampleType1>& activeEngine,
                               ImogenEngine<SampleType2>& idleEngine);
    
    
    ImogenEngine<float>  floatEngine;
    ImogenEngine<double> doubleEngine;
    
    int latencySamples;
    
    juce::PluginHostType host;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters() const;
    
    juce::AudioProcessor::BusesProperties makeBusProperties() const;
    
    bool wasBypassedLastCallback; // used to activate a fade out instead of an instant kill when the bypass is activated
    
    
    template <typename SampleType>
    void updatePitchDetectionWrapped (ImogenEngine<SampleType>& activeEngine);
    
    template <typename SampleType>
    void updateGainsPrivate (ImogenEngine<SampleType>& activeEngine);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
