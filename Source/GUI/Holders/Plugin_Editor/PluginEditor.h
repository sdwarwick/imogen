
#pragma once

#include "DSP/PluginProcessor/PluginProcessor.h"

#include "ImogenGUI/ImogenGUI.h"


using namespace Imogen;


class ImogenAudioProcessorEditor :  public juce::AudioProcessorEditor
{
public:
    ImogenAudioProcessorEditor (ImogenAudioProcessor& p);

    ~ImogenAudioProcessorEditor() override;

    /*=========================================================================================*/
    /* juce::Component functions */

    void paint (juce::Graphics&) override final;
    void resized() override final;

    /*=========================================================================================*/

private:
    ImogenAudioProcessor& imgnProcessor; // reference to the processor that created this editor

    ImogenGUI gui;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
