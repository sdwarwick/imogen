#include "PluginEditor.h"

//==============================================================================
ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p):
    AudioProcessorEditor (&p), audioProcessor (p), currentSkin(ImogenLookAndFeel::Skin::CasualDenim), prevSkin(ImogenLookAndFeel::Skin::CasualDenim), midiPanel(p, lookAndFeel), ioPanel(p, lookAndFeel), staffDisplay(p, lookAndFeel), viewHelp(false)
{
    setSize (940, 435);
    
    lookAndFeel.changeSkin(currentSkin);
    
    midiPanel   .setLookAndFeel(&lookAndFeel);
    ioPanel     .setLookAndFeel(&lookAndFeel);
    staffDisplay.setLookAndFeel(&lookAndFeel);
    selectSkin  .setLookAndFeel(&lookAndFeel);
    skinLabel   .setLookAndFeel(&lookAndFeel);
    helpButton  .setLookAndFeel(&lookAndFeel);
    helpScreen  .setLookAndFeel(&lookAndFeel);
    
    Timer::startTimerHz(FRAMERATE);
    
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
    
    addAndMakeVisible(midiPanel);
    addAndMakeVisible(ioPanel);
    addAndMakeVisible(staffDisplay);
    addAndMakeVisible(selectSkin);
    addAndMakeVisible(helpButton);
    addAndMakeVisible(skinLabel);
    //addAndMakeVisible(selectPreset);
    
};

ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor() {
    setLookAndFeel(nullptr);
    Timer::stopTimer();
};

//==============================================================================
void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::blankCanvasColourId));
    
};


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
};



void ImogenAudioProcessorEditor::timerCallback()
{
    staffDisplay.repaint();
};


void ImogenAudioProcessorEditor::skinSelectorChanged()
{
    switch(selectSkin.getSelectedId())
    {
        case(1):
            currentSkin = ImogenLookAndFeel::Skin::CasualDenim;
            break;
        case(2):
            currentSkin = ImogenLookAndFeel::Skin::PlayfulBoho;
            break;
        case(3):
            currentSkin = ImogenLookAndFeel::Skin::ChicEveningwear;
            break;
        case(4):
            currentSkin = ImogenLookAndFeel::Skin::design4;
            break;
        default:
            return;
    }
    
    if(currentSkin != prevSkin)
    {
        lookAndFeel.changeSkin(currentSkin);
        prevSkin = currentSkin;
        this->repaint();
    }
};


void ImogenAudioProcessorEditor::helpButtonClicked()
{
    if(! helpScreen.isVisible() )
    {
        helpScreen.setVisible(true);
        helpScreen.repaint();
    }
    else
        helpScreen.setVisible(false);
};


void ImogenAudioProcessorEditor::updateNumVoicesCombobox(const int newNumVoices)
{
    midiPanel.updateNumVoicesCombobox(newNumVoices);
};


void ImogenAudioProcessorEditor::newPresetSelected()
{
    audioProcessor.loadPreset(selectPreset.getItemText(selectPreset.getSelectedId()));
};


void ImogenAudioProcessorEditor::makePresetMenu(ComboBox& box)
{
    int id = 1;
    for(DirectoryEntry entry : RangedDirectoryIterator(audioProcessor.getPresetsFolder(), false))
    {
        box.addItem(entry.getFile().getFileName(), id);
        ++id;
    }
};
