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
	void renderNextBlock(AudioBuffer<float> outputBuffer, const int startSample, const int numSamples);
	
	int getCurrentlyPlayingNote() const noexcept { return currentlyPlayingNote; }
	
	bool isVoiceActive() const { return currentlyPlayingNote >= 0; }
	
	// returns true if a voice is sounding, but its key has been released
	bool isPlayingButReleased() const noexcept
	{
		return isVoiceActive() && ! (keyIsDown || sostenutoPedalDown || sustainPedalDown);
	}
	
	// Returns true if this voice started playing its current note before the other voice did. */
	bool wasStartedBefore (const HarmonizerVoice& other) const noexcept { return noteOnTime < other.noteOnTime; }
	
	void setCurrentPlaybackSamplerate(double newRate) { currentSampleRate = newRate; }
	double getSamplerate() const noexcept { return currentSampleRate; }
	
	// returns true if the sustain pedal is currently active for this voice
	bool isSustainPedalDown() const noexcept { return sustainPedalDown; }
	void setSustainPedalDown(bool isNowDown) noexcept { sustainPedalDown = isNowDown; }
	
	// returns true if the sostenuto pedal is currently active for this voice
	bool isSostenutoPedalDown() const noexcept { return sostenutoPedalDown; }
	void setSostenutoPedalDown(bool isNowDown) noexcept { sostenutoPedalDown = isNowDown; }
	
	// Returns true if the key that triggered this voice is still held down. Note that the voice may still be playing after the key was released (e.g because the sostenuto pedal is down).
	bool isKeyDown() const noexcept { return keyIsDown; }
	void setKeyDown(bool isNowDown) noexcept { keyIsDown = isNowDown; }
	
	void setMidiVelocitySensitivity(const int newsensitity);
	
	void startNote(const int midiPitch, const float velocity, const int currentPitchWheelPosition);
	void stopNote(const float velocity, const bool allowTailOff);
	void pitchWheelMoved(const int newPitchWheelValue);
	void aftertouchChanged(const int newAftertouchValue);
	void channelPressureChanged(const int newChannelPressureValue);
	void controllerMoved(const int controllerNumber, const int newControllerValue);
	
	
	void updateAdsrSettings(const float attack, const float decay, const float sustain, const float release);
	void setAdsrOnOff(const bool isOn) { adsrIsOn = isOn; }
	
	void updatePitchbendSettings(const int rangeUp, const int rangeDown);
	
	
protected:
	//Resets the state of this voice after a note has finished playing. The subclass must call this when it finishes playing a note and becomes available to play new ones. It must either call it in the stopNote() method, or if the voice is tailing off, then it should call it later during the renderNextBlock method, as soon as it finishes its tail-off. It can also be called at any time during the render callback if the sound happens
	void clearCurrentNote() { currentlyPlayingNote = -1; }
	
	
private:
	
	friend class Harmonizer;
	
	ADSR adsr;
	ADSR::Parameters adsrParams;
	bool adsrIsOn;
	
	int currentlyPlayingNote;
	float currentOutputFreq;
	float currentVelocityMultiplier;
	int pitchbendRangeUp, pitchbendRangeDown;
	int lastRecievedPitchbend, lastRecievedVelocity;
	double currentSampleRate;
	uint32 noteOnTime;
	bool keyIsDown, sustainPedalDown, sostenutoPedalDown;
	int midiVelocitySensitivity;
	
	AudioBuffer<float> tempBuffer;
	
	// calculates the actual target frequency, based on the last recieved midiPitch, the last recieved pitch wheel value, and the current pitch bend settings
	float getOutputFreqFromMidinoteAndPitchbend(const int lastRecievedNote, const int pitchBend);
	
	
	// calculates a [0.0, 1.0] gain value to be applied to the output signal that corresponds to the latest recieved midi Velocity & the selected midi velocity sensitivity
	float calcVelocityMultiplier(const int inputVelocity);
	
	void esola(AudioBuffer<float>& outputBuffer, const int startSample, const int numSamples);
	
	JUCE_LEAK_DETECTOR(HarmonizerVoice);
};



