/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
    Parent file: PluginEditor.h
*/


#include "PluginEditor.h"
#include "BinaryData.h"


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p):
    AudioProcessorEditor (&p), audioProcessor (p),
    midiPanel(p, lookAndFeel), ioPanel(p, lookAndFeel),
    currentSkin(bav::ImogenLookAndFeel::Skin::CasualDenim),
    prevSkin(currentSkin)
{
    this->setBufferedToImage (true);
    
    lookAndFeel.changeSkin(currentSkin);
    
    midiPanel   .setLookAndFeel(&lookAndFeel);
    ioPanel     .setLookAndFeel(&lookAndFeel);
    selectSkin  .setLookAndFeel(&lookAndFeel);
    skinLabel   .setLookAndFeel(&lookAndFeel);
    
    selectSkin.addItem("Casual Denim", 1);
    selectSkin.addItem("Playful Boho", 2);
    selectSkin.addItem("Chic Eveningwear", 3);
    selectSkin.setSelectedId(1);
    selectSkin.onChange = [this] { skinSelectorChanged(); };
    lookAndFeel.initializeLabel(skinLabel, "Select skin");
    
    makePresetMenu(selectPreset);
    selectPreset.onChange = [this] { newPresetSelected(); };
    
    modulatorInputSource.addItem("left",  1);
    modulatorInputSource.addItem("right", 2);
    modulatorInputSource.addItem("mix to mono", 3);
    //modulatorInputSource.onChange = [this] { changeModulatorInputSource(); };
    modulatorInputSource.setSelectedId (1, juce::NotificationType::dontSendNotification);
    
    addAndMakeVisible(midiPanel);
    addAndMakeVisible(ioPanel);
    addAndMakeVisible(selectSkin);
    addAndMakeVisible(skinLabel);
    //addAndMakeVisible(selectPreset);
    //addAndMakeVisible(modulatorInputSource);
    
    setSize (940, 435);
    
    Timer::startTimerHz(60); // framerate of graphics
}


ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor()
{
    this->setLookAndFeel(nullptr);
    Timer::stopTimer();
}


void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (lookAndFeel.findColour(bav::ImogenLookAndFeel::uiColourIds::blankCanvasColourId));
}


void ImogenAudioProcessorEditor::resized()
{
    midiPanel   .setBounds(10, 10, 300, 415);
    ioPanel     .setBounds(320, 10, 300, 415);
    
    selectSkin  .setBounds(775, 388, 150, 30);
    skinLabel   .setBounds(775, 365, 150, 25);
    
    //selectPreset.setBounds(x, y, w, h);
    
    //modulatorInputSource.setBounds(x, y, w, h);
}



void ImogenAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.hasUpdatedParamDefaults())
        updateParameterDefaults();
}


inline void ImogenAudioProcessorEditor::changeModulatorInputSource()
{
//    switch (modulatorInputSource.getSelectedId())
//    {
//        case 1:
//            audioProcessor.changeModulatorInputSource(ImogenAudioProcessor::ModulatorInputSource::left);
//            break;
//        case 2:
//            audioProcessor.changeModulatorInputSource(ImogenAudioProcessor::ModulatorInputSource::right);
//            break;
//        case 3:
//            audioProcessor.changeModulatorInputSource(ImogenAudioProcessor::ModulatorInputSource::mixToMono);
//            break;
//        default:
//            return;
//    }
}


inline void ImogenAudioProcessorEditor::skinSelectorChanged()
{
    switch (selectSkin.getSelectedId())
    {
        case(1):
            currentSkin = bav::ImogenLookAndFeel::Skin::CasualDenim;
            break;
        case(2):
            currentSkin = bav::ImogenLookAndFeel::Skin::PlayfulBoho;
            break;
        case(3):
            currentSkin = bav::ImogenLookAndFeel::Skin::ChicEveningwear;
            break;
    }
    
    if (currentSkin != prevSkin)
    {
        lookAndFeel.changeSkin(currentSkin);
        prevSkin = currentSkin;
        this->repaint();
    }
}


void ImogenAudioProcessorEditor::updateNumVoicesCombobox (const int newNumVoices)
{
    midiPanel.updateNumVoicesCombobox(newNumVoices);
}


inline void ImogenAudioProcessorEditor::newPresetSelected()
{
    auto preset = selectPreset.getItemText (selectPreset.getSelectedId());
    
    audioProcessor.loadPreset (preset);
}


inline void ImogenAudioProcessorEditor::makePresetMenu (juce::ComboBox& box)
{
    int id = 1;
    
    for (juce::DirectoryEntry entry : juce::RangedDirectoryIterator(audioProcessor.getPresetsFolder(), false))
    {
        box.addItem(entry.getFile().getFileName(), id);
        ++id;
    }
}


void ImogenAudioProcessorEditor::updateParameterDefaults()
{
    ioPanel.updateParameterDefaults();
    midiPanel.updateParameterDefaults();
}
