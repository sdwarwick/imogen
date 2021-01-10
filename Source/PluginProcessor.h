#pragma once

#include <JuceHeader.h>

#include "GlobalDefinitions.h"
#include "Harmonizer.h"
#include "Freezer.h"


class ImogenAudioProcessorEditor; // forward declaration...

class ImogenAudioProcessor;



template<typename SampleType>
class ImogenEngine
{
public:
    ImogenEngine(ImogenAudioProcessor& p);
    
    ~ImogenEngine();
    
    void process (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages,
                  const bool applyFadeIn, const bool applyFadeOut);
    
    void processBypassed (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output);
    
    void initialize (const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices);
    
    void prepare (double sampleRate, int samplesPerBlock);
    
    void reset();
    
    void releaseResources();
    
    void updateNumVoices(const int newNumVoices); // updates the # of cuncurrently running instances of the pitch shifting algorithm
    
    void returnActivePitches(Array<int>& outputArray) const { return harmonizer.reportActiveNotes(outputArray); };
    
    void updateSamplerate (const int newSamplerate);
    void updateDryWet     (const float newWetMixProportion);
    void updateDryVoxPan  (const int newMidiPan);
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
    void updateInputGain  (const float newInGain);
    void updateOutputGain (const float newOutGain);
    void updateDryGain (const float newDryGain);
    void updateWetGain (const float newWetGain);
    
    void clearBuffers();
    
    bool hasBeenReleased()    const noexcept { return resourcesReleased; };
    bool hasBeenInitialized() const noexcept { return initialized; };
    
    static constexpr int internalBlocksize = 32; // the size of the processing blocks, in samples, that the algorithm will be processing at a time. This is for the pitch detection and the ESOLA algorithm.
    
    
private:
    
    void processWrapped (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output,
                         MidiBuffer& midiMessages,
                         const bool applyFadeIn, const bool applyFadeOut);
    
    void processBypassedWrapped (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output);
    
    
    void renderNoChopping (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages);
    
    void renderWithChopping (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages);
    
    void renderBlock (const AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output,
                      MidiBuffer& midiMessages,
                      const bool applyFadeIn, const bool applyFadeOut);
    
    ImogenAudioProcessor& processor;
    
    Harmonizer<SampleType> harmonizer;
    
    AudioBuffer<SampleType> inputCollectionBuffer; // this buffer is used to collect input samples if we recieve too few to do processing, so that the blocksizes fed to renderBlock can be regulated. THIS BUFFER SHOULD BE SIZE internalBlocksize * 2 !!
    
    AudioBuffer<SampleType> inputInterimBuffer; // buffer used to actually pass grains of regulated size into the renderBlock function. THiS BUFFER SHOULD BE SIZE internalBlocksize * 2 !!
    
    int numStoredInputSamples; // the # of overflow input samples left from the last frame too small to be processed on its own.
    // INIT TO 0!!!
    
    AudioBuffer<SampleType> outputCollectionBuffer;
    
    AudioBuffer<SampleType> outputInterimBuffer;
    
    int numStoredOutputSamples;
    
    
    AudioBuffer<SampleType> inBuffer;  // this buffer is used to store the mono input signal so that input gain can be applied
    AudioBuffer<SampleType> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
    AudioBuffer<SampleType> dryBuffer; // this buffer is used for panning & delaying the dry signal
    
    dsp::ProcessSpec dspSpec;
    dsp::Limiter <SampleType> limiter;
    dsp::DryWetMixer<SampleType> dryWetMixer;
    dsp::DelayLine<SampleType> bypassDelay; // a delay line used for latency compensation when the processing is bypassed
    
    bool resourcesReleased;
    bool initialized;
    
    float dryPanningMults[2];
    float prevDryPanningMults[2];
    
    float dryGain, prevDryGain;
    float wetGain, prevWetGain;
    
    float inputGain, prevInputGain;
    float outputGain, prevOutputGain;
    
    MidiBuffer midiChoppingBuffer;
    
    void copyRangeOfMidiBuffer (const MidiBuffer& inputBuffer, MidiBuffer& outputBuffer,
                                const int startSampleOfInput,
                                const int startSampleOfOutput,
                                const int numSamples);
    
    void usedOutputSamples (const int numSamplesUsed);
    void usedInputSamples  (const int numSamplesUsed);
    
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
    
    
    void processBlock (juce::AudioBuffer<float>&  buffer, juce::MidiBuffer& midiMessages) override;
    
    void processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override;
    
   
    void processBlockBypassed (AudioBuffer<float>&   buffer, MidiBuffer& midiMessages) override;
    
    void processBlockBypassed (AudioBuffer<double>&  buffer, MidiBuffer& midiMessages) override;
    
    
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
    void updateDryWetGains();
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
    
    void returnActivePitches(Array<int>& outputArray) const;
    
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
    
    ModulatorInputSource getModulatorSource() const noexcept { return modulatorInput; };
    
    bool isLimiterOn() const noexcept { return limiterIsOn.load(); };

    bool shouldChopInput() const noexcept { return choppingInput; };
    
private:
    
    template <typename SampleType>
    void processBlockWrapped (AudioBuffer<SampleType>& buffer, MidiBuffer& midiMessages, ImogenEngine<SampleType>& engine);
    
    template <typename SampleType>
    void processBlockBypassedWrapped (AudioBuffer<SampleType>& buffer, MidiBuffer& midiMessages, ImogenEngine<SampleType>& engine);
    
    template <typename SampleType1, typename SampleType2>
    void prepareToPlayWrapped (const double sampleRate, const int samplesPerBlock,
                               ImogenEngine<SampleType1>& activeEngine,
                               ImogenEngine<SampleType2>& idleEngine);
    
    template<typename SampleType>
    void updateAllParameters(ImogenEngine<SampleType>& activeEngine);
    
    ImogenEngine<float>  floatEngine;
    ImogenEngine<double> doubleEngine;
    
    ModulatorInputSource modulatorInput; // determines how the modulator signal is parsed from the [usually] stereo buffer passed into processBlock
    
    std::atomic_bool limiterIsOn;
    
    // variables to store previous parameter values, to avoid unnecessary update operations:
    int prevDryPan;
    float prevideb, prevodeb, prevddeb, prevwdeb;
    
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
    AudioParameterFloat* dryGain            = nullptr;
    AudioParameterFloat* wetGain            = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};





