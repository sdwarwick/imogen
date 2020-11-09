/*
  ==============================================================================

    MidiLatchManager.h
    Created: 8 Nov 2020 12:45:27am
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once


class MidiLatchManager
{
public:

	void clear() {
		for(int i = 0; i < numberOfVoices; ++i)
		{
			heldNoteOffs[i] = -1;
		}
	};
	
	
	void noteOffRecieved(const int noteNumber) {
		int i = 0;
		while (i < numberOfVoices)
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
		while (i < numberOfVoices)
		{
			if (heldNoteOffs[i] == noteNumber) {
				heldNoteOffs[i] = -1;
				break;
			} else {
				++i;
			}
		}
	};
	
	
	int noteAtIndex(const int indexToRead)
	{
		return heldNoteOffs[indexToRead];
	};
	
	
private:
	
	const static int numberOfVoices = 12; // the max # of notes that can be latched is the # of active instances of HarmonyVoice
										// link this to global numVoices setting
	
	int heldNoteOffs[numberOfVoices] = { -1 }; // array holds all note offs recieved while latch is active. Holds -1 for "empty array slot"
};
