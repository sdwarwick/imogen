/*
  ==============================================================================

    MidiProcessor.h
    Created: 3 Nov 2020 1:44:22am
    Author:  Ben Vining
 
 	This class processes incoming MIDI. Note ons and offs are routed to the appropriate functions of the PolhyphonyVoiceManager class (PolyphonyVoiceManager will actually DO the reporting/storing/recalling of voices), and resulting data is routed to the appropraite instance of HarmonyVoice.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "PolyphonyVoiceManager.h"
#include "MidiPanningManager.h"


class MidiProcessor
{
	
public:
											
	void processIncomingMidi (MidiBuffer& midiMessages, OwnedArray<HarmonyVoice>& harmonyEngine)
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
					const int newPitch = currentMessage.getNoteNumber();
					const int newVelocity = currentMessage.getVelocity();
					const int newVoiceNumber = polyphonyManager.nextAvailableVoice();  // returns -1 if no voices are available
					
					if(newVoiceNumber >= 0) {
						polyphonyManager.updatePitchCollection(newVoiceNumber, newPitch);
						harmonyEngine[newVoiceNumber]->startNote(newPitch, newVelocity, midiPanningManager.getNextPanVal());
					}
				}
				else
				{
					const int voiceToTurnOff = polyphonyManager.turnOffNote(currentMessage.getNoteNumber());
					polyphonyManager.updatePitchCollection(voiceToTurnOff, -1);
					harmonyEngine[voiceToTurnOff]->stopNote();
				}
			}
			else
			{
				// non-note events go to here...
				// pitch wheel / pitch bend, sustain pedal, aftertouch, key pressure, etc...
			}
		}
	};
	
	
	
	void updateStereoWidth(float* newStereoWidth) {
		midiPanningManager.updateStereoWidth(*newStereoWidth);
	};
	
	
private:
	

	
	PolyphonyVoiceManager polyphonyManager;
	MidiPanningManager midiPanningManager;
};
