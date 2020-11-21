/*
  ==============================================================================

    MidiLatchManager.h
    Created: 8 Nov 2020 12:45:27am
    Author:  Ben Vining
 
 		This class tracks any incoming note events recieved by the plugin while the MIDI LATCH option is ACTIVE :
 			* incoming note offs are collected into a list, so that if MIDI LATCH is turned off, the appropriate note offs can be sent to the Harmonizer and no notes will be "stuck on"
 			* incoming note ons...
 					* if not already an active harmony pitch, will be turned on without retriggering HarmonyVoices' ADSRs
 					* if they were previously on (ie, 'latched'), then that pitch will be REMOVED from LatchManager's held list of note offs to send upon MIDI LATCH being deactivated. The reason for this is because if the user re-presses a key while MIDI LATCH is still on, then releases MIDI LATCH but hasn't released that key, then of course that note should still be on. These notes are considered "retriggered".

  ==============================================================================
*/

#pragma once

#ifndef NUMBER_OF_VOICES
#define NUMBER_OF_VOICES 12
#endif


class MidiLatchManager
{
public:
	
	MidiLatchManager() {
		clear();
	};

	
	void clear() {
		for(int i = 0; i < NUMBER_OF_VOICES; ++i)
		{
			heldNoteOffs[i] = -1;
		}
	};
	
	
	void noteOffRecieved(const int noteNumber) {
		int i = 0;
		while (i < NUMBER_OF_VOICES)
		{
			if (heldNoteOffs[i] == -1) {
				heldNoteOffs[i] = noteNumber;
				break;
			} else {
				++i;
			}
		}
	};
	
	
	void noteOnRecieved(const int noteNumber) {
		int i = 0;
		while (i < NUMBER_OF_VOICES)
		{
			if (heldNoteOffs[i] == noteNumber) {
				heldNoteOffs[i] = -1;
				break;
			} else {
				++i;
			}
		}
	};
	
	
	int noteAtIndex(const int indexToRead) const {
		return heldNoteOffs[indexToRead];
	};
	
	
private:
	
	int heldNoteOffs[NUMBER_OF_VOICES]; // array holds all note offs recieved while latch is active. Holds -1 for "empty array slot"
};
