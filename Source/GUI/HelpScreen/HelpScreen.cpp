/*
    This file defines a pop-up screen that provides onboard documentation and help for users of Imogen
    Parent file: HelpScreen.h.
*/

#include "HelpScreen.h"


namespace bav

{

HelpScreen::HelpScreen():
closeIcon(juce::ImageCache::getFromMemory(BinaryData::closeIcon_png, BinaryData::closeIcon_pngSize))
{
    // close button
    {
        closeButton.setImages(true, true, true, closeIcon, 1.0f, juce::Colours::transparentBlack, closeIcon, 1.0f, juce::Colours::transparentWhite, closeIcon, 1.0f, juce::Colours::transparentWhite);
        addAndMakeVisible(closeButton);
        closeButton.onClick = [this] { this->setVisible(false); };
    }
};

HelpScreen::~HelpScreen()
{
    setLookAndFeel(nullptr);
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
    closeButton.setBounds(5, 5, 25, 25);
};


};  // namespace
