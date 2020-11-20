/*
  ==============================================================================

    MidiPanningManager.h
    Created: 4 Nov 2020 6:41:49am
    Author:  Ben Vining

 stores list of possible midiPanning values based on desired stereo width of harmony signal
 
  ==============================================================================
*/

#pragma once

#ifndef NUMBER_OF_VOICES
#define NUMBER_OF_VOICES 12
#endif


class MidiPanningManager
{
public:
	
	MidiPanningManager(): middleIndex(ceil(NUMBER_OF_VOICES / 2)), indexOfLastSentPanVal(0)
	{
		mapArrayIndexes();
		for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
			possiblePanVals[i] = 64;
			panValsInAssigningOrder[i] = 64;
		}
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
	};
	
	
	int getNextPanVal() const {
		const int indexReadingFrom = indexOfLastSentPanVal + 1;
		if (indexReadingFrom < NUMBER_OF_VOICES) {
			return panValsInAssigningOrder[indexReadingFrom];
			indexOfLastSentPanVal = indexReadingFrom;
			if (indexOfLastSentPanVal >= NUMBER_OF_VOICES) {
				indexOfLastSentPanVal = 0;
			}
		} else {
			return panValsInAssigningOrder[0];
			indexOfLastSentPanVal = 0;
		}
	};
	
	
	void reset() {  // run this function when all voices are cleared
		indexOfLastSentPanVal = 0;
	};
	
	
	int retrievePanVal(const int index) const {
		return panValsInAssigningOrder[index];
	};
								   							
	
private:

	const int middleIndex;
	
	int possiblePanVals[NUMBER_OF_VOICES];
	int panValsInAssigningOrder[NUMBER_OF_VOICES];
	int arrayIndexesMapped[NUMBER_OF_VOICES];
	mutable int indexOfLastSentPanVal;
	
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
