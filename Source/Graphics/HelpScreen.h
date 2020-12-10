/*
  ==============================================================================

    HelpScreen.h
    Created: 10 Dec 2020 2:08:25pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"

//==============================================================================
/*
*/
class HelpScreen  : public juce::Component
{
public:
    HelpScreen();
    ~HelpScreen() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelpScreen)
};
