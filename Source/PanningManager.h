/*
  ==============================================================================

    PanningManager.h
    Created: 14 Dec 2020 3:20:30pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"


class PanningManager
{
public:
	
	PanningManager();
	~PanningManager();
	
	// used to change the # of harmonizer voices currently active
	void setNumberOfVoices(const int newNumVoices);
	
	
	// used to update the width of the stereo field of currently available / valid pan values
	void updateStereoWidth(const int newWidth);
	
	
	// returns next available panning value
	int getNextPanVal();
	
	
	// used to tell the PanningManager when a previously assigned panningvalue is turned off - ie, is now available again for another voice 
	void panValTurnedOff(const int panVal);
	

	// used when updating stereo width -- voices should grab the new pan val that's closest to their old pan val
	int getClosestNewPanValFromOld(const int oldPan);
	
	
	// tells the PanningManager that all voices have been turned off -- ie, all the pan vals are available again
	// the boolean argument should normally be false. this is used for if reset() needs to be called in a flow where the first value has been used for a new voice
	void reset(const bool grabbedFirst64);
	
	
private:
	CriticalSection lock;
	
	Array<int> possiblePanVals;
	Array<int> panValsInAssigningOrder;
	Array<int> arrayIndexesMapped;
	
	Array<int> unsentPanVals; // this is the array we will actually be reading pan vals from! the others are for sorting
	
	Array<int> absDistances; // used for finding which new pan value is the closest to a voice's old pan val
	
	int lastRecievedStereoWidth;
	int currentNumVoices;
	
	void setNumVoicesPrivate(const int newNumVoices);
	
	void mapArrayIndexes();
	
	JUCE_LEAK_DETECTOR(PanningManager)
};
