/*
  ==============================================================================

    MidiProcessor.h
    Created: 3 Nov 2020 1:44:22am
    Author:  Ben Vining
 
 	This class processes incoming MIDI. Note ons and offs are routed to the appropriate functions of the PolhyphonyVoiceManager class (PolyphonyVoiceManager will actually DO the reporting/storing/recalling of voices).

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "PolyphonyVoiceManager.h"


class MidiProcessor
{
	
public:
													// currently taking polyphonyManager passed from audio process block as a reference -- but be sure that it's actually changing the PROCESS BLOCK's version of polyphonyManager and not just a copy local to this class!
											
	void assignNewNotes (MidiBuffer& midiMessages, PolyphonyVoiceManager& polyphonyManager)
	{
		MidiBuffer::Iterator it(midiMessages);
		MidiMessage currentMessage;
		int samplePos;
		
		while (it.getNextEvent(currentMessage, samplePos))
		{
			if(currentMessage.isNoteOnOrOff())
			{
				if(currentMessage.isNoteOn())
				{
					int newPitch = currentMessage.getNoteNumber();
					int newVelocity = currentMessage.getVelocity();
					
					int newVoiceNumber = polyphonyManager.nextAvailableVoice();
					
					polyphonyManager.updatePitchCollection(newVoiceNumber, newPitch); // need to save to AUDIO PROCESS BLOCK'S copy of polyphonyManager and not just a copy local to this MidiProcessor class!
					
					
					// need to transmit note data to appropriate instance of HarmonyVoice within harmEngine
					// harmEngine[voiceNumber]->startNote(pitch, velocity);
					
			//		harmEngine[newVoiceNumber].HarmonyVoice::startNote(newPitch, newVelocity);
				
				}
				else
				{
					int voiceToTurnOff = polyphonyManager.turnOffNote(currentMessage.getNoteNumber());
					
					polyphonyManager.updatePitchCollection(voiceToTurnOff, -1);
					
					// harmEngine[voiceToTurnOff]->stopNote();
					// need to transmit note data to appropriate instance of HarmonyVoice within harmEngine
				}
			}
			else
				// non-note events go to here...
				// pitch wheel / pitch bend, sustain pedal, etc...
			{
				
			}
		}
	}
	
private:
	
};
