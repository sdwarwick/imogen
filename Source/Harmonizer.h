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
	
	// returns the currently playing MIDI pitch. Returns -1 if no note is active.
	int getCurrentlyPlayingNote() const noexcept { return currentlyPlayingNote; }
	
	// checks whether or not the voice is currently active
	bool isVoiceActive() const { return currentlyPlayingNote >= 0; }
	
	// called to start a new note. Will be called during the rendering callback.
	void startNote(int midiPitch, float velocity, int currentPitchWheelPosition);
	
	// called to stop a note. Will be called during the rendering callback.
	// The velocity indicates how quickly the note was released - 0 is slowly, 1 is quickly.
	// If allowTailOff is false, sound stops immediately
	// If allowTailOff is true, then it's allowed to begin fading out its sound, and it can stop playing until it's finished.
	void stopNote(float velocity, bool allowTailOff);
	
	// called to let the voice know of a pitch bend change. Will be called during the rendering callback.
	void pitchWheelMoved(int newPitchWheelValue);
	
	// called to let the voice know that a MIDI controller has been moved. Will be called during the rendering callback.
	void controllerMoved(int controllerNumber, int newControllerValue);
	
	// called to let the voice know that the aftertouch has changed. Will be called during the rendering callback.
	void aftertouchChanged(int newAftertouchValue);
	
	// called to let the voice know that the channel pressure has changed. Will be called during the rendering callback.
	void channelPressureChanged(int newChannelPressureValue);
	
	
	// renders the next block of audio data for this voice.
	// The output audio data will be added to the current contents of the buffer provided.
	// Only the region of the buffer between startSample and (startSample + numSamples) should be altered by this method.
	// If the voice is currently silent, it should just return without doing anything.
	// The size of the blocks that are rendered can change each time it is called, and may involve rendering as little as 1 sample at a time.
	// In between rendering callbacks, the voice's methods will be called to tell it about note and controller events.
	void renderNextBlock(AudioBuffer<float> outputBuffer, int startSample, int numSamples);
	
	
	void setCurrentPlaybackSamplerate(double newRate) { currentSampleRate = newRate; }
	
	double getSamplerate() const noexcept { return currentSampleRate; }
	
	// Returns true if the key that triggered this voice is still held down. Note that the voice may still be playing after the key was released (e.g because the sostenuto pedal is down).
	bool isKeyDown() const noexcept { return keyIsDown; }
	
	void setKeyDown(bool isNowDown) noexcept { keyIsDown = isNowDown; }
	
	// returns true if the sustain pedal is currently active for this voice
	bool isSustainPedalDown() const noexcept { return sustainPedalDown; }
	
	void setSustainPedalDown(bool isNowDown) noexcept { sustainPedalDown = isNowDown; }
	
	// returns true if the sostenuto pedal is currently active for this voice
	bool isSostenutoPedalDown() const noexcept { return sostenutoPedalDown; }
	
	void setSostenutoPedalDown(bool isNowDown) noexcept { sostenutoPedalDown = isNowDown; }
	
	// returns true if a voice is sounding, but in the release phase of its ADSR envelope
	bool isPlayingButReleased() const noexcept
	{
		return isVoiceActive() && ! (keyIsDown || sostenutoPedalDown || sustainPedalDown);
	}
	
	// Returns true if this voice started playing its current note before the other voice did. */
	bool wasStartedBefore (const HarmonizerVoice& other) const noexcept { return noteOnTime < other.noteOnTime; }
	
	
	void updateAdsrSettings(float attack, float decay, float sustain, float release);
	
	
protected:
	//Resets the state of this voice after a note has finished playing. The subclass must call this when it finishes playing a note and becomes available to play new ones. It must either call it in the stopNote() method, or if the voice is tailing off, then it should call it later during the renderNextBlock method, as soon as it finishes its tail-off. It can also be called at any time during the render callback if the sound happens
	void clearCurrentNote() { currentlyPlayingNote = -1; }
	
	
private:
	
	friend class Harmonizer;
	
	ADSR adsr;
	ADSR::Parameters adsrParams;
	
	int currentlyPlayingNote;
	double currentSampleRate;
	uint32 noteOnTime;
	bool keyIsDown, sustainPedalDown, sostenutoPedalDown;
	
	AudioBuffer<float> tempBuffer;
	
	void esola(AudioBuffer<float>& outputBuffer, int startSample, int numSamples);
	
	JUCE_LEAK_DETECTOR(HarmonizerVoice);
};



class Harmonizer
{
public:
	Harmonizer();
	
	~Harmonizer();
	
	// deletes all harmony voices
	void deleteAllVoices();
	
	// returns the number of currently active harmony voices
	int getNumVoices() const noexcept { return voices.size(); }
	
	// returns a pointer to one of the voices that has been added
	HarmonizerVoice* getVoice(int index) const;
	
	// Adds a new voice to the harmonizer. The object passed in will be managed by the synthesiser, which will delete it later on when no longer needed. The caller should not retain a pointer to the voice.
	HarmonizerVoice* addVoice(HarmonizerVoice* newVoice);
	
