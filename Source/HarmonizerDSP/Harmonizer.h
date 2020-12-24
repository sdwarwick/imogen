/*
  ==============================================================================

    Harmonizer.h
    Created: 13 Dec 2020 7:53:30pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"
#include "DspUtils.h"
#include "PanningManager.h"


/*
 HarmonizerVoice : represents a "voice", or instance of the DSP algorithm, that the Harmonizer can use to generate sound. A voice plays a single note/sound at a time; the Harmonizer holds an array of voices so that it can play polyphonically.
 */

class HarmonizerVoice
{
public:
	HarmonizerVoice();
	
	~HarmonizerVoice();
	
	// renders the next block of audio data for this voice.
	// The output audio data will be added to the current contents of the buffer provided.
	// Only the region of the buffer between startSample and (startSample + numSamples) should be altered by this method.
	// If the voice is currently silent, it should just return without doing anything.
	// The size of the blocks that are rendered can change each time it is called, and may involve rendering as little as 1 sample at a time.
	// In between rendering callbacks, the voice's methods will be called to tell it about note and controller events.
	void renderNextBlock(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices);
	
	void updateInputFreq(const float newFreq) noexcept { currentInputFreq = newFreq; }
	
	int getCurrentlyPlayingNote() const noexcept { return currentlyPlayingNote; }
	
	bool isVoiceActive() const noexcept { return currentlyPlayingNote >= 0; }
	
	// returns true if a voice is sounding, but its key has been released
	bool isPlayingButReleased() const noexcept { return isVoiceActive() && ! (keyIsDown || sostenutoPedalDown || sustainPedalDown); }
	
	// Returns true if this voice started playing its current note before the other voice did. */
	bool wasStartedBefore (const HarmonizerVoice& other) const noexcept { return noteOnTime < other.noteOnTime; }
	
	void setCurrentPlaybackSamplerate(const double newRate);
	double getSamplerate() const noexcept { return currentSampleRate; }
	
	void setConcertPitch(const int newConcertPitch);
	
	bool isSustainPedalDown() const noexcept { return sustainPedalDown; }
	void setSustainPedalDown(bool isNowDown) noexcept { sustainPedalDown = isNowDown; }
	
	bool isSostenutoPedalDown() const noexcept { return sostenutoPedalDown; }
	void setSostenutoPedalDown(bool isNowDown) noexcept { sostenutoPedalDown = isNowDown; }
	
	// Returns true if the key that triggered this voice is still held down. Note that the voice may still be playing after the key was released (e.g because the sostenuto pedal is down).
	bool isKeyDown() const noexcept { return keyIsDown; }
	void setKeyDown(bool isNowDown) noexcept { keyIsDown = isNowDown; }
	
	void setMidiVelocitySensitivity(const float newsensitity);
	
	void setPan(const int newPan);
	int getPan() const noexcept { return currentMidipan; }
	
	void startNote(const int midiPitch, const float velocity, const int currentPitchWheelPosition);
	void changeNote(const int midiPitch, const float velocity, const int currentPitchWheelPosition);
	void stopNote(const float velocity, const bool allowTailOff);
	void pitchWheelMoved(const int newPitchWheelValue);
	void aftertouchChanged(const int newAftertouchValue);
	void channelPressureChanged(const int newChannelPressureValue);
	void controllerMoved(const int controllerNumber, const int newControllerValue);
	
	
	void updateAdsrSettings(ADSR::Parameters& newParams) noexcept { adsr.setParameters(newParams); }
	void updateQuickReleaseSettings(ADSR::Parameters& newParams) noexcept { quickRelease.setParameters(newParams); }
	void setAdsrOnOff(const bool isOn) noexcept { adsrIsOn = isOn; }
	
	void updatePitchbendSettings(const int rangeUp, const int rangeDown);
	
	
protected:
	//Resets the state of this voice after a note has finished playing. The subclass must call this when it finishes playing a note and becomes available to play new ones. It must either call it in the stopNote() method, or if the voice is tailing off, then it should call it later during the renderNextBlock method, as soon as it finishes its tail-off. 
	void clearCurrentNote() noexcept { currentlyPlayingNote = -1; }

	
private:
	
	friend class Harmonizer;
	
	ADSR adsr;
	bool adsrIsOn;
	ADSR quickRelease; // used to quickly fade out signal when stopNote() is called with the allowTailOff argument set to false, instead of jumping signal to 0
	bool isFading;
	
	bool noteTurnedOff;
	
	MidiPitchConverter converter;
	
