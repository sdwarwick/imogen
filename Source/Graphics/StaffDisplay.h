/*
  ==============================================================================

    StaffDisplay.h
    Created: 26 Nov 2020 11:27:15pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"

//==============================================================================
/*
*/
class StaffDisplay  : public juce::Component
{
public:
    StaffDisplay();
    ~StaffDisplay() override;

    void paint (juce::Graphics&) override;
    void resized() override;
	
	void displayFlats(const bool displayingFlats);

private:
	
	Array<int> yCoordsOfActiveNotes;
	int yCoordLookupTable[128];
	bool useFlats;
	
	void drawPitches(Array<int> activePitches);
	void drawNotehead(const int x, const int y);
	void drawAccidental(const int x, const int y);
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StaffDisplay)
};
