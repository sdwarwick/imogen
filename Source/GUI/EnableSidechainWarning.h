/*
    This file defines a pop-up screen that warns the user that, if they're using Logic or Garageband, sidechain audio input must be enabled for the plugin to function correctly.
    This file is only needed when Imogen is built as a plugin.
    This file's direct parent is PluginEditor.h.
*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>


class EnableSidechainWarning  : public juce::Component
{
public:
    EnableSidechainWarning()
    {

    }

    ~EnableSidechainWarning() override
    {
        setLookAndFeel(nullptr);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

        g.setColour (juce::Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (juce::Colours::white);
        g.setFont (14.0f);
        g.drawText ("EnableSidechainWarning", getLocalBounds(),
                    juce::Justification::centred, true);   // draw some placeholder text
    }

    void resized() override
    {

    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnableSidechainWarning)
};
