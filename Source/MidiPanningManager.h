/*
  ==============================================================================

    MidiPanningManager.h
    Created: 4 Nov 2020 6:41:49am
    Author:  Ben Vining

 stores list of possible midiPanning values based on desired stereo width of harmony signal
 
  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MidiPanningManager
{
public:
	
	void updateStereoWidth(int newStereoWidth) {
		
	};
	
	
	int getNextPanVal() {
		return 0;
	};
	
	
private:
	const static int numberOfVoices = 12;  // link this to global # of voices setting
	int possiblePanVals[numberOfVoices] = { };
};
