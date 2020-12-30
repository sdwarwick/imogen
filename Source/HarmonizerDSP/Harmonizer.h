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
#include "HarmonizerUtilities.h"
#include "PanningManager.h"


class Harmonizer; // forward declaration...



/*
 HarmonizerVoice : represents a "voice", or instance of the DSP algorithm, that the Harmonizer can use to generate sound. A voice plays a single note/sound at a time; the Harmonizer holds an array of voices so that it can play polyphonically.
 */

class HarmonizerVoice
{
public:
	HarmonizerVoice(Harmonizer* h);
	
	~HarmonizerVoice();
	
	void renderNextBlock(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices);
	
	int getCurrentlyPlayingNote() const noexcept { return currentlyPlayingNote; }
	
	bool isVoiceActive() const noexcept { return currentlyPlayingNote >= 0; }
	
	bool isPlayingButReleased() const noexcept; // returns true if a voice is sounding, but its key has been released
	
	// Returns true if this voice started playing its current note before the other voice did. */
	bool wasStartedBefore (const HarmonizerVoice& other) const noexcept { return noteOnTime < other.noteOnTime; }
	
	// Returns true if the key that triggered this voice is still held down. Note that the voice may still be playing after the key was released (e.g because the sostenuto pedal is down).
	bool isKeyDown() const noexcept { return keyIsDown; }
	void setKeyDown(bool isNowDown) noexcept { keyIsDown = isNowDown; }
	
	void setPan(const int newPan);
	
	void startNote(const int midiPitch, const float velocity);
	void stopNote(const float velocity, const bool allowTailOff);
	void aftertouchChanged(const int newAftertouchValue);
	
	
private:
	
	friend class Harmonizer;
	
	Harmonizer* parent; // this is a pointer to the Harmonizer object that controls this HarmonizerVoice
	
	ADSR adsr;
	ADSR quickRelease; // used to quickly fade out signal when stopNote() is called with the allowTailOff argument set to false, instead of jumping signal to 0
	bool isFading;
	bool noteTurnedOff;
	int currentlyPlayingNote;
	float currentOutputFreq;
	float currentVelocityMultiplier;
	float lastRecievedVelocity;
	uint32 noteOnTime;
	bool keyIsDown;
	int currentMidipan;
	float panningMults[2];
	
	AudioBuffer<float> tempBuffer;
	
	void clearCurrentNote() noexcept { currentlyPlayingNote = -1; }
	
	void esola(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices, const float shiftingRatio);
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonizerVoice)
};



class Harmonizer
{
public:
	Harmonizer();
	
	~Harmonizer();
	
	void renderVoices (AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, AudioBuffer<float>& outputBuffer, Array<int>& epochIndices);
	
	int getNumActiveVoices();
	
	void setCurrentInputFreq(const float inputFreqHz) noexcept { currentInputFreq = inputFreqHz; };
	
	void handleMidiEvent(const MidiMessage& m);
	void updateMidiVelocitySensitivity(const int newSensitivity);
	
	void resetNoteOnCounter() noexcept { lastNoteOnCounter = 0; }
	
	void setCurrentPlaybackSampleRate(const double newRate);
	double getSamplerate() const noexcept { return sampleRate; }
	
	void setConcertPitchHz(const int newConcertPitchhz);
	
	void updateStereoWidth(const int newWidth);
	void updateLowestPannedNote(const int newPitchThresh) noexcept;
	int getCurrentLowestPannedNote() const noexcept { return lowestPannedNote; };
	
	void setNoteStealingEnabled (bool shouldSteal) { shouldStealNotes = shouldSteal; }
	bool isNoteStealingEnabled() const noexcept { return shouldStealNotes; }
	
	Array<int> reportActiveNotes() const; // returns an array of the currently active pitches, or a single -1 if no notes are active
	Array<int> reportActivesNoReleased() const; // the same, but excludes notes that are still ringing but whose key has been released
	
	// turn off all notes
	void allNotesOff(const bool allowTailOff);
	
	void setMidiLatch(const bool shouldBeOn, const bool allowTailOff);
	bool isLatched() const noexcept { return latchIsOn; };
	
	void updateADSRsettings(const float attack, const float decay, const float sustain, const float release);
	void setADSRonOff(const bool shouldBeOn) noexcept{ adsrIsOn = shouldBeOn; };
	bool isADSRon() const noexcept { return adsrIsOn; };
	void updateQuickReleaseMs(const int newMs);
	ADSR::Parameters getCurrentAdsrParams() const noexcept { return adsrParams; }
	
