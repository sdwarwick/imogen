/*
  ==============================================================================

    StaffDisplay.h
    Created: 26 Nov 2020 11:27:15pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StaffDisplay)
};