	// deletes one of the voices
	void removeVoice(int index);
	
	
	// If set to true, then the synth will try to take over an existing voice if it runs out and needs to play another note.
	void setNoteStealingEnabled (bool shouldSteal) { shouldStealNotes = shouldSteal; }
	
	// Returns true if note-stealing is enabled.
	bool isNoteStealingEnabled() const noexcept { return shouldStealNotes; }
	
	
	// triggers a note-on event. Finds a free voice and uses the voice to generate sound for the desired pitch.
	void noteOn(int midiPitch, float velocity);
	
	// triggers a note-off event. If allowTailOff is true, the voices will be allowed to fade out the notes gracefully (if they can do). If this is false, the notes will all be cut off immediately.
	void noteOff (int midiNoteNumber, float velocity, bool allowTailOff);
	
	// turn off all notes
	void allNotesOff(bool allowTailOff);
	
	
	// sends a pitch wheel message to all voices
	void handlePitchWheel(int wheelValue);
	
	// sends a midi CC message to all voices.
	// controllerNumber = the midi controller type, as returned by MidiMessage::getControllerNumber()
	// controllerValue  = the midi controller value, between 0 and 127, as returned by MidiMessage::getControllerValue()
	void handleController(int controllerNumber, int controllerValue);
	
	// sends an aftertouch message to any voices that are playing the applicable note
	// aftertouchValue = the aftertouch value, between 0 and 127, as returned by MidiMessage::getAftertouchValue()
	void handleAftertouch(int midiNoteNumber, int aftertouchValue);
	
	void handleChannelPressure(int channelPressureValue);
	
	void handleSustainPedal(bool isDown);
	
	void handleSostenutoPedal(bool isDown);
	
	void handleSoftPedal(bool isDown);
	
	
	void setCurrentPlaybackSampleRate(double newRate);
	
	double getSamplerate() const noexcept { return sampleRate; }
	
	void updateADSRsettings(float attack, float decay, float sustain, float release);
	
	
	// Creates the next block of audio output.
	// This will process the next numSamples of data from all the voices, and add that output to the audio block supplied, starting from the offset specified. Note that the data will be added to the current contents of the buffer, so you should clear it before calling this method if necessary.
	// The midi events in the inputMidi buffer are parsed for note and controller events, and these are used to trigger the voices. Note that the startSample offset applies both to the audio output buffer and the midi input buffer, so any midi events with timestamps outside the specified region will be ignored.
	void renderNextBlock(AudioBuffer<float>& outputAudio, const MidiBuffer& inputMidi, int startSample, int numSamples);
	
	
	// Sets a minimum limit on the size to which audio sub-blocks will be divided when rendering. When rendering, the audio blocks that are passed into renderNextBlock() will be split up into smaller blocks that lie between all the incoming midi messages, and it is these smaller sub-blocks that are rendered with multiple calls to renderVoices(). Obviously in a pathological case where there are midi messages on every sample, then renderVoices() could be called once per sample and lead to poor performance, so this setting allows you to set a lower limit on the block size. The default setting is 32, which means that midi messages are accurate to about < 1ms accuracy, which is probably fine for most purposes, but you may want to increase or decrease this value for your synth. If shouldBeStrict is true, the audio sub-blocks will strictly never be smaller than numSamples. If shouldBeStrict is false (default), the first audio sub-block in the buffer is allowed to be smaller, to make sure that the first MIDI event in a buffer will always be sample-accurate (this can sometimes help to avoid quantisation or phasing issues).
	void setMinimumRenderingSubdivisionSize (int numSamples, bool shouldBeStrict = false) noexcept;
	
	
protected:
	CriticalSection lock;
	OwnedArray<HarmonizerVoice> voices;
	int lastPitchWheelValue;
	
	// Searches through the voices to find one that's not currently playing. Returns nullptr if all voices are busy and stealing isn't enabled.
	HarmonizerVoice* findFreeVoice (int midiNoteNumber, bool stealIfNoneAvailable) const;
	
	// Chooses a voice that is most suitable for being re-used. The default method will attempt to find the oldest voice that isn't the bottom or top note being played.
	HarmonizerVoice* findVoiceToSteal (int midiNoteNumber) const;
	
	
	// renders the voices for the given range
	void renderVoices (AudioBuffer<float>& outputAudio, int startSample, int numSamples);
	
	
	void handleMidiEvent(const MidiMessage& m);
	
	
private:
	
	double sampleRate;
	bool shouldStealNotes;
	uint32 lastNoteOnCounter;
	int minimumSubBlockSize;
	bool subBlockSubdivisionIsStrict;
	
	// Starts a specified voice playing a specified note
	void startVoice (HarmonizerVoice* voice, int midiPitch, float velocity);
	
	// Stops a given voice.
	void stopVoice (HarmonizerVoice* voice, float velocity, bool allowTailOff);
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};
