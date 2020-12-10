/*
  ==============================================================================

    StaffDisplay.cpp
    Created: 26 Nov 2020 11:27:15pm
    Author:  Ben Vining

  ==============================================================================
*/

#include <JuceHeader.h>
#include "StaffDisplay.h"

//==============================================================================
StaffDisplay::StaffDisplay(): useFlats(false)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

	yCoordsOfActiveNotes.ensureStorageAllocated(NUMBER_OF_VOICES);
	yCoordsOfActiveNotes.clearQuick();
	
	const int noteheadHeightPx = 15;
	yCoordLookupTable[0] = 0;
	for(int n = 1; n < 128; ++n)
	{
		const int modulo = n % 12;
		if(modulo == 1 || modulo == 3 || modulo == 6 || modulo == 8 || modulo == 10)
		{
			yCoordLookupTable[n] = yCoordLookupTable[n - 1];
		}
		else
		{
			yCoordLookupTable[n] = yCoordLookupTable[n - 1] + noteheadHeightPx;
		}
	}
}

StaffDisplay::~StaffDisplay()
{
	setLookAndFeel(nullptr);
}

void StaffDisplay::paint (juce::Graphics& g)
{
	
    g.fillAll (juce::Colours::ivory);

}

void StaffDisplay::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}


void StaffDisplay::displayFlats(const bool displayingFlats)
{
	useFlats = displayingFlats;
};


void StaffDisplay::drawPitches(Array<int> activePitches)
{
	if(activePitches.isEmpty() == false)
	{
		yCoordsOfActiveNotes.clearQuick();
		for(int n = 0; n < activePitches.size(); ++n)
		{
			if(activePitches.getUnchecked(n) > -1) {
			yCoordsOfActiveNotes.add(yCoordLookupTable[activePitches.getUnchecked(n)]);
			}
		}
		
		int xOffset = 0;
		float someScaleFactor = 28.0f; // the scaleing factor applied to the incremented "x offset" value (ie, the amount that "1 offset" equals)
		const int baseXCoord = 35; // the base x coordinate to which the offset value is added
		
		for(int n = 0; n < yCoordsOfActiveNotes.size(); ++n)
		{
			
			const int yCoord = yCoordsOfActiveNotes.getUnchecked(n);
			const float halfTheStafflineHeight = 10.0f;
			
			if(n == yCoordsOfActiveNotes.size() - 1) {
				xOffset = 0;
			} else if(yCoordsOfActiveNotes.getUnchecked(n + 1) - yCoord > halfTheStafflineHeight) { xOffset = 0; }; // bc of the n+1 here
			
			const int xCoord = xOffset * someScaleFactor + baseXCoord;
			
			drawNotehead(xCoord, yCoord);
			
			const int atester = activePitches.getUnchecked(n) % 12;
			if(atester == 1 || atester == 3 || atester == 6 || atester == 8 || atester == 10)
			{
				drawAccidental(xCoord, yCoord);
			}
			
			xOffset ^= 1;
			
		}
	}
};


void StaffDisplay::drawNotehead(const int x, const int y)
{
	// x & y coords should be the center of the notehead.
};

void StaffDisplay::drawAccidental(const int x, const int y)
{
	// x & y coords are the center of the accidental symbol
	if(useFlats)
	{
		
	}
	else
	{
		
	}
};
