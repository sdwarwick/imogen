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
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class StaffDisplay  : public juce::Component
{
public:
	ImogenAudioProcessor& audioProcessor;
    StaffDisplay(ImogenAudioProcessor& p);
    ~StaffDisplay() override;

    void paint (juce::Graphics&) override;
    void resized() override;
	
	ComboBox displayFlats;

private:
	
	Array<int> yCoordsOfActiveNotes;
	int yCoordLookupTable[128];
	bool useFlats;
	
	void drawPitches(Array<int> activePitches, Graphics& g);
	void drawNotehead(const int x, const int y, Graphics& g);
	void drawAccidental(const int x, const int y, Graphics& g);
	
	const float halfTheStafflineHeight;
	const int accidentalXoffset;
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StaffDisplay)
};
