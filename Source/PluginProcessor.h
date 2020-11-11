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
	bool midiLatch = false;
	
	OwnedArray<HarmonyVoice> harmEngine;  // this array houses all the instances of the harmony engine that are running
	
	// float array to send to PluginEditor for waveform visualization
	Array<float> history; // a running averaged wave, for GUI purposes
	int historyLength; // length of history array ( number of frequency bins represented ) // should be roughly equal to length of waveform GUI in pixels
	
//==============================================================================
	
private:
	
	double lastSampleRate;
	int lastBlockSize;
	double prevLastSampleRate = 0.0;
	int prevLastBlockSize = 0;
	
	float* adsrAttackListener = (float*)(tree.getRawParameterValue("adsrAttack"));
	float* adsrDecayListener = (float*)(tree.getRawParameterValue("adsrDecay"));
	float* adsrSustainListener = (float*)(tree.getRawParameterValue("adsrSustain"));
	float* adsrReleaseListener = (float*)(tree.getRawParameterValue("adsrRelease"));
	float prevAttack = 0.0f;
	float prevDecay = 0.0f;
	float prevSustain = 0.0f;
	float prevRelease = 0.0f;

	float* stereoWidthListener = (float*)(tree.getRawParameterValue("stereoWidth"));
	float previousStereoWidth = 0.0f;

	float* midiVelocitySensListener = (float*)(tree.getRawParameterValue("midiVelocitySensitivity"));
	float prevVelocitySens = 0.0f;
	
	float* pitchBendUpListener = (float*)(tree.getRawParameterValue("PitchBendUpRange"));
	float* pitchBendDownListener = (float*)(tree.getRawParameterValue("PitchBendDownRange"));
	
	float* inputGainListener = (float*)(tree.getRawParameterValue("inputGain"));
	float inputGainMultiplier;
	float* outputGainListener = (float*)(tree.getRawParameterValue("outputGain"));
	float outputGainMultiplier;
	
	float* midiLatchListener = (float*)(tree.getRawParameterValue("midiLatch"));
	bool previousLatch = false;
	
	float* voiceStealingListener = (float*)(tree.getRawParameterValue("voiceStealing"));
	
	AudioProcessorValueTreeState::ParameterLayout createParameters();
	void grabCurrentParameterValues();
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};



