/*
    This file defines a user interface control panel containing controls linked to the audio processor's various I/O controls, options, and functions.
    Parent file: IOControlPanel.h.
*/

#include "IOControlPanel.h"

#undef bvi_ROTARY_SLIDER
#undef bvi_LINEAR_SLIDER

namespace bav

{
    
#define bvi_ROTARY_SLIDER Slider::SliderStyle::RotaryVerticalDrag
#define bvi_LINEAR_SLIDER Slider::SliderStyle::LinearVertical
    
IOControlPanel::IOControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l):
    audioProcessor(p), lookAndFeel(l),
    dryPanLink      (std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "dryPan", dryPan)),
    inputGainLink   (std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "inputGain", inputGain)),
    masterDryWetLink(std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "masterDryWet", masterDryWet)),
    dryGainLink     (std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "dryGain", dryGain)),
    wetGainLink     (std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "wetGain", wetGain)),
    outputGainLink  (std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "outputGain", outputGain)),
    limiterPanel(p, l)
{
    lookAndFeel.initializeSlider (dryPan, bvi_ROTARY_SLIDER, audioProcessor.getDryPan());
    addAndMakeVisible(dryPan);
    lookAndFeel.initializeLabel(drypanLabel, "Modulator pan");
    addAndMakeVisible(drypanLabel);

    lookAndFeel.initializeSlider (masterDryWet, bvi_ROTARY_SLIDER, audioProcessor.getDryWet());
    addAndMakeVisible(masterDryWet);
    lookAndFeel.initializeLabel(drywetLabel, "% wet signal");
    addAndMakeVisible(drywetLabel);

    lookAndFeel.initializeSlider (inputGain, bvi_LINEAR_SLIDER, audioProcessor.getInputGain());
    addAndMakeVisible(inputGain);
    lookAndFeel.initializeLabel(inputGainLabel, "Input gain");
    addAndMakeVisible(inputGainLabel);

    lookAndFeel.initializeSlider (dryGain, bvi_LINEAR_SLIDER, audioProcessor.getDryGain());
    //addAndMakeVisible(dryGain);
    lookAndFeel.initializeLabel(dryGainLabel, "Dry gain");
    //addAndMakeVisible(dryGainLabel);

    lookAndFeel.initializeSlider (wetGain, bvi_LINEAR_SLIDER, audioProcessor.getWetGain());
    //addAndMakeVisible(wetGain);
    lookAndFeel.initializeLabel(wetGainLabel, "Wet gain");
    //addAndMakeVisible(wetGainLabel);

    lookAndFeel.initializeSlider(outputGain, bvi_LINEAR_SLIDER, audioProcessor.getOutputGain());
    addAndMakeVisible(outputGain);
    lookAndFeel.initializeLabel(outputgainLabel, "Output gain");
    addAndMakeVisible(outputgainLabel);

    addAndMakeVisible(limiterPanel);
}
    
#undef bvi_ROTARY_SLIDER
#undef bvi_LINEAR_SLIDER
    

IOControlPanel::~IOControlPanel()
{
    this->setLookAndFeel(nullptr);
}
    
    
void IOControlPanel::paint (juce::Graphics& g)
{
    g.fillAll (lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::backgroundPanelColourId));
    
    g.setColour(lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::insetPanelColourId));
    
    juce::Rectangle<int> inputControlPanel (5, 5, 290, 125);
    g.fillRect(inputControlPanel);
    
    juce::Rectangle<int> outputControlPanel (5, 135, 290, 125);
    g.fillRect(outputControlPanel);
}

void IOControlPanel::resized()
{
    inputGainLabel.setBounds(10, 0, 75, 35);
    inputGain.setBounds		(22, 25, 50, 100);

    drypanLabel.setBounds(200, 0, 90, 35);
    dryPan.setBounds	 (210, 25, 75, 75);

    drywetLabel.setBounds (50, 138, 75, 35);
    masterDryWet.setBounds(50, 163, 75, 75);

    //dryGain.setBounds(x, y, w, h);
    //dryGainLabel.setBounds(x, y, w, h);

    //wetGain.setBounds(x, y, w, h);
    //wetGainLabel.setBounds(x, y, w, h);

    outputgainLabel.setBounds(165, 138, 75, 35);
    outputGain.setBounds	 (177, 160, 50, 90);

    limiterPanel.setBounds(5, 265, 290, 145);
}


}  // namespace
