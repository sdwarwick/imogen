/*
  ==============================================================================

    MidiProcessor.h
    Created: 3 Nov 2020 1:44:22am
    Author:  Ben Vining
 
 	This class processes incoming MIDI. Note ons and offs are routed to the appropriate functions of the PolhyphonyVoiceManager class (PolyphonyVoiceManager will actually DO the reporting/storing/recalling of voices), and resulting data is routed to the appropraite instance of HarmonyVoice.
 
 
 	HELPER CLASSES:
 
 		* PolyphonyVoiceManager.h	: keeps track of which instances of HarmonyVoice are currently active, and what pitches are being played by the active voices. Essentially a wrapper class for an array of size NUMBER_OF_VOICES containing midiPitch values. -1 is an inactive voice.
 
 		* VoiceStealingManager.h 	: voice "stealing" is the concept that if all available instances of HarmonyVoice are already active with a previous note when a new note on comes in, the Harmonizer can intelligently choose which HarmonyVoice instance to assign the new note to -- "stealing" that voice from its old note. VoiceStealingManager keeps track of which voie # has been on the LONGEST, so that the stolen voice will always be the oldest pitch
 
 		* MidiPanningManager.h 		: creates and stores a list of possible MIDIpanning values based on the user's selected "stereo width" parameter, and attempts to assign these values to newly activated voices from the "middle out".
 
 		* MidiLatchManager.h 		: processes incoming note events while MIDI LATCH is active (and allows latch to be turned OFF)

  ==============================================================================
*/

#pragma once

#include "GlobalDefinitions.h"
#include "PolyphonyVoiceManager.h"
#include "VoiceStealingManager.h"
#include "MidiPanningManager.h"
#include "MidiLatchManager.h"


class MidiProcessor
{
	
public:
	
	MidiProcessor(OwnedArray<HarmonyVoice>& h): harmonyEngine(h), polyphonyManager(midiPanningManager, stealingManager, latchManager), lastRecievedPitchBend(64), isStealingOn(true), lowestPannedNote(0), prevPedalPitch(128)
	{
		activePitches.ensureStorageAllocated(NUMBER_OF_VOICES);
	};
	
	
	// the "MIDI CALLBACK" ::
	void processIncomingMidi (MidiBuffer& midiMessages, const bool midiLatch, const bool stealing, const int lowestPannedMidipitch, const bool isPedalPitchOn, const int pedalPitchThresh)
	{
		isStealingOn = stealing;
		lowestPannedNote = lowestPannedMidipitch;
		
		for (const MidiMessageMetadata meta : midiMessages)
		{
			const MidiMessage currentMessage = meta.getMessage();
		//	const int samplePos = meta.samplePosition;
			
			if(currentMessage.isNoteOnOrOff())
			{
				if(midiLatch == false) {
					if(currentMessage.isNoteOn()) {
						harmonyNoteOn(currentMessage);  // voice "stealing" is dealt with inside these functions.
					} else {
						harmonyNoteOff(currentMessage.getNoteNumber());
					}
				} else {
					processActiveLatch(currentMessage);
				}
			}
			else
			{
				if(currentMessage.isPitchWheel())
				{
					const int pitchBend = currentMessage.getPitchWheelValue();
					for(int i = 0; i < NUMBER_OF_VOICES; ++i) { harmonyEngine[i]->pitchBend(pitchBend); }
					lastRecievedPitchBend = pitchBend;
				} else {
					// non-note events go to here...
					// sustain pedal, aftertouch, key pressure, etc...
				}
			}
			
			if (currentMessage.isAllNotesOff() || currentMessage.isAllSoundOff()) { killAll(); }
		}
		
		if(isPedalPitchOn) {
			pedalPitch(pedalPitchThresh); // doubles the lowest active note an octave below
		} else {
			if(prevPedalPitchOnOff) {
				if(prevPedalPitch != 128) { harmonyNoteOff(prevPedalPitch); }
			}
		}
		prevPedalPitchOnOff = isPedalPitchOn;
	};
	// :: END MIDI CALLBACK
	
	
	// run this function to kill all active harmony notes:
	void killAll() {  // run this function to clear all held / turned on midi notes
		for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
			const int returnedmidipitch = polyphonyManager.pitchAtIndex(i);
			if (returnedmidipitch != -1) { harmonyNoteOff(returnedmidipitch); }
		}
		if(polyphonyManager.areAllVoicesOff() != true) {
			polyphonyManager.clear();
		}
		prevPedalPitch = 128;
	};

	
	void updateStereoWidth(float* newStereoWidth) {
		midiPanningManager.updateStereoWidth(*newStereoWidth);
		for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
			if(harmonyEngine[i]->voiceIsOn) {
				const int newPan = midiPanningManager.getClosestNewPanVal(harmonyEngine[i]->reportPan());
				harmonyEngine[i]->changePanning(newPan);
			}
		}
	};
	
	
	void turnOffLatch()  // run this function to turn off latch & send held note offs out to harmony engine
	{
		for(int i = 0; i < NUMBER_OF_VOICES; ++i)
		{
			const int returnedVal = latchManager.noteAtIndex(i);
			if(returnedVal != -1) { harmonyNoteOff(returnedVal); }
		}
		latchManager.clear();
	};
	
	
	Array<int> getActivePitches()
	{
		activePitches.clearQuick();
		
		for(int i = 0; i < NUMBER_OF_VOICES; ++i)
		{
			const int testpitch = polyphonyManager.pitchAtIndex(i);
			if(testpitch != -1) {
				activePitches.add(testpitch);
			}
		}
		
		if(activePitches.isEmpty() == false)
		{
			activePitches.sort();
		} else {
			activePitches.add(-1);
		}
		
		return activePitches;
	};
	
	
	