	void updatePitchbendSettings(const int rangeUp, const int rangeDown);
	
	// Adds a new voice to the harmonizer. The object passed in will be managed by the synthesiser, which will delete it later on when no longer needed. The caller should not retain a pointer to the voice.
	HarmonizerVoice* addVoice(HarmonizerVoice* newVoice);
	
	// removes a specified # of voices, attempting to remove inactive voices first
	void removeNumVoices(const int voicesToRemove);
	
	// deletes all harmony voices
	void deleteAllVoices();
	
	int getNumVoices() const noexcept { return voices.size(); }
	
	void setPedalPitch(const bool isOn);
	bool isPedalPitchOn() const noexcept { return pedalPitchIsOn; };
	void setPedalPitchUpperThresh(const int newThresh);
	int getCurrentPedalPitchUpperThresh() const noexcept { return pedalPitchUpperThresh; };
	void setPedalPitchInterval(const int newInterval);
	int getCurrentPedalPitchInterval() const noexcept { return pedalPitchInterval; };
	
	void setDescant(const bool isOn);
	bool isDescantOn() const noexcept { return descantIsOn; };
	void setDescantLowerThresh(const int newThresh);
	int getCurrentDescantLowerThresh() const noexcept { return descantLowerThresh; };
	void setDescantInterval(const int newInterval);
	int getCurrentDescantInterval() const noexcept { return descantInterval; };
	
	
protected:
	CriticalSection lock;
	OwnedArray<HarmonizerVoice> voices;
	int lastPitchWheelValue;
	
	PanningManager panner;
	
	// MIDI
	void noteOn(const int midiPitch, const float velocity);
	void noteOff (const int midiNoteNumber, const float velocity, const bool allowTailOff, const bool partOfList);
	void handlePitchWheel(const int wheelValue);
	void handleAftertouch(const int midiNoteNumber, const int aftertouchValue);
	void handleChannelPressure(const int channelPressureValue);
	void handleController(const int controllerNumber, const int controllerValue);
	void handleSustainPedal(const bool isDown);
	void handleSostenutoPedal(const bool isDown);
	void handleSoftPedal(const bool isDown);
	
	void handleModWheel(const int wheelValue);
	void handleBreathController(const int controlValue);
	void handleFootController(const int controlValue);
	void handlePortamentoTime(const int controlValue);
	void handleMainVolume(const int controlValue);
	void handleBalance(const int controlValue);
	void handleLegato(const bool isOn);
	
	// voice allocation
	HarmonizerVoice* findFreeVoice (const int midiNoteNumber, const bool stealIfNoneAvailable) const;
	HarmonizerVoice* findVoiceToSteal (const int midiNoteNumber) const;

	
private:
	
	friend class HarmonizerVoice;
	
	PitchConverter pitchConverter;
	PitchBendHelper bendTracker;
	VelocityHelper velocityConverter;
	
	bool latchIsOn;
	MidiLatchManager latchManager;
	
	ADSR::Parameters adsrParams;
	ADSR::Parameters quickReleaseParams;
	bool adsrIsOn;
	
	float currentInputFreq;
	
	double sampleRate;
	bool shouldStealNotes;
	uint32 lastNoteOnCounter;
	int lowestPannedNote;
	
	bool sustainPedalDown, sostenutoPedalDown;
	
	mutable Array<int> currentlyActiveNotes;
	mutable Array<int> currentlyActiveNoReleased;
	
	// Starts a specified voice playing a specified note
	void startVoice (HarmonizerVoice* voice, const int midiPitch, const float velocity);
	
	// Stops a given voice.
	void stopVoice (HarmonizerVoice* voice, const float velocity, const bool allowTailOff);
	
	Array<int> unLatched;
	void turnOffList(Array<int>& toTurnOff, const float velocity, const bool allowTailOff);
	
	//Array<int> activePitches;
	
	// this function is called any time the collection of pitches is changed (ie, with regular keyboard input, on each note on/off, or for chord input, once after each chord is triggered). Used for things like pedal pitch, etc
	void pitchCollectionChanged();
	
	void applyPedalPitch();
	bool pedalPitchIsOn;
	int lastPedalPitch;
	int pedalPitchUpperThresh;
	int pedalPitchInterval;
	
	void applyDescant();
	bool descantIsOn;
	int lastDescantPitch;
	int descantLowerThresh;
	int descantInterval;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};