class Harmonizer
{
public:
	Harmonizer();
	
	~Harmonizer();
	
	// Creates the next block of audio output.
	// This will process the next numSamples of data from all the voices, and add that output to the audio block supplied, starting from the offset specified. Note that the data will be added to the current contents of the buffer, so you should clear it before calling this method if necessary.
	// The midi events in the inputMidi buffer are parsed for note and controller events, and these are used to trigger the voices. Note that the startSample offset applies both to the audio output buffer and the midi input buffer, so any midi events with timestamps outside the specified region will be ignored.
	void renderNextBlock(AudioBuffer<float>& outputAudio, const MidiBuffer& inputMidi, int startSample, int numSamples);
	
	
	void updateMidiVelocitySensitivity(const int newSensitivity);
	
	void setCurrentPlaybackSampleRate(const double newRate);
	double getSamplerate() const noexcept { return sampleRate; }
	
	// Sets a minimum limit on the size to which audio sub-blocks will be divided when rendering. When rendering, the audio blocks that are passed into renderNextBlock() will be split up into smaller blocks that lie between all the incoming midi messages, and it is these smaller sub-blocks that are rendered with multiple calls to renderVoices(). Obviously in a pathological case where there are midi messages on every sample, then renderVoices() could be called once per sample and lead to poor performance, so this setting allows you to set a lower limit on the block size. The default setting is 32, which means that midi messages are accurate to about < 1ms accuracy, which is probably fine for most purposes, but you may want to increase or decrease this value for your synth. If shouldBeStrict is true, the audio sub-blocks will strictly never be smaller than numSamples. If shouldBeStrict is false (default), the first audio sub-block in the buffer is allowed to be smaller, to make sure that the first MIDI event in a buffer will always be sample-accurate (this can sometimes help to avoid quantisation or phasing issues).
	void setMinimumRenderingSubdivisionSize (const int numSamples, const bool shouldBeStrict = false) noexcept;
	
	// If set to true, then the synth will try to take over an existing voice if it runs out and needs to play another note.
	void setNoteStealingEnabled (bool shouldSteal) { shouldStealNotes = shouldSteal; }
	bool isNoteStealingEnabled() const noexcept { return shouldStealNotes; }
	
	// returns an array of the currently active pitches, or -1 if no notes are active
	Array<int> reportActiveNotes() const;
	
	// turn off all notes
	void allNotesOff(const bool allowTailOff);
	
	void updateADSRsettings(const float attack, const float decay, const float sustain, const float release);
	void setADSRonOff(const bool shouldBeOn);
	
	void updatePitchbendSettings(const int rangeUp, const int rangeDown);
	
	// Adds a new voice to the harmonizer. The object passed in will be managed by the synthesiser, which will delete it later on when no longer needed. The caller should not retain a pointer to the voice.
	HarmonizerVoice* addVoice(HarmonizerVoice* newVoice);
	
	// deletes one of the voices
	void removeVoice(const int index);
	
	// returns a pointer to one of the voices that has been added
	HarmonizerVoice* getVoice(const int index) const;
	
	// deletes all harmony voices
	void deleteAllVoices();
	
	int getNumVoices() const noexcept { return voices.size(); }
	
	
	
protected:
	CriticalSection lock;
	OwnedArray<HarmonizerVoice> voices;
	int lastPitchWheelValue;
	
	// renders the voices for the given range
	void renderVoices (AudioBuffer<float>& outputAudio, const int startSample, const int numSamples);
	
	// MIDI
	void handleMidiEvent(const MidiMessage& m);
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
	
	double sampleRate;
	bool shouldStealNotes;
	uint32 lastNoteOnCounter;
	int minimumSubBlockSize;
	bool subBlockSubdivisionIsStrict;
	
	mutable Array<int> currentlyActiveNotes;
	
	// Starts a specified voice playing a specified note
	void startVoice (HarmonizerVoice* voice, const int midiPitch, const float velocity);
	
	// Stops a given voice.
	void stopVoice (HarmonizerVoice* voice, const float velocity, const bool allowTailOff);
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};
