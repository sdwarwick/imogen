/*
  ==============================================================================

    MidiProcessor.h
    Created: 3 Nov 2020 1:44:22am
    Author:  Ben Vining
 
 	This class processes incoming MIDI. Note ons and offs are routed to the appropriate functions of the PolhyphonyVoiceManager class (PolyphonyVoiceManager will actually DO the reporting/storing/recalling of voices), and resulting data is routed to the appropraite instance of HarmonyVoice.

  ==============================================================================
*/

#pragma once

#include "PolyphonyVoiceManager.h"
#include "VoiceStealingManager.h"
#include "MidiPanningManager.h"
#include "MidiLatchManager.h"

#ifndef NUMBER_OF_VOICES
#define NUMBER_OF_VOICES 12
#endif

class MidiProcessor
{
	
public:
	
	MidiProcessor(OwnedArray<HarmonyVoice>& h): harmonyEngine(h), lastRecievedPitchBend(64)
	{
		numberOfVoices = NUMBER_OF_VOICES;
		
	};
	
	
	// the "MIDI CALLBACK" ::
	void processIncomingMidi (MidiBuffer& midiMessages, const bool midiLatch, const bool stealing)
	{
		for (const MidiMessageMetadata meta : midiMessages)
		{
			const MidiMessage currentMessage = meta.getMessage();
		//	const int samplePos = meta.samplePosition;
			
			if(currentMessage.isNoteOnOrOff())
			{
				if(midiLatch == false) {
					if(currentMessage.isNoteOn()) {
						harmonyNoteOn(currentMessage, stealing);  // voice "stealing" is dealt with inside these functions.
					} else {
						harmonyNoteOff(currentMessage.getNoteNumber());
					}
				} else {
					processActiveLatch(currentMessage, stealing);
				}
			}
			else
			{
				if(currentMessage.isPitchWheel())
				{
					const int pitchBend = currentMessage.getPitchWheelValue();
					for(int i = 0; i < numberOfVoices; ++i) { harmonyEngine[i]->pitchBend(pitchBend); }
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
		for(int i = 0; i < numberOfVoices; ++i) {
			const int returnedmidipitch = polyphonyManager.pitchAtIndex(i);
			if (returnedmidipitch != -1) { harmonyNoteOff(returnedmidipitch); }
		}
		polyphonyManager.clear();
		stealingManager.clear();
		midiPanningManager.reset();
		latchManager.clear();
	};

	
	void updateStereoWidth(float* newStereoWidth) {
		midiPanningManager.updateStereoWidth(*newStereoWidth);
	};
	
	
	void refreshMidiPanVal (const int voiceNumber, const int indexToRead) {
		harmonyEngine[voiceNumber]->changePanning(midiPanningManager.retrievePanVal(indexToRead));
	};
	
	
	void turnOffLatch()  // run this function to turn off latch & send held note offs out to harmony engine
	{
		for(int i = 0; i < numberOfVoices; ++i)
		{
			const int returnedVal = latchManager.noteAtIndex(i);
			if(returnedVal != -1) { harmonyNoteOff(returnedVal); }
		}
		latchManager.clear();
	};
	
	
	
private:
	OwnedArray<HarmonyVoice>& harmonyEngine;
	
	int numberOfVoices;  // link this to global # of voices setting
	
	PolyphonyVoiceManager polyphonyManager;
	VoiceStealingManager stealingManager;
	MidiPanningManager midiPanningManager;
	MidiLatchManager latchManager;
	
	int lastRecievedPitchBend;
	
	// sends a note on out to the harmony engine
	void harmonyNoteOn(const MidiMessage currentMessage, const bool stealingIsOn)
	{
		const int newPitch = currentMessage.getNoteNumber();
		const int newVelocity = currentMessage.getVelocity();
		const int newVoiceNumber = polyphonyManager.nextAvailableVoice();  // returns -1 if no voices are available
		
		if(newVoiceNumber >= 0 && newVoiceNumber < numberOfVoices) {
			polyphonyManager.updatePitchCollection(newVoiceNumber, newPitch);
			stealingManager.addSentVoice(newVoiceNumber);
			harmonyEngine[newVoiceNumber]->startNote(newPitch, newVelocity, midiPanningManager.getNextPanVal(), lastRecievedPitchBend);
		} else {
			if (stealingIsOn == true) {
				const int voiceToSteal = stealingManager.voiceToSteal();
				polyphonyManager.updatePitchCollection(voiceToSteal, newPitch);
				stealingManager.addSentVoice(voiceToSteal);
				harmonyEngine[voiceToSteal]->changeNote(newPitch, newVelocity, midiPanningManager.getNextPanVal(), lastRecievedPitchBend);
			}
		}
	};
	
	
	// sends a note off out to the harmony engine
	void harmonyNoteOff(const int pitch) {
		const int voiceToTurnOff = polyphonyManager.turnOffNote(pitch);
		stealingManager.removeSentVoice(voiceToTurnOff);
		harmonyEngine[voiceToTurnOff]->stopNote();
	};
	
	
	void processActiveLatch(const MidiMessage currentMessage, const bool stealing)
	{
		const int midiPitch = currentMessage.getNoteNumber();
		if(currentMessage.isNoteOn())
		{
			if (polyphonyManager.isPitchActive(midiPitch) == false) {
				harmonyNoteOn(currentMessage, stealing);  // if note isn't already on (latched), then turn it on
			} else {
				latchManager.noteOnRecieved(midiPitch);
			}
		} else {
			latchManager.noteOffRecieved(midiPitch);
		}
	}; // processes note events that occur while midiLatch is active
};
