/*
    This file defines a user interface control panel containing controls linked to the audio processor's various I/O controls, options, and functions.
    Parent file: IOControlPanel.h.
*/

#include "../../Source/GUI/IOControlPanel/IOControlPanel.h"


namespace bav

{
    
    
IOControlPanel::IOControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l):
    audioProcessor(p), lookAndFeel(l),
    dryPanLink      (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "dryPan", dryPan)),
    inputGainLink   (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "inputGain", inputGain)),
    masterDryWetLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "masterDryWet", masterDryWet)),
    dryGainLink     (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "dryGain", dryGain)),
    wetGainLink     (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "wetGain", wetGain)),
    outputGainLink  (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "outputGain", outputGain)),
    limiterPanel(p, l)
{
    // dry pan
    {
        dryPan.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        dryPan.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
        addAndMakeVisible(dryPan);
        dryPan.setValue(audioProcessor.dryPan->get());
        dryPan.onValueChange = [this] { audioProcessor.updateDryVoxPan(); };
        lookAndFeel.initializeLabel(drypanLabel, "Modulator pan");
        addAndMakeVisible(drypanLabel);
    }
    
    // master dry/wet
    {
        masterDryWet.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        masterDryWet.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
        addAndMakeVisible(masterDryWet);
        masterDryWet.setValue(audioProcessor.dryWet->get());
        masterDryWet.onValueChange = [this] { audioProcessor.updateDryWet(); };
        lookAndFeel.initializeLabel(drywetLabel, "% wet signal");
        addAndMakeVisible(drywetLabel);
    }
    
    // input gain
    {
        inputGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
        inputGain.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
        addAndMakeVisible(inputGain);
        inputGain.setValue(audioProcessor.inputGain->get());
        inputGain.onValueChange = [this] { audioProcessor.updateGains(); };
        lookAndFeel.initializeLabel(inputGainLabel, "Input gain");
        addAndMakeVisible(inputGainLabel);
    }
    
    // dry gain
    {
        dryGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
        dryGain.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
        //addAndMakeVisible(dryGain);
        dryGain.setValue(audioProcessor.dryGain->get());
        dryGain.onValueChange = [this] { audioProcessor.updateGains(); };
        lookAndFeel.initializeLabel(dryGainLabel, "Dry gain");
        //addAndMakeVisible(dryGainLabel);
    }
    
    // wet gain
    {
        wetGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
        wetGain.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
        //addAndMakeVisible(wetGain);
        wetGain.setValue(audioProcessor.wetGain->get());
        wetGain.onValueChange = [this] { audioProcessor.updateGains(); };
        lookAndFeel.initializeLabel(wetGainLabel, "Wet gain");
        //addAndMakeVisible(wetGainLabel);
    }
    
    // output gain
    {
        outputGain.setSliderStyle(Slider::SliderStyle::LinearVertical);
        outputGain.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        addAndMakeVisible(outputGain);
        outputGain.setValue(audioProcessor.outputGain->get());
        outputGain.onValueChange = [this] { audioProcessor.updateGains(); };
        lookAndFeel.initializeLabel(outputgainLabel, "Output gain");
        addAndMakeVisible(outputgainLabel);
    }
    
    addAndMakeVisible(limiterPanel);
};

IOControlPanel::~IOControlPanel()
{
    setLookAndFeel(nullptr);
};

void IOControlPanel::paint (juce::Graphics& g)
{
    g.fillAll (lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::backgroundPanelColourId));
    
    g.setColour(lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::insetPanelColourId));
    
    juce::Rectangle<int> inputControlPanel (5, 5, 290, 125);
    g.fillRect(inputControlPanel);
    
    juce::Rectangle<int> outputControlPanel (5, 135, 290, 125);
    g.fillRect(outputControlPanel);
};

void IOControlPanel::resized()
{
    // input gain
    {
        inputGainLabel.setBounds(10, 0, 75, 35);
        inputGain.setBounds		(22, 25, 50, 100);
    }
    
    // dry pan
    {
        drypanLabel.setBounds(200, 0, 90, 35);
        dryPan.setBounds	 (210, 25, 75, 75);
    }
    
    // master dry/wet { % wet signal }
    {
        drywetLabel.setBounds (50, 138, 75, 35);
        masterDryWet.setBounds(50, 163, 75, 75);
    }
    
    // dry gain
    {
        //dryGain.setBounds(x, y, w, h);
        //dryGainLabel.setBounds(x, y, w, h);
    }
    
    // wet gain
    {
        //wetGain.setBounds(x, y, w, h);
        //wetGainLabel.setBounds(x, y, w, h);
    }
    
    // output gain
    {
        outputgainLabel.setBounds(165, 138, 75, 35);
        outputGain.setBounds	 (177, 160, 50, 90);
    }
    
    limiterPanel.setBounds(5, 265, 290, 145);
};


};  // namespace