private:
	OwnedArray<HarmonyVoice>& harmonyEngine;
	
	PolyphonyVoiceManager polyphonyManager;
	MidiLatchManager latchManager;
	VoiceStealingManager stealingManager;
	MidiPanningManager midiPanningManager;
	
	int lastRecievedPitchBend;
	Array<int> activePitches;
	bool isStealingOn;
	bool prevPedalPitchOnOff;
	int lowestPannedNote;
	int prevPedalPitch;
	
	
	// sends a note on out to the harmony engine
	void harmonyNoteOn(const MidiMessage currentMessage)
	{
		if(currentMessage.isNoteOn()) {
			const int newPitch = currentMessage.getNoteNumber();
			const int newVelocity = currentMessage.getVelocity();
			const int newVoiceNumber = polyphonyManager.nextAvailableVoice();  // returns -1 if no voices are available
			int panningvalue = 64;
		
			if(newPitch >= lowestPannedNote) {
				panningvalue = midiPanningManager.getNextPanVal();
			} else {
				panningvalue = 64;
			}
			const int panval = panningvalue;
		
			if(newVoiceNumber > 0 && newVoiceNumber < NUMBER_OF_VOICES) {
				polyphonyManager.updatePitchCollection(newVoiceNumber, newPitch);
				stealingManager.addSentVoice(newVoiceNumber);
				harmonyEngine[newVoiceNumber]->startNote(newPitch, newVelocity, panval, lastRecievedPitchBend);
			} else {
				if (isStealingOn == true) {
					const int voiceToSteal = stealingManager.voiceToSteal();
					if(voiceToSteal > 0 && voiceToSteal < NUMBER_OF_VOICES) {
						polyphonyManager.updatePitchCollection(voiceToSteal, newPitch);
						stealingManager.addSentVoice(voiceToSteal);
						harmonyEngine[voiceToSteal]->changeNote(newPitch, newVelocity, panval, lastRecievedPitchBend);
					}
				}
			}
		}
	};
	
	
	// sends a note off out to the harmony engine
	void harmonyNoteOff(const int pitch) {
		const int voiceToTurnOff = polyphonyManager.getIndex(pitch);
		if(voiceToTurnOff > 0 && voiceToTurnOff < NUMBER_OF_VOICES) {
			polyphonyManager.updatePitchCollection(voiceToTurnOff, -1);
			stealingManager.removeSentVoice(voiceToTurnOff);
			midiPanningManager.turnedoffPanVal(harmonyEngine[voiceToTurnOff]->reportPan());
			harmonyEngine[voiceToTurnOff]->stopNote();
		}
	};
	
	
	void processActiveLatch(const MidiMessage currentMessage)
	{
		const int midiPitch = currentMessage.getNoteNumber();
		if(currentMessage.isNoteOn())
		{
			if (polyphonyManager.isPitchActive(midiPitch) == false) {
				harmonyNoteOn(currentMessage);  // if note isn't already on (latched), then turn it on
			} else {
				latchManager.noteOnRecieved(midiPitch);
			}
		} else {
			latchManager.noteOffRecieved(midiPitch);
		}
	}; // processes note events that occur while midiLatch is active
	
	
	void pedalPitch(const int thresholdPitch) {
		// this function doubles the lowest currently active midiPitch an octave lower. the argument is the "threshold", ie the highest midiPitch that will be doubled 8vb. the lowest pitch that will be doubled is 0.
		const int lowestActive = polyphonyManager.getLowestActiveNote();
		if(lowestActive > -1) {
			if(lowestActive <= thresholdPitch) {
				const int newpedalpitch = lowestActive - 12;
				if(newpedalpitch >= 0) {
					if(newpedalpitch != prevPedalPitch) {
						if(prevPedalPitch != 128) { harmonyNoteOff(prevPedalPitch); }
						auto newNoteOn = juce::MidiMessage::noteOn(1, newpedalpitch, (juce::uint8) 127);
						harmonyNoteOn(newNoteOn);
						prevPedalPitch = newpedalpitch;
					}
				} else {
					if(prevPedalPitch != 128) { harmonyNoteOff(prevPedalPitch); }
				}
			} else {
				if(prevPedalPitch != 128) { harmonyNoteOff(prevPedalPitch); }
				prevPedalPitch = 128;
			}
		} else {
			if(prevPedalPitch != 128) { harmonyNoteOff(prevPedalPitch); }
			prevPedalPitch = 128;
		}
	}; // doubles the lowest active note an octave below
	

};



