#pragma once

#include <JuceHeader.h>

#include "GlobalDefinitions.h"
#include "HarmonyVoice.h"
#include "MidiProcessor.h"
#include "EpochExtractor.h"
#include "Yin.h"

#define TIMER_RATE_MS 150


//==============================================================================
/**
*/
class ImogenAudioProcessor    : public juce::AudioProcessor,
								public Timer
{
	
public:
	
    ImogenAudioProcessor();
    ~ImogenAudioProcessor() override;
	
	void timerCallback() override;
	
	Array<int> returnActivePitches();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
	void processBlockPrivate(AudioBuffer<float>&, const int numSamples, const int inputChannel);
	
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
	
	double voxCurrentPitch;  // a variable to store the modulator signal's current input pitch [as frequency!]
	
	AudioProcessorValueTreeState tree;
	
	OwnedArray<HarmonyVoice> harmEngine;  // this array houses all the instances of the harmony engine that are running
	
	MidiProcessor midiProcessor;
	bool midiLatch;
	
	Array<int> epochLocations;
	
//==============================================================================
	
private:
	
	double lastSampleRate;
	int lastBlockSize;
	double prevLastSampleRate;
	int prevLastBlockSize;
	
	bool frameIsPitched;
	
	// variables for tracking GUI-changeable parameters
		float* adsrAttackListener = (float*)(tree.getRawParameterValue("adsrAttack"));
		float* adsrDecayListener = (float*)(tree.getRawParameterValue("adsrDecay"));
		float* adsrSustainListener = (float*)(tree.getRawParameterValue("adsrSustain"));
		float* adsrReleaseListener = (float*)(tree.getRawParameterValue("adsrRelease"));
		float* adsrOnOffListener = (float*)(tree.getRawParameterValue("adsrOnOff"));
		bool adsrIsOn;
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
	int analysisLimit;

	Yin pitchTracker;
	EpochExtractor epochs;
	
	void writeToDryBuffer (AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples);
	int* dryVoxPanListener = (int*)(tree.getRawParameterValue("dryPan"));
	int previousmidipan;
	int dryvoxpanningmults[2];
	
	int* masterDryWetListener = (int*)(tree.getRawParameterValue("masterdryWet"));
	int previousMasterDryWet;
	float dryMultiplier;
	float wetMultiplier;
	
	float prevideb;
	float prevodeb;
	
	dsp::Limiter<float> limiter;
	float* limiterThreshListener = (float*)(tree.getRawParameterValue("limiterThresh"));
	int* limiterReleaseListener = (int*)(tree.getRawParameterValue("limiterRelease"));
	
	AudioProcessorValueTreeState::ParameterLayout createParameters();
	void grabCurrentParameterValues();
	
	AudioBuffer<float> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
	AudioBuffer<float> dryBuffer; // this buffer holds the original input signal, delayed for latency, so it can be mixed back together with the wet signal for the dry/wet effect
	int dryBufferWritePosition;
	int dryBufferReadPosition;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};



