#pragma once

#include <JuceHeader.h>

#include "GlobalDefinitions.h"
#include "Harmonizer.h"

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
    
    void releaseResources() override { reset(); };
    
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    
    void processBlock (juce::AudioBuffer<float>&  buffer, juce::MidiBuffer& midiMessages) override;
   
    void processBlockBypassed (AudioBuffer<float>&  buffer, MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; };
    
    const juce::String getName() const override { return JucePlugin_Name; };
    
    bool  acceptsMidi()  const override { return true;  };
    bool  producesMidi() const override { return true; };
    bool  supportsMPE()  const override { return false; };
    bool  isMidiEffect() const override { return false; };
    
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
    void updateIOgains();
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
    void updatePedalPitch();
    void updateDescant();
    
    void updatePitchDetectionSettings(const float newMinHz, const float newMaxHz, const float newTolerance);
    
    // misc utility functions -----------------------------------------------------------------------------------------------------------------------
    Array<int> returnActivePitches() const { return harmonizer.reportActiveNotes(); };
    
    void killAllMidi() { reset(); };
    
    void updateNumVoices(const int newNumVoices); // updates the # of cuncurrently running instances of the pitch shifting algorithm
    
    AudioProcessorValueTreeState tree;
    
    enum ModulatorInputSource { left, right, mixToMono }; // determines how the plugin will take input from the stereo buffer fed to it from the host
    
    void changeModulatorInputSource(ModulatorInputSource newSource) noexcept { modulatorInput = newSource; };
    
    bool canAddBus(bool isInput) const override;
    
    bool shouldWarnUserToEnableSidechain() const;
    
    bool supportsDoublePrecisionProcessing() const override { return false; };
    
    void updateTrackProperties (const TrackProperties& properties) override; // informs the plugin about the properties of the DAW mixer track it's loaded on
        
    //==============================================================================
    
private:
    
    // the top-level processBlock call redirects here, just so that this function can be wrapped & templated for either float or double AudioBuffer types.
    void processBlockWrapped (AudioBuffer<float>& inBus, AudioBuffer<float>& output, MidiBuffer& midiMessages,
                              const bool applyFadeIn, const bool applyFadeOut);
    
    // takes the chunks in between midi messages and makes sure they don't exceed the internal preallocated buffer sizes
    // if they do, this function breaks the in/out buffers into smaller chunks & calls renderChunk() on each of these in sequence.
    void renderBlock (AudioBuffer<float>& inBuffer, AudioBuffer<float>& outBuffer,
                      const int startSample, const int numSamples);
    
    // this function actually does the audio processing on a chunk of audio.
    void renderChunk (const AudioBuffer<float>& inBuffer, AudioBuffer<float>& outBuffer);
    
    void updateAllParameters();
    void updateSampleRate(const double newSamplerate);
    void clearBuffers();
    
    Harmonizer harmonizer;
    
    AudioBuffer<float> inBuffer;  // this buffer is used to store the mono input signal so that input gain can be applied
    AudioBuffer<float> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
    AudioBuffer<float> dryBuffer; // this buffer is used for panning & delaying the dry signal
    AudioBuffer<float> monoSummingBuffer; // this buffer is used in case a multichannel input needs to be summed to mono.
    
    dsp::ProcessSpec dspSpec;
    dsp::Limiter <float> limiter;
    dsp::DryWetMixer<float> dryWetMixer;
    dsp::DelayLine<float> bypassDelay; // a delay line used for latency compensation when the processing is bypassed
    
    // these variables store CURRENT states:
    bool limiterIsOn;
    float inputGainMultiplier, outputGainMultiplier;
    
    int dryvoxpanningmults[2]; // stores gain multiplier values, which when applied to the input signal, achieve the desired dry vox panning
    
    // variables to store previous parameter values, to avoid unnecessary update operations:
    int prevDryPan;
    float prevideb, prevodeb;
    
    ModulatorInputSource modulatorInput;
    
    // listener variables linked to AudioProcessorValueTreeState parameters:
    AudioParameterInt* 	 dryPan             = nullptr;
    AudioParameterInt* 	 dryWet             = nullptr;
    AudioParameterInt* 	 inputChan          = nullptr;
    AudioParameterFloat* adsrAttack         = nullptr;
    AudioParameterFloat* adsrDecay          = nullptr;
    AudioParameterFloat* adsrSustain        = nullptr;
    AudioParameterFloat* adsrRelease        = nullptr;
    AudioParameterBool*  adsrToggle         = nullptr;
    AudioParameterInt*	 quickKillMs        = nullptr;
    AudioParameterInt* 	 quickAttackMs      = nullptr;
    AudioParameterInt*   stereoWidth        = nullptr;
    AudioParameterInt*   lowestPanned       = nullptr;
    AudioParameterInt*   velocitySens       = nullptr;
    AudioParameterInt*   pitchBendUp        = nullptr;
    AudioParameterInt*   pitchBendDown      = nullptr;
    AudioParameterBool*  pedalPitchIsOn     = nullptr;
    AudioParameterInt*   pedalPitchThresh   = nullptr;
    AudioParameterInt*   pedalPitchInterval = nullptr;
    AudioParameterBool*  descantIsOn        = nullptr;
    AudioParameterInt*	 descantThresh      = nullptr;
    AudioParameterInt*	 descantInterval    = nullptr;
    AudioParameterInt*	 concertPitchHz     = nullptr;
    AudioParameterBool*  voiceStealing      = nullptr;
    AudioParameterBool*  latchIsOn          = nullptr;
    AudioParameterFloat* inputGain          = nullptr;
    AudioParameterFloat* outputGain	        = nullptr;
    AudioParameterBool*	 limiterToggle      = nullptr;
    AudioParameterFloat* limiterThresh      = nullptr;
    AudioParameterInt*   limiterRelease     = nullptr;
    
    PluginHostType host;
    
    
    void writeToDryBuffer(const AudioBuffer<float>& input);
    
    AudioProcessorValueTreeState::ParameterLayout createParameters() const;
    
    AudioProcessor::BusesProperties makeBusProperties() const;
    
    void initialize(const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices);
    
    void increaseBufferSizes(const int newMaxBlocksize);
    
    void resizeBuffers(const int newBlocksize);
    
    bool wasBypassedLastCallback; // used to activate a fade out instead of an instant kill when the bypass is activated
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};



