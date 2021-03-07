/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
    Parent file: PluginEditor.h
*/


#include "PluginEditor.h"
#include "BinaryData.h"


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p):
    AudioProcessorEditor (&p), audioProcessor (p),
    midiPanel(p, lookAndFeel), ioPanel(p, lookAndFeel),
    staffDisplay(p, lookAndFeel),
    currentSkin(bav::ImogenLookAndFeel::Skin::CasualDenim),
    prevSkin(currentSkin)
{
    this->setBufferedToImage (true);
    
    lookAndFeel.changeSkin(currentSkin);
    
    midiPanel   .setLookAndFeel(&lookAndFeel);
    ioPanel     .setLookAndFeel(&lookAndFeel);
    staffDisplay.setLookAndFeel(&lookAndFeel);
    selectSkin  .setLookAndFeel(&lookAndFeel);
    skinLabel   .setLookAndFeel(&lookAndFeel);
    helpButton  .setLookAndFeel(&lookAndFeel);
    helpScreen  .setLookAndFeel(&lookAndFeel);
    touchOnceSettingsButton.setLookAndFeel(&lookAndFeel);
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    sidechainWarning.setLookAndFeel(&lookAndFeel);
    sidechainWarningShowing = false;
    addChildComponent(sidechainWarning);
#endif
    
    selectSkin.addItem("Casual Denim", 1);
    selectSkin.addItem("Playful Boho", 2);
    selectSkin.addItem("Chic Eveningwear", 3);
    //selectSkin.addItem("design4", 4);
    selectSkin.setSelectedId(1);
    selectSkin.onChange = [this] { skinSelectorChanged(); };
    lookAndFeel.initializeLabel(skinLabel, "Select skin");
    
    helpButton.setButtonText("Help");
    helpButton.onClick = [this] { helpButtonClicked(); };
    addChildComponent(helpScreen);
    
    touchOnceSettingsButton.setButtonText("Advanced preferences");
    touchOnceSettingsButton.onClick = [this] { touchOnceSettingsButtonClicked(); };
    addChildComponent (touchOnceSettings);
    
    makePresetMenu(selectPreset);
    selectPreset.onChange = [this] { newPresetSelected(); };
    
    modulatorInputSource.addItem("left",  1);
    modulatorInputSource.addItem("right", 2);
    modulatorInputSource.addItem("mix to mono", 3);
    modulatorInputSource.onChange = [this] { changeModulatorInputSource(); };
    modulatorInputSource.setSelectedId (1, juce::NotificationType::dontSendNotification);
    
    addAndMakeVisible(midiPanel);
    addAndMakeVisible(ioPanel);
    addAndMakeVisible(staffDisplay);
    addAndMakeVisible(selectSkin);
    addAndMakeVisible(helpButton);
    //addAndMakeVisible(touchOnceSettingsButton);
    addAndMakeVisible(skinLabel);
    //addAndMakeVisible(selectPreset);
    //addAndMakeVisible(modulatorInputSource);
    
    // addChildComponent(touchOnceSettings);
    
    setSize (940, 435);
    
    Timer::startTimerHz(60); // framerate of graphics
}


ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor()
{
    this->setBufferedToImage (false);
    this->setLookAndFeel(nullptr);
    helpScreen.setLookAndFeel(nullptr);
    touchOnceSettings.setLookAndFeel(nullptr);
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
    staffDisplay.setBounds(630, 10, 300, 350);
    
    selectSkin  .setBounds(775, 388, 150, 30);
    skinLabel   .setBounds(775, 365, 150, 25);
    helpButton  .setBounds(685, 388, 75, 30);
    helpScreen  .setBounds(158, 45, 625, 315);
    //touchOnceSettingsButton.setBounds(x, y, 150, 30);
    touchOnceSettings.setBounds (158, 45, 625, 315);
    
    //selectPreset.setBounds(x, y, w, h);
    
    //modulatorInputSource.setBounds(x, y, w, h);
    
    //sidechainWarning.setBounds(x, y, w, h);
}



void ImogenAudioProcessorEditor::timerCallback()
{
    //staffDisplay.repaint();
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    if (! juce::JUCEApplicationBase::isStandaloneApp())
    {
        const bool shouldBeShowing = audioProcessor.shouldWarnUserToEnableSidechain();
        
        if (sidechainWarningShowing != shouldBeShowing)
        {
            sidechainWarning.setVisible (shouldBeShowing);
            
            if (shouldBeShowing)
                sidechainWarning.repaint();
            else
                this->repaint();
            
            sidechainWarningShowing = shouldBeShowing;
        }
    }
#endif
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
        case(4):
            currentSkin = bav::ImogenLookAndFeel::Skin::design4;
            break;
        default:
            return;
    }
    
    if (currentSkin != prevSkin)
    {
        lookAndFeel.changeSkin(currentSkin);
        prevSkin = currentSkin;
        this->repaint();
    }
}


inline void ImogenAudioProcessorEditor::helpButtonClicked()
{
    if (! helpScreen.isVisible() )
    {
        helpScreen.setVisible(true);
        helpScreen.repaint();
    }
    else
        helpScreen.setVisible(false);
}


inline void ImogenAudioProcessorEditor::touchOnceSettingsButtonClicked()
{
    if (! touchOnceSettings.isVisible() )
    {
        touchOnceSettings.setVisible(true);
        touchOnceSettings.repaint();
    }
    else
        touchOnceSettings.setVisible(false);
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
