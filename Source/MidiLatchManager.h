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
	MidiLatchManager() {
		
	};
	
	void clear() {
		// clears all elements from the bucket
	};
	
	void noteOffRecieved(const int noteNumber) {
		// adds noteNumber to the bucket
	};
	
	void noteOnRecieved(const int noteNumber) {
		// removes noteNumber from the bucket
	};
	
	// function to send all note offs once latch is deactivated 
	
private:
	
};