	int currentlyPlayingNote;
	float currentOutputFreq;
	float currentVelocityMultiplier;
	int pitchbendRangeUp, pitchbendRangeDown;
	int lastRecievedPitchbend, lastRecievedVelocity;
	double currentSampleRate;
	uint32 noteOnTime;
	bool keyIsDown, sustainPedalDown, sostenutoPedalDown;
	float midiVelocitySensitivity;
	int currentMidipan;
	float panningMults[2];
	float currentInputFreq;
	
	AudioBuffer<float> tempBuffer;
	
	// calculates the actual target frequency, based on the last recieved midiPitch, the last recieved pitch wheel value, and the current pitch bend settings
	float getOutputFreqFromMidinoteAndPitchbend(const int lastRecievedNote, const int pitchBend);
	
	
	// calculates a [0.0, 1.0] gain value to be applied to the output signal that corresponds to the latest recieved midi Velocity & the selected midi velocity sensitivity
	float calcVelocityMultiplier(const float inputVelocity);
	
	void esola(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices, const float shiftingRatio);
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonizerVoice)
};



class Harmonizer
{
public:
	Harmonizer();
	
	~Harmonizer();
	
	// renders the voices for the given range
	void renderVoices (AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices);
	
	void setCurrentInputFreq(const float inputFreqHz) noexcept;
	
	void handleMidiEvent(const MidiMessage& m);
	
	
	void resetNoteOnCounter() noexcept { lastNoteOnCounter = 0; }
	
	
	void updateMidiVelocitySensitivity(const int newSensitivity);
	
	void setCurrentPlaybackSampleRate(const double newRate);
	double getSamplerate() const noexcept { return sampleRate; }
	
	void setConcertPitchHz(const int newConcertPitchhz);
	
	void updateStereoWidth(const int newWidth);
	void updateLowestPannedNote(const int newPitchThresh) noexcept { lowestPannedNote = newPitchThresh; }
	
	void setNoteStealingEnabled (bool shouldSteal) { shouldStealNotes = shouldSteal; }
	bool isNoteStealingEnabled() const noexcept { return shouldStealNotes; }
	
	// returns an array of the currently active pitches, or a single -1 if no notes are active
	Array<int> reportActiveNotes() const;
	
	// turn off all notes
	void allNotesOff(const bool allowTailOff);
	
	void updateADSRsettings(const float attack, const float decay, const float sustain, const float release);
	void setADSRonOff(const bool shouldBeOn);
	void updateQuickReleaseMs(const int newMs);
	ADSR::Parameters getCurrentAdsrParams() const noexcept { return adsrParams; }
	
	void updatePitchbendSettings(const int rangeUp, const int rangeDown);
	
	// Adds a new voice to the harmonizer. The object passed in will be managed by the synthesiser, which will delete it later on when no longer needed. The caller should not retain a pointer to the voice.
	HarmonizerVoice* addVoice(HarmonizerVoice* newVoice);
	
	// deletes one of the voices
	void removeVoice(const int index);
	
	// removes a specified # of voices, attempting to remove inactive voices first
	void removeNumVoices(const int voicesToRemove);
	
	// returns a pointer to one of the voices that has been added
	HarmonizerVoice* getVoice(const int index) const;
	
	// deletes all harmony voices
	void deleteAllVoices();
	
	int getNumVoices() const noexcept { return voices.size(); }
	
	
	
protected:
	CriticalSection lock;
	OwnedArray<HarmonizerVoice> voices;
	int lastPitchWheelValue;
	
	PanningManager panner;
	
	// MIDI
	void noteOn(const int midiPitch, const float velocity);
	void noteOff (const int midiNoteNumber, const float velocity, const bool allowTailOff);
	void handlePitchWheel(const int wheelValue);
	void handleAftertouch(const int midiNoteNumber, const int aftertouchValue);
	void handleChannelPressure(const int channelPressureValue);
	void handleController(const int controllerNumber, const int controllerValue);
	void handleSustainPedal(const bool isDown);
	void handleSostenutoPedal(const bool isDown);
	void handleSoftPedal(const bool isDown);
	
	// voice allocation
	HarmonizerVoice* findFreeVoice (const int midiNoteNumber, const bool stealIfNoneAvailable) const;
	HarmonizerVoice* findVoiceToSteal (const int midiNoteNumber) const;

	
	
private:
	
	ADSR::Parameters adsrParams;
	ADSR::Parameters quickReleaseParams;
	
	float currentInputFreq;
	
	double sampleRate;
	bool shouldStealNotes;
	uint32 lastNoteOnCounter;
	int lowestPannedNote;
	
	mutable Array<int> currentlyActiveNotes;
	
	// Starts a specified voice playing a specified note
	void startVoice (HarmonizerVoice* voice, const int midiPitch, const float velocity);
	
	// Stops a given voice.
	void stopVoice (HarmonizerVoice* voice, const float velocity, const bool allowTailOff);
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};



