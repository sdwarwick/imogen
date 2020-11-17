#pragma once

#include <JuceHeader.h>

#include "HarmonyVoice.h"
#include "MidiProcessor.h"
#include "inputPitchTracker.h"

//==============================================================================
/**
*/
class ImogenAudioProcessor  : public juce::AudioProcessor
{
	
public:
	
    ImogenAudioProcessor();
    ~ImogenAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
	
	void killAllMidi();

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
	
	//==============================================================================
	
	static const int numVoices = 12;  // global setting for how many instances of the harmony engine will be running concurrently
	
	static const int numChannels = 2; // declares 2 output channels (stereo). could conceivably be reconstructed to work with more sophisticated spatialization techniques in the future
	
	double voxCurrentPitch = 440.;  // a variable to store the modulator signal's current input pitch [as frequency!]
	
	AudioProcessorValueTreeState tree;
	
	OwnedArray<HarmonyVoice> harmEngine;  // this array houses all the instances of the harmony engine that are running
	
	MidiProcessor midiProcessor;
	bool midiLatch;
	
//==============================================================================
	
private:
	
	double lastSampleRate;
	int lastBlockSize;
	double prevLastSampleRate = 0.0;
	int prevLastBlockSize = 0;
	
	// variables for tracking GUI-changeable parameters
		float* adsrAttackListener = (float*)(tree.getRawParameterValue("adsrAttack"));
		float* adsrDecayListener = (float*)(tree.getRawParameterValue("adsrDecay"));
		float* adsrSustainListener = (float*)(tree.getRawParameterValue("adsrSustain"));
		float* adsrReleaseListener = (float*)(tree.getRawParameterValue("adsrRelease"));
		float prevAttack;
		float prevDecay;
		float prevSustain;
		float prevRelease;
		float* stereoWidthListener = (float*)(tree.getRawParameterValue("stereoWidth"));
		float previousStereoWidth;
		float* midiVelocitySensListener = (float*)(tree.getRawParameterValue("midiVelocitySensitivity"));
		float prevVelocitySens;
		float* pitchBendUpListener = (float*)(tree.getRawParameterValue("PitchBendUpRange"));
		float prevPitchBendUp;
		float* pitchBendDownListener = (float*)(tree.getRawParameterValue("PitchBendDownRange"));
		float prevPitchBendDown;
		float* inputGainListener = (float*)(tree.getRawParameterValue("inputGain"));
		float inputGainMultiplier;
		float* outputGainListener = (float*)(tree.getRawParameterValue("outputGain"));
		float outputGainMultiplier;
		float* midiLatchListener = (float*)(tree.getRawParameterValue("midiLatch"));
		bool latchIsOn;
		bool previousLatch;
		float* voiceStealingListener = (float*)(tree.getRawParameterValue("voiceStealing"));
		bool stealingIsOn;
	
		int* inputChannelListener = (int*)(tree.getRawParameterValue("inputChan"));
	
	void analyzeInput (AudioBuffer<float>& input, const int inputChan, const int numSamples);
	int analysisShift;
	int analysisShiftHalved;
	PitchTracker pitchTracker;
	
	void writeToDryBuffer (const float* readingPointer, AudioBuffer<float>& dryBuffer, const int numSamples);
	int* dryVoxPanListener = (int*)(tree.getRawParameterValue("dryPan"));
	int previousmidipan;
	int dryvoxpanningmults[2];
	
	int* masterDryWetListener = (int*)(tree.getRawParameterValue("masterdryWet"));
	int previousMasterDryWet;
	float dryMultiplier;
	float wetMultiplier;
	
	float prevideb;
	float prevodeb;
	
	AudioProcessorValueTreeState::ParameterLayout createParameters();
	void grabCurrentParameterValues();
	
	AudioBuffer<float> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
	AudioBuffer<float> dryBuffer; // this buffer holds the original input signal, delayed for latency, so it can be mixed back together with the wet signal for the dry/wet effect
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};



