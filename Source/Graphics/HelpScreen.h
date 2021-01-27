/*
 ==============================================================================
 
 HelpScreen.h
 Created: 10 Dec 2020 2:08:25pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>


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
    
    Image closeIcon;
    ImageButton closeButton;
    
private:
    
    void closeButtonClicked();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelpScreen)
};
