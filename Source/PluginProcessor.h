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
	
	void turnOnHarmonyVoice(int voiceNumber, int pitch, int velocity);
	
	double voxCurrentPitch = 440.;  // a variable to store the modulator signal's current input pitch [as frequency!]
	
	AudioProcessorValueTreeState tree;
	
	PolyphonyVoiceManager polyphonyManager;
	
//==============================================================================
	
private:
	
	static const int numVoices = 12; 
	HarmonyVoice *harmEngine[numVoices];
	
	double lastSampleRate;
	int lastBlockSize;
	
	// pointers for ADSR parameter values
	float* adsrAttackListener = (float*)(tree.getRawParameterValue("adsrAttack"));
	float* adsrDecayListener = (float*)(tree.getRawParameterValue("adsrDecay"));
	float* adsrSustainListener = (float*)(tree.getRawParameterValue("adsrSustain"));
	float* adsrReleaseListener = (float*)(tree.getRawParameterValue("adsrRelease"));
	
	MidiProcessor midiProcessor;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};
