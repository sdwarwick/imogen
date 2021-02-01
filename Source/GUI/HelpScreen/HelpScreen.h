/*
 ==============================================================================
 
 HelpScreen.h
 Created: 10 Dec 2020 2:08:25pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>


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
    
    juce::Image closeIcon;
    juce::ImageButton closeButton;
    
private:
    
    void closeButtonClicked();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelpScreen)
};
