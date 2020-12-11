/*
  ==============================================================================

    HelpScreen.cpp
    Created: 10 Dec 2020 2:08:25pm
    Author:  Ben Vining

  ==============================================================================
*/

#include <JuceHeader.h>
#include "HelpScreen.h"

//==============================================================================
HelpScreen::HelpScreen()
{
	
};

HelpScreen::~HelpScreen()
{
};

void HelpScreen::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::black);
    g.drawRect (getLocalBounds(), 1);

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("Help Screen", getLocalBounds(), juce::Justification::centred, true);
};

void HelpScreen::resized()
{
    
};
