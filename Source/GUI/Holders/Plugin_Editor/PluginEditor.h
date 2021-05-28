
#pragma once

#include "DSP/PluginProcessor/PluginProcessor.h"

#include "ImogenGUI/ImogenGUI.h"


using namespace Imogen;


class ImogenAudioProcessorEditor : public bav::gui::EditorBase
{
public:
    ImogenAudioProcessorEditor (ImogenAudioProcessor& p);

    ~ImogenAudioProcessorEditor() override;

    /*=========================================================================================*/

private:
    void paint (juce::Graphics&) override final;
    
    void resizeTriggered() override final;
    
    ImogenGUI gui;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenAudioProcessorEditor)
};
