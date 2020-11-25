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
	
	MidiProcessor(OwnedArray<HarmonyVoice>& h): harmonyEngine(h), polyphonyManager(midiPanningManager), lastRecievedPitchBend(64), isStealingOn(true)
	{
		activePitches.ensureStorageAllocated(NUMBER_OF_VOICES);
	};
	
	
	// the "MIDI CALLBACK" ::
	void processIncomingMidi (MidiBuffer& midiMessages, const bool midiLatch, const bool stealing)
	{
		isStealingOn = stealing;
		
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
		} else {
			midiPanningManager.reset();
		}
		stealingManager.clear();
		latchManager.clear();
	};

	
	void updateStereoWidth(float* newStereoWidth) {
		midiPanningManager.updateStereoWidth(*newStereoWidth);
		int activeVoiceNumber = 0;
		for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
			if(harmonyEngine[i]->voiceIsOn) {
				harmonyEngine[i]->changePanning(midiPanningManager.retrievePanVal(activeVoiceNumber));
				++activeVoiceNumber;
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
		
		activePitches.removeAllInstancesOf(-1);
		
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
	
	
	
	// sends a note on out to the harmony engine
	void harmonyNoteOn(const MidiMessage currentMessage)
	{
		const int newPitch = currentMessage.getNoteNumber();
		const int newVelocity = currentMessage.getVelocity();
		const int newVoiceNumber = polyphonyManager.nextAvailableVoice();  // returns -1 if no voices are available
		
		if(newVoiceNumber > 0 && newVoiceNumber < NUMBER_OF_VOICES) {
			polyphonyManager.updatePitchCollection(newVoiceNumber, newPitch);
			stealingManager.addSentVoice(newVoiceNumber);
			harmonyEngine[newVoiceNumber]->startNote(newPitch, newVelocity, midiPanningManager.getNextPanVal(), lastRecievedPitchBend);
		} else {
			if (isStealingOn == true) {
				const int voiceToSteal = stealingManager.voiceToSteal();
				if(voiceToSteal > 0 && voiceToSteal < NUMBER_OF_VOICES) {
					polyphonyManager.updatePitchCollection(voiceToSteal, newPitch);
					stealingManager.addSentVoice(voiceToSteal);
					harmonyEngine[voiceToSteal]->changeNote(newPitch, newVelocity, midiPanningManager.getNextPanVal(), lastRecievedPitchBend);
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
};

