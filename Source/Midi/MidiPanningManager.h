/*
  ==============================================================================

    MidiPanningManager.h
    Created: 4 Nov 2020 6:41:49am
    Author:  Ben Vining

 		Stores list of possible midiPanning values based on the desired stereo width of the harmony signal.
 
 		Attempts to assign panning values from the "middle out" -- even with a selected stereo width of 100, the first MIDI panning value sent will be 64. The goal is for the sound to get wider as more polyphony voices are turned on.
 
  ==============================================================================
*/

#pragma once

#include "GlobalDefinitions.h"


class MidiPanningManager
{
public:
	
	MidiPanningManager(): middleIndex(ceil(NUMBER_OF_VOICES / 2)), lastsentoverflow(0)
	{
		mapArrayIndexes();
		for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
			possiblePanVals[i] = 64;
			panValsInAssigningOrder[i] = 64;
		}
		newPanValAbsDist.ensureStorageAllocated(NUMBER_OF_VOICES);
		newPanValAbsDist.fill(0);
		newPanValsLeft.ensureStorageAllocated(NUMBER_OF_VOICES);
		newPanValsLeft.fill(0);
		availablePanValIndexes.ensureStorageAllocated(NUMBER_OF_VOICES);
		availablePanValIndexes.fill(0);
		
		reset();
	};
	
	
	void updateStereoWidth(const int newStereoWidth) {
		
		const float rangeMultiplier = newStereoWidth/100.0f;
		const float maxPan = 63.5f + (63.5f * rangeMultiplier);
		const float minPan = 63.5f - (63.5f * rangeMultiplier);
		const float increment = (maxPan - minPan) / NUMBER_OF_VOICES;
		
		// first, assign all possible, evenly spaced pan vals within range to an array
		for (int i = 0; i < NUMBER_OF_VOICES - 1; ++i) {
			const float panningVal = minPan + (i * increment) + (increment/2.0f);
			const int panning = round(panningVal);
			possiblePanVals[i] = panning;
		}
		
		// then reorder them into "assigning order" -- center out, by writing from the possiblePanVals array to the panValsInAssigningOrder array in the array index order held in arrayIndexesMapped
		for (int i = 0; i < NUMBER_OF_VOICES; ++i) {
			panValsInAssigningOrder[i] = possiblePanVals[arrayIndexesMapped[i]];
		}
		
		newPanValsLeft.clearQuick();
		availablePanValIndexes.clearQuick();
		for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
			newPanValsLeft.add(panValsInAssigningOrder[i]);
			availablePanValIndexes.add(i);
		}
	};
	
	
	int getNextPanVal() {
		if(availablePanValIndexes.isEmpty() == false)
		{
			lastsentoverflow = 0;
			availablePanValIndexes.sort();
			const int indexReadingFrom = availablePanValIndexes.getUnchecked(0);
			availablePanValIndexes.remove(0);
			return panValsInAssigningOrder[indexReadingFrom];
		}
		else
		{
			const int returnpan = panValsInAssigningOrder[lastsentoverflow];
			++lastsentoverflow;
			return returnpan;
		}
	};
	
	
	void turnedoffPanVal(const int newAvailPanVal)
	{
		int newindex = -1;
		for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
			if(panValsInAssigningOrder[i] == newAvailPanVal) {
				newindex = i;
				break;
			}
		}
		
		if(newindex > -1) {
			availablePanValIndexes.add(newindex);
			availablePanValIndexes.sort();
		}
	};
	
	
	void reset() {  // run this function when all voices are cleared
		availablePanValIndexes.clearQuick();
		for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
			availablePanValIndexes.add(i);
		}
		lastsentoverflow = 0;
	};
	
	
	int getClosestNewPanVal(const int prevPan)
	{
		// find the value in list of new pan values whose absolute value of its distance from the voice's old pan val is the smallest
		newPanValAbsDist.clearQuick();
		for(int i = 0; i < newPanValsLeft.size(); ++i) {
			const int distance = prevPan - newPanValsLeft.getUnchecked(i);
			newPanValAbsDist.add(abs(distance));
		}
		
		int min = newPanValAbsDist.getUnchecked(0);
		for(int i = 1; i < newPanValAbsDist.size(); ++i) {
			if(newPanValAbsDist.getUnchecked(i) < min) {
				min = newPanValAbsDist.getUnchecked(i);
			}
		}
		
		const int newpanindex = newPanValAbsDist.indexOf(min);
		const int newpan = newPanValsLeft.getUnchecked(newpanindex);
		newPanValsLeft.remove(newpanindex);
		availablePanValIndexes.remove(availablePanValIndexes.indexOf(newpan));
		return newpan;
	};
	
	
	
								   							
	
private:

	const int middleIndex;
	
	int possiblePanVals[NUMBER_OF_VOICES];
	int panValsInAssigningOrder[NUMBER_OF_VOICES];
	int arrayIndexesMapped[NUMBER_OF_VOICES];
	
	Array<int> newPanValAbsDist;
	Array<int> newPanValsLeft;
	Array<int> availablePanValIndexes;
	
	int lastsentoverflow;
	
	void mapArrayIndexes() {
		/* In my updateStereoWidth() function, possible panning values are written to the possiblePanVals array in order from least to greatest absolute value. Index 0 will contain the smallest midiPan value, and index 11 the highest.
		 
		 When an instance of the harmony algorithm requests a new midiPan value, I want to assign them from the "center out" -- so that the first voice that turns on will be the one in the center, then the sides get added as more voices turn on.
		 
		 So I need to transfer the values in possiblePanVals into another array - panValsInAssigningOrder - which will hold the panning values in order so that index 0 contains 64, index 1 contains 72, index 2 contains 52... index 10 contains 127 and index 11 contains 0.
		 
		 In order to transfer the panning values from array to array like this, I need to essentially have a list of array indices of possiblePanVals to read from, in order of the panValsInAssigningOrder indices I'll be writing to. IE, in this list, index 0 will contain 6, meaning that I want to write the value in index 6 of possiblePanVals to index 0 of panValsInAssigningOrder.
		 
		 I'm storing this list in another array called arrayIndexesMapped.
		 */
		
		arrayIndexesMapped[0] = middleIndex;
		
		int i = 1;
		int p = 1;
		int m = -1;
		
		while (i < NUMBER_OF_VOICES) {
			if(i % 2 == 0) {
				// i is even
				if(middleIndex + p)
					arrayIndexesMapped[i] = middleIndex + p;
				++p;
			} else {
				// i is odd
				arrayIndexesMapped[i] = middleIndex - m;
				--m;
			}
			++i;
		}
	};
	
};
