/*
    This file defines a pop-up screen that provides onboard documentation and help for users of Imogen
    When Imogen is built as a plugin, this file's direct parent is PluginEditor.h.
*/

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "BinaryData.h"


namespace bav

{

class HelpScreen  : public juce::Component
{
public:
    
    HelpScreen();
    ~HelpScreen() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;

    
private:
    
    juce::Image closeIcon;
    juce::ImageButton closeButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelpScreen)
};

}  // namespace
