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
class ImogenAudioProcessor    : public juce::AudioProcessor

{
	
public:
	
    ImogenAudioProcessor();
    ~ImogenAudioProcessor() override;
	
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

		bool adsrIsOn;
		float prevAttack;
		float prevDecay;
		float prevSustain;
		float prevRelease;
		float previousStereoWidth;
		int lowestPannedNote;
		float prevVelocitySens;
		float prevPitchBendUp;
		float prevPitchBendDown;
		bool pedalPitchToggle;
		int pedalPitchThresh;
		float inputGainMultiplier;
		float outputGainMultiplier;
		bool latchIsOn;
		bool previousLatch;
		bool stealingIsOn;
	
	void analyzeInput (AudioBuffer<float>& input, const int inputChan, const int numSamples);
	
	int analysisShift;
	int analysisShiftHalved;
	int analysisLimit;

	Yin pitchTracker;
	EpochExtractor epochs;
	
	void writeToDryBuffer (AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples);
	
	int previousmidipan;
	int dryvoxpanningmults[2]; // should have NUMBER_OF_CHANNELS elements.
	
	int previousMasterDryWet;
	float dryMultiplier;
	float wetMultiplier;
	
	float prevideb;
	float prevodeb;
	
	dsp::Limiter<float> limiter;
	bool limiterIsOn;
	
	AudioProcessorValueTreeState::ParameterLayout createParameters();
	void grabCurrentParameterValues();
	
	AudioBuffer<float> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
	AudioBuffer<float> dryBuffer; // this buffer holds the original input signal, delayed for latency, so it can be mixed back together with the wet signal for the dry/wet effect
	int dryBufferWritePosition;
	int dryBufferReadPosition;
	
		const std::atomic<float>& adsrAttackListener;
		const std::atomic<float>& adsrDecayListener;
		const std::atomic<float>& adsrSustainListener;
		const std::atomic<float>& adsrReleaseListener;
		const std::atomic<float>& adsrOnOffListener;
		const std::atomic<float>& stereoWidthListener;
		const std::atomic<float>& lowestPanListener;
		const std::atomic<float>& midiVelocitySensListener;
		const std::atomic<float>& pitchBendUpListener;
		const std::atomic<float>& pitchBendDownListener;
		const std::atomic<float>& pedalPitchToggleListener;
		const std::atomic<float>& pedalPitchThreshListener;
		const std::atomic<float>& inputGainListener;
		const std::atomic<float>& outputGainListener;
		const std::atomic<float>& midiLatchListener;
		const std::atomic<float>& voiceStealingListener;
		const std::atomic<float>& inputChannelListener;
		const std::atomic<float>& dryVoxPanListener;
		const std::atomic<float>& masterDryWetListener;
		const std::atomic<float>& limiterThreshListener;
		const std::atomic<float>& limiterReleaseListener;
		const std::atomic<float>& limiterToggleListener;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};



