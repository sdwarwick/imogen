#pragma once

#include <JuceHeader.h>

#include "GlobalDefinitions.h"
#include "Harmonizer.h"


class ImogenAudioProcessorEditor; // forward declaration...

class ImogenAudioProcessor;



template<typename SampleType>
class ImogenEngine
{
public:
    ImogenEngine(ImogenAudioProcessor& p);
    
    ~ImogenEngine();
    
    void process (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages,
                  const bool applyFadeIn, const bool applyFadeOut, const bool chopInput);
    
    void processBypassed (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output);
    
    void initialize (const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices);
    
    void prepare (double sampleRate, int samplesPerBlock);
    
    void reset();
    
    void releaseResources();
    
    void updateNumVoices(const int newNumVoices); // updates the # of cuncurrently running instances of the pitch shifting algorithm
    
    Array<int> returnActivePitches() const { return harmonizer.reportActiveNotes(); };
    
    void updateSamplerate (const int newSamplerate);
    void updateDryWet     (const float newWetMixProportion);
    void updateAdsr       (const float attack, const float decay, const float sustain, const float release, const bool isOn);
    void updateQuickKill  (const int newMs);
    void updateQuickAttack(const int newMs);
    void updateStereoWidth(const int newStereoWidth, const int lowestPannedNote);
    void updateMidiVelocitySensitivity(const int newSensitivity);
    void updatePitchbendSettings(const int rangeUp, const int rangeDown);
    void updatePedalPitch  (const bool isOn, const int upperThresh, const int interval);
    void updateDescant     (const bool isOn, const int lowerThresh, const int interval);
    void updateConcertPitch(const int newConcertPitchHz);
    void updateNoteStealing(const bool shouldSteal);
    void updateMidiLatch   (const bool isLatched);
    void updateLimiter     (const float thresh, const float release);
    void updatePitchDetectionSettings (const float newMinHz, const float newMaxHz, const float newTolerance);
    
    void clearBuffers();
    
    bool hasBeenReleased()    const noexcept { return resourcesReleased; };
    bool hasBeenInitialized() const noexcept { return initialized; };
    
private:
    
    void processNoChopping (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages);
    
    void processWithChopping (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages);
    
    void renderBlock (AudioBuffer<SampleType>& inBuffer, AudioBuffer<SampleType>& outBuffer,
                      const int startSample, const int numSamples);
    
    void renderChunk (const AudioBuffer<SampleType>& inBuffer, AudioBuffer<SampleType>& outBuffer);
    
    ImogenAudioProcessor& processor;
    
    Harmonizer<SampleType> harmonizer;
    
    AudioBuffer<SampleType> inBuffer;  // this buffer is used to store the mono input signal so that input gain can be applied
    AudioBuffer<SampleType> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
    AudioBuffer<SampleType> dryBuffer; // this buffer is used for panning & delaying the dry signal
    AudioBuffer<SampleType> monoSummingBuffer; // this buffer is used in case a multichannel input needs to be summed to mono.
    
    dsp::ProcessSpec dspSpec;
    dsp::Limiter <SampleType> limiter;
    dsp::DryWetMixer<SampleType> dryWetMixer;
    dsp::DelayLine<SampleType> bypassDelay; // a delay line used for latency compensation when the processing is bypassed
    
    bool resourcesReleased;
    bool initialized;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenEngine)
};




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
    
    
    void processBlock (juce::AudioBuffer<float>&  buffer, juce::MidiBuffer& midiMessages) override
    {
        processBlockWrapped (buffer, midiMessages, floatEngine);
    };
    
    void processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override
    {
        processBlockWrapped (buffer, midiMessages, doubleEngine);
    };
   
    void processBlockBypassed (AudioBuffer<float>&   buffer, MidiBuffer& midiMessages) override
    {
        processBlockBypassedWrapped (buffer, midiMessages, floatEngine);
    };
    
    void processBlockBypassed (AudioBuffer<double>&  buffer, MidiBuffer& midiMessages) override
    {
        processBlockBypassedWrapped (buffer, midiMessages, doubleEngine);
    };
    
    
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; };
    
    const juce::String getName() const override { return JucePlugin_Name; };
    
    bool  acceptsMidi()  const override { return true;  };
    bool  producesMidi() const override { return true;  };
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
    
    void updateChunkChopping(const bool shouldChop) { choppingInput = shouldChop; };
    void updatePitchDetectionSettings(const float newMinHz, const float newMaxHz, const float newTolerance);
    
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
    
    
    // misc utility functions -----------------------------------------------------------------------------------------------------------------------
    
    Array<int> returnActivePitches() const;
    
    void killAllMidi();
    
    AudioProcessorValueTreeState tree;
    
    enum ModulatorInputSource { left, right, mixToMono }; // determines how the plugin will take input from the stereo buffer fed to it from the host
    
    void changeModulatorInputSource(ModulatorInputSource newSource) noexcept { modulatorInput = newSource; };
    
    bool canAddBus(bool isInput) const override;
    
    bool shouldWarnUserToEnableSidechain() const;
    
    bool supportsDoublePrecisionProcessing() const override { return true; };
    
    void updateTrackProperties (const TrackProperties& properties) override; // informs the plugin about the properties of the DAW mixer track it's loaded on
    
    void updateNumVoices(const int newNumVoices);
    
    //==============================================================================
    
    float getDryPanningMult(const int index) const { return dryvoxpanningmults[index]; };
    
    float getInputGainMult()  const noexcept { return inputGainMultiplier.load(); };
    float getOutputGainMult() const noexcept { return outputGainMultiplier.load(); };
    
    ModulatorInputSource getModulatorSource() const noexcept { return modulatorInput; };
    
    bool isLimiterOn() const noexcept { return limiterIsOn.load(); };

    
private:
    
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
    
    void updateAllParameters();
    
    float dryvoxpanningmults[2]; // stores gain multiplier values, which when applied to the input signal, achieve the desired dry vox panning
    std::atomic_long inputGainMultiplier, outputGainMultiplier;
    
    ModulatorInputSource modulatorInput; // determines how the modulator signal is parsed from the [usually] stereo buffer passed into processBlock
    
    std::atomic_bool limiterIsOn;
    
    // variables to store previous parameter values, to avoid unnecessary update operations:
    int prevDryPan;
    float prevideb, prevodeb;
    
    bool choppingInput;
    
    PluginHostType host;
    
    AudioProcessorValueTreeState::ParameterLayout createParameters() const;
    
    AudioProcessor::BusesProperties makeBusProperties() const;
    
    bool wasBypassedLastCallback; // used to activate a fade out instead of an instant kill when the bypass is activated
    
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
    AudioParameterFloat* inputGain          = nullptr;
    AudioParameterFloat* outputGain         = nullptr;
    AudioParameterBool*  limiterToggle      = nullptr;
    AudioParameterFloat* limiterThresh      = nullptr;
    AudioParameterInt*   limiterRelease     = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};





