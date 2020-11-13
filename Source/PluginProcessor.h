#pragma once

#include <JuceHeader.h>

#include "HarmonyVoice.h"
#include "MidiProcessor.h"

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
	
	double voxCurrentPitch = 440.;  // a variable to store the modulator signal's current input pitch [as frequency!]
	
	AudioProcessorValueTreeState tree;
	
	MidiProcessor midiProcessor;
	bool midiLatch;
	
	OwnedArray<HarmonyVoice> harmEngine;  // this array houses all the instances of the harmony engine that are running
	
	// float array to send to PluginEditor for waveform visualization
	Array<float> history; // a running averaged wave, for GUI purposes
	int historyLength; // length of history array / should be roughly equal to length of waveform GUI in pixels
	Array<float> getHistory() { return history; };
	
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
	
	AudioProcessorValueTreeState::ParameterLayout createParameters();
	void grabCurrentParameterValues();
	
	AudioBuffer<float> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};



