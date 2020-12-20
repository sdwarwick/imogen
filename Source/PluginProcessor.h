#pragma once

#include <JuceHeader.h>

#include "GlobalDefinitions.h"
#include "Harmonizer.h"
#include "InputAnalysis.h"


//==============================================================================
/**
*/
class ImogenAudioProcessor    : public juce::AudioProcessor

{
	
public:
	
    ImogenAudioProcessor();
    ~ImogenAudioProcessor() override;
	
	AudioProcessor::BusesProperties makeBusProperties();
	
	Array<int> returnActivePitches() const { return harmonizer.reportActiveNotes(); }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
	
	void killAllMidi() { harmonizer.allNotesOff(false); }

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
	
	AudioProcessorValueTreeState tree;
	
	void updateNumVoices(const int newNumVoices);
	
//==============================================================================
	
private:
	
	void processBlockPrivate(AudioBuffer<float>&, const int numSamples, const int inputChannel, MidiBuffer& inputMidi);
	
	Harmonizer harmonizer;
	
	double lastSampleRate;
	int lastBlockSize;

	bool adsrIsOn;
	float previousStereoWidth;
	float inputGainMultiplier;
	float outputGainMultiplier;
	
	int prevQuickKillMs;
	int prevConcertPitch;

	int previousmidipan;
	int dryvoxpanningmults[2]; // should have NUMBER_OF_CHANNELS elements.
	
	float prevideb;
	float prevodeb;
	
	dsp::ProcessSpec dspSpec;

	dsp::Limiter<float> limiter;
	bool limiterIsOn;
	
	dsp::DryWetMixer<float> dryWet;
	
	AudioProcessorValueTreeState::ParameterLayout createParameters();
	
	AudioBuffer<float> wetBuffer; // this buffer is where the 12 harmony voices' output gets added together
	
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
		const std::atomic<float>& inputGainListener;
		const std::atomic<float>& outputGainListener;
		const std::atomic<float>& voiceStealingListener;
		const std::atomic<float>& inputChannelListener;
		const std::atomic<float>& dryVoxPanListener;
		const std::atomic<float>& masterDryWetListener;
		const std::atomic<float>& limiterThreshListener;
		const std::atomic<float>& limiterReleaseListener;
		const std::atomic<float>& limiterToggleListener;
		const std::atomic<float>& quickKillMsListener;
		const std::atomic<float>& concertPitchListener;
	
	void updateAdsr();
	void updateIOgains();
	void updateLimiter();
	void updateStereoWidth();
	void updateQuickKillMs();
	void updateDryVoxPan();
	

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessor)
};



