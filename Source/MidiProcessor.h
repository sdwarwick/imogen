/*
  ==============================================================================

    MidiProcessor.h
    Created: 3 Nov 2020 1:44:22am
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PolyphonyVoiceManager.h"

class MidiProcessor
{
	
public:
	
	void assignNewNotes (MidiBuffer& midiMessages, PolyphonyVoiceManager polyphonyManager)
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
				/* if it's a note on:
				 - find the # of the next available voice in harmEngine
				 - assign the midiPitch & velocity to that instance
				 */
				
					int newPitch = currentMessage.getNoteNumber();
					int newVelocity = currentMessage.getVelocity();
					
					int newVoiceNumber = polyphonyManager.nextAvailableVoice();
					
					polyphonyManager.updatePitchCollection(newVoiceNumber, newVelocity);
					
					// harmEngine[newVoiceNumber]->startNote(newPitch, velocity);
					// need to transmit note data to appropriate instance of HarmonyVoice within harmEngine
				
				}
				else
				{
				/*
				  if it's a note off:
				 - find the # of the voice that was playing that pitch
				 - turn that voice off
				 */
					
					int voiceToTurnOff = polyphonyManager.turnOffNote(currentMessage.getNoteNumber());
					
					// harmEngine[voiceToTurnOff]->stopNote();
					// need to transmit note data to appropriate instance of HarmonyVoice within harmEngine
				}
			}
			else
				// non-note events go to here...
			{
				
			}
		}
	}
	
//	PolyphonyVoiceManager polyphonyManager;
	
private:
	
};
