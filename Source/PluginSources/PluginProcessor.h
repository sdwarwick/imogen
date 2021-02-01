#pragma once

#include <JuceHeader.h>

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
    
    
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;
    
    void processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override;
    
    void processBlockBypassed (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;
    
    void processBlockBypassed (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override;
    
    
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
    
    void returnActivePitches(Array<int>& outputArray) const;
    
    void killAllMidi();
    
    AudioProcessorValueTreeState tree;
    
    bool canAddBus(bool isInput) const override;
    
    bool shouldWarnUserToEnableSidechain() const;
    
    bool supportsDoublePrecisionProcessing() const override { return true; }
    
    void updateTrackProperties (const TrackProperties& properties) override; // informs the plugin about the properties of the DAW mixer track it's loaded on
    
    void updateNumVoices(const int newNumVoices);
    
    //==============================================================================
    
    
    template<typename SampleType>
    void updateAllParameters (ImogenEngine<SampleType>& activeEngine);
    
    // listener variables linked to AudioProcessorValueTreeState parameters:
    AudioParameterInt*   dryPan             = nullptr;
    AudioParameterInt*   dryWet             = nullptr;
    AudioParameterInt*   inputChan          = nullptr;
    AudioParameterFloat* adsrAttack         = nullptr;
    AudioParameterFloat* adsrDecay          = nullptr;
    AudioParameterFloat* adsrSustain        = nullptr;
    AudioParameterFloat* adsrRelease        = nullptr;
    AudioParameterBool*  adsrToggle         = nullptr;
    AudioParameterInt*   quickKillMs        = nullptr;
    AudioParameterInt*   quickAttackMs      = nullptr;
    AudioParameterInt*   stereoWidth        = nullptr;
    AudioParameterInt*   lowestPanned       = nullptr;
    AudioParameterInt*   velocitySens       = nullptr;
    AudioParameterInt*   pitchBendUp        = nullptr;
    AudioParameterInt*   pitchBendDown      = nullptr;
    AudioParameterBool*  pedalPitchIsOn     = nullptr;
    AudioParameterInt*   pedalPitchThresh   = nullptr;
    AudioParameterInt*   pedalPitchInterval = nullptr;
    AudioParameterBool*  descantIsOn        = nullptr;
    AudioParameterInt*   descantThresh      = nullptr;
    AudioParameterInt*   descantInterval    = nullptr;
    AudioParameterInt*   concertPitchHz     = nullptr;
    AudioParameterBool*  voiceStealing      = nullptr;
    AudioParameterBool*  latchIsOn          = nullptr;
    AudioParameterBool*  intervalLockIsOn   = nullptr;
    AudioParameterFloat* inputGain          = nullptr;
    AudioParameterFloat* outputGain         = nullptr;
    AudioParameterBool*  limiterToggle      = nullptr;
    AudioParameterFloat* limiterThresh      = nullptr;
    AudioParameterInt*   limiterRelease     = nullptr;
    AudioParameterFloat* dryGain            = nullptr;
    AudioParameterFloat* wetGain            = nullptr;
    AudioParameterFloat* softPedalGain      = nullptr;
    AudioParameterInt*   minDetectedHz      = nullptr;
    AudioParameterInt*   maxDetectedHz      = nullptr;
    AudioParameterFloat* confidenceThresh   = nullptr;
    
    
private:
    
    template <typename SampleType>
    void initialize (ImogenEngine<SampleType>& activeEngine);
    
    template <typename SampleType>
    void processBlockWrapped (AudioBuffer<SampleType>& buffer, MidiBuffer& midiMessages, ImogenEngine<SampleType>& engine);
    
    template <typename SampleType>
    void processBlockBypassedWrapped (AudioBuffer<SampleType>& buffer, MidiBuffer& midiMessages, ImogenEngine<SampleType>& engine);
    
    template <typename SampleType1, typename SampleType2>
    void prepareToPlayWrapped (const double sampleRate, const int samplesPerBlock,
                               ImogenEngine<SampleType1>& activeEngine,
                               ImogenEngine<SampleType2>& idleEngine);
    
    
    ImogenEngine<float>  floatEngine;
    ImogenEngine<double> doubleEngine;
    
    int latencySamples;
    
    PluginHostType host;
    
    AudioProcessorValueTreeState::ParameterLayout createParameters() const;
    
    AudioProcessor::BusesProperties makeBusProperties() const;
    
    bool wasBypassedLastCallback; // used to activate a fade out instead of an instant kill when the bypass is activated
    
    
    template <typename SampleType>
    void updatePitchDetectionWrapped (ImogenEngine<SampleType>& activeEngine);
    
    template <typename SampleType>
    void updateGainsPrivate (ImogenEngine<SampleType>& activeEngine);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
