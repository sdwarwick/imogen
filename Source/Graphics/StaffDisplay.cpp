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
StaffDisplay::StaffDisplay(ImogenAudioProcessor& p, ImogenLookAndFeel& l): audioProcessor(p), lookAndFeel(l),  grandStaff(ImageCache::getFromMemory(BinaryData::grandStaff_png, BinaryData::grandStaff_pngSize)),
		useFlats(false), halfTheStafflineHeight(10.0f), accidentalXoffset(10)
{
	displayFlats.addItem("Display flats", 1);
	displayFlats.addItem("Display sharps", 2);
	displayFlats.setSelectedId(2);
	addAndMakeVisible(displayFlats);

	yCoordsOfActiveNotes.ensureStorageAllocated(NUMBER_OF_VOICES);
	yCoordsOfActiveNotes.clearQuick();
	
	const int noteheadHeightPx = 15;
	yCoordLookupTable[0] = 0;
	for(int n = 1; n < 128; ++n)
	{
		if(const int modulo = n % 12; modulo == 1 || modulo == 3 || modulo == 6 || modulo == 8 || modulo == 10)
		{
			yCoordLookupTable[n] = yCoordLookupTable[n - 1];
		}
		else
		{
			yCoordLookupTable[n] = yCoordLookupTable[n - 1] + noteheadHeightPx;
		}
	}
	
	staffImage.setImage(grandStaff);
	addAndMakeVisible(staffImage);
};

StaffDisplay::~StaffDisplay()
{
	setLookAndFeel(nullptr);
};

void StaffDisplay::paint (juce::Graphics& g)
{
	
	g.fillAll (lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::staffDisplayBackgroundColourId));

	drawPitches(audioProcessor.returnActivePitches(), g);
	
};

void StaffDisplay::resized()
{
	staffImage.setBounds(17, 40, 265, 197);
	displayFlats.setBounds(80, 300, 140, 35);
};


void StaffDisplay::drawPitches(Array<int> activePitches, Graphics& g)
{
	if(displayFlats.getSelectedId() == 1) { useFlats = true; } else (useFlats = false);
	
	if(activePitches.isEmpty() == false)
	{
		yCoordsOfActiveNotes.clearQuick();
		for(int n = 0; n < activePitches.size(); ++n)
		{
			if(const int returnedpitch = activePitches.getUnchecked(n); returnedpitch > -1)
			{
				yCoordsOfActiveNotes.add(yCoordLookupTable[returnedpitch]);
			}
		}
		
		if(yCoordsOfActiveNotes.isEmpty() == false)
		{
			int xOffset = 0;
			float someScaleFactor = 28.0f; // the scaleing factor applied to the incremented "x offset" value (ie, the amount that "1 offset" equals)
			const int baseXCoord = 35; // the base x coordinate to which the offset value is added
			
			for(int n = 0; n < yCoordsOfActiveNotes.size(); ++n)
			{
				
				const int yCoord = yCoordsOfActiveNotes.getUnchecked(n);
				
				if(n == yCoordsOfActiveNotes.size() - 1) {
					xOffset = 0;
				}
				if(n < yCoordsOfActiveNotes.size() - 2) {
					if(yCoordsOfActiveNotes.getUnchecked(n + 1) - yCoord > halfTheStafflineHeight) { xOffset = 0; };
				}
				
				const int xCoord = xOffset * someScaleFactor + baseXCoord;
				
				drawNotehead(xCoord, yCoord, g);
				
				if(const int atester = activePitches.getUnchecked(n) % 12; atester == 1 || atester == 3 || atester == 6 || atester == 8 || atester == 10)
				{
					drawAccidental(xCoord - accidentalXoffset, yCoord, g);
				}
				
				xOffset ^= 1;
				
			}
		}
	}
};


void StaffDisplay::drawNotehead(const int x, const int y, Graphics& g)
{
	// x & y coords are the center of the notehead.
	
	g.setColour(juce::Colours::black);
	
};


void StaffDisplay::drawAccidental(const int x, const int y, Graphics& g)
{
	g.setColour(juce::Colours::black);
	
	// x & y coords are the center of the accidental symbol
	if(useFlats)
	{
		
	}
	else
	{
		
	}
	
};
