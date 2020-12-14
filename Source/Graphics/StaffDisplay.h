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
#include "LookAndFeel.h"

//==============================================================================
/*
*/
class StaffDisplay  : public juce::Component
{
public:
	ImogenAudioProcessor& audioProcessor;
	ImogenLookAndFeel& lookAndFeel;
	StaffDisplay(ImogenAudioProcessor& p, ImogenLookAndFeel& l);
    ~StaffDisplay() override;

    void paint (juce::Graphics&) override;
    void resized() override;
	
	ComboBox displayFlats;

private:
	
	Array<int> currentlyActive;
	
	Image grandStaff;
	ImageComponent staffImage;
	
	Array<int> yCoordsOfActiveNotes;
	Array<int> prevActivePitches;
	int yCoordLookupTable[128];
	bool useFlats;
	
	void drawPitches(Array<int> activePitches, Graphics& g);
	void drawNotehead(const int x, const int y, Graphics& g);
	void drawAccidental(const int x, const int y, Graphics& g);
	
	const float halfTheStafflineHeight;
	const int accidentalXoffset;
	
	String noteheadSvg;
	Path noteheadPath;
	//String flatSvg;
	//Path flatPath;
	//String sharpSvg;
	//Path sharpPath;
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StaffDisplay)
};
