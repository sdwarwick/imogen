/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
    Parent file: PluginEditor.h
*/


#include "../../Source/PluginSources/PluginEditor.h"

#include "BinaryData.h"


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p):
    AudioProcessorEditor (&p), audioProcessor (p),
    currentSkin(bav::ImogenLookAndFeel::Skin::CasualDenim),
    prevSkin(bav::ImogenLookAndFeel::Skin::CasualDenim),
    midiPanel(p, lookAndFeel), ioPanel(p, lookAndFeel),
    staffDisplay(p, lookAndFeel)
{
    setSize (940, 435);
    this->setBufferedToImage (true);
    
    auto splash = new juce::SplashScreen ("Welcome to Imogen!",
                                          bav::ImageCache::getFromMemory (BinaryData::imogen_icon_png, BinaryData::imogen_icon_pngSize)
                                                .rescaled (400, 400, juce::Graphics::ResamplingQuality::highResamplingQuality),
                                          true);
    
    lookAndFeel.changeSkin(currentSkin);
    
    midiPanel   .setLookAndFeel(&lookAndFeel);
    ioPanel     .setLookAndFeel(&lookAndFeel);
    staffDisplay.setLookAndFeel(&lookAndFeel);
    selectSkin  .setLookAndFeel(&lookAndFeel);
    skinLabel   .setLookAndFeel(&lookAndFeel);
    helpButton  .setLookAndFeel(&lookAndFeel);
    helpScreen  .setLookAndFeel(&lookAndFeel);
    
#if ! JUCE_STANDALONE_APPLICATION
    sidechainWarning.setLookAndFeel(&lookAndFeel);
    sidechainWarningShowing = false;
    addChildComponent(sidechainWarning);
#endif
    
    Timer::startTimerHz(60); // framerate of graphics
    
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
    
    makePresetMenu(selectPreset);
    selectPreset.onChange = [this] { newPresetSelected(); };
    
    modulatorInputSource.addItem("left",  1);
    modulatorInputSource.addItem("right", 2);
    modulatorInputSource.addItem("mix to mono", 3);
    modulatorInputSource.setSelectedId(1);
    modulatorInputSource.onChange = [this] { changeModulatorInputSource(); };
    
    addAndMakeVisible(midiPanel);
    addAndMakeVisible(ioPanel);
    addAndMakeVisible(staffDisplay);
    addAndMakeVisible(selectSkin);
    addAndMakeVisible(helpButton);
    addAndMakeVisible(skinLabel);
    //addAndMakeVisible(selectPreset);
    //addAndMakeVisible(modulatorInputSource);
    
    splash->deleteAfterDelay(juce::RelativeTime(4), true);
}

ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor() {
    setLookAndFeel(nullptr);
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
    
    //selectPreset.setBounds(x, y, w, h);
    
    //modulatorInputSource.setBounds(x, y, w, h);
    
    //sidechainWarning.setBounds(x, y, w, h);
}



void ImogenAudioProcessorEditor::timerCallback()
{
    staffDisplay.repaint();
    
#if ! JUCE_STANDALONE_APPLICATION
    if (host.isLogic() || host.isGarageBand())
    {
        const bool shouldBeShowing = audioProcessor.shouldWarnUserToEnableSidechain();
        
        if (sidechainWarningShowing != shouldBeShowing)
        {
            sidechainWarningShowing = shouldBeShowing;
            
            if(shouldBeShowing)
            {
                sidechainWarning.setVisible (true);
                sidechainWarning.repaint();
            }
            else
            {
                sidechainWarning.setVisible (false);
                this->repaint();
            }
        }
    }
#endif
}


void ImogenAudioProcessorEditor::changeModulatorInputSource()
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


void ImogenAudioProcessorEditor::skinSelectorChanged()
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


void ImogenAudioProcessorEditor::helpButtonClicked()
{
    if (! helpScreen.isVisible() )
    {
        helpScreen.setVisible(true);
        helpScreen.repaint();
    }
    else
        helpScreen.setVisible(false);
}


void ImogenAudioProcessorEditor::updateNumVoicesCombobox (const int newNumVoices)
{
    midiPanel.updateNumVoicesCombobox(newNumVoices);
}


void ImogenAudioProcessorEditor::newPresetSelected()
{
    audioProcessor.loadPreset(selectPreset.getItemText(selectPreset.getSelectedId()));
}


void ImogenAudioProcessorEditor::makePresetMenu (juce::ComboBox& box)
{
    int id = 1;
    
    for (juce::DirectoryEntry entry : juce::RangedDirectoryIterator(audioProcessor.getPresetsFolder(), false))
    {
        box.addItem(entry.getFile().getFileName(), id);
        ++id;
    }
}
