/*
  ==============================================================================

    MidiLatchManager.h
    Created: 8 Nov 2020 12:45:27am
    Author:  Ben Vining

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
