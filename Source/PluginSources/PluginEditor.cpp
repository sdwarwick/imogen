/*
    This file defines the interactive GUI widow generated & linked to the audio processor when Imogen is built as a plugin
    Parent file: PluginEditor.h
*/


#include "PluginEditor.h"
#include "bv_SharedCode/misc/BinaryDataHelpers.h"


#define bvi_GRAPHICS_FRAMERATE_HZ 60


ImogenAudioProcessorEditor::ImogenAudioProcessorEditor (ImogenAudioProcessor& p):
    AudioProcessorEditor (&p), imgnProcessor(p), params(p)
{
    this->setBufferedToImage (true);
    
    makePresetMenu(selectPreset);
    selectPreset.onChange = [this] { newPresetSelected(); };
    
    //addAndMakeVisible(selectPreset);
    
    setSize (940, 435);
    
    Timer::startTimerHz (bvi_GRAPHICS_FRAMERATE_HZ);
}

#undef bvi_GRAPHICS_FRAMERATE_HZ


ImogenAudioProcessorEditor::~ImogenAudioProcessorEditor()
{
    this->setLookAndFeel(nullptr);
    Timer::stopTimer();
}


void ImogenAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}


void ImogenAudioProcessorEditor::resized()
{
    //selectPreset.setBounds(x, y, w, h);
}



void ImogenAudioProcessorEditor::timerCallback()
{
    if (imgnProcessor.hasUpdatedParamDefaults())
        updateParameterDefaults();
    
    imgnProcessor.paramChangesForEditor.getReadyMessages (currentMessages, true);
    
    for (const auto msg : currentMessages)
    {
        if (! msg.isValid())
            continue;
        
        switch (msg.type())
        {
            case (ids::leadBypassID):           continue;
            case (ids::harmonyBypassID):        continue;
            case (ids::dryPanID):               continue;
            case (ids::dryWetID):               continue;
            case (ids::adsrAttackID):           continue;
            case (ids::adsrDecayID):            continue;
            case (ids::adsrSustainID):          continue;
            case (ids::adsrReleaseID):          continue;
            case (ids::adsrToggleID):           continue;
            case (ids::stereoWidthID):          continue;
            case (ids::lowestPannedID):         continue;
            case (ids::velocitySensID):         continue;
            case (ids::pitchBendRangeID):       continue;
            case (ids::pedalPitchIsOnID):       continue;
            case (ids::pedalPitchThreshID):     continue;
            case (ids::pedalPitchIntervalID):   continue;
            case (ids::descantIsOnID):          continue;
            case (ids::descantThreshID):        continue;
            case (ids::descantIntervalID):      continue;
            case (ids::concertPitchHzID):       continue;
            case (ids::voiceStealingID):        continue;
            case (ids::inputGainID):            continue;
            case (ids::outputGainID):           continue;
            case (ids::limiterToggleID):        continue;
            case (ids::noiseGateToggleID):      continue;
            case (ids::noiseGateThresholdID):   continue;
            case (ids::compressorToggleID):     continue;
            case (ids::compressorAmountID):     continue;
            case (ids::vocalRangeTypeID):       continue;
            case (ids::aftertouchGainToggleID): continue;
            case (ids::deEsserToggleID):        continue;
            case (ids::deEsserThreshID):        continue;
            case (ids::deEsserAmountID):        continue;
            case (ids::reverbToggleID):         continue;
            case (ids::reverbDryWetID):         continue;
            case (ids::reverbDecayID):          continue;
            case (ids::reverbDuckID):           continue;
            case (ids::reverbLoCutID):          continue;
            case (ids::reverbHiCutID):          continue;
            case (ids::inputSourceID):          continue;
            case (ids::numVoicesID):            continue;
        }
    }
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    const bool shouldWarn = imgnProcessor.shouldWarnUserToEnableSidechain();
    juce::ignoreUnused (shouldWarn);
#endif
}


inline void ImogenAudioProcessorEditor::newPresetSelected()
{
    imgnProcessor.loadPreset (selectPreset.getItemText (selectPreset.getSelectedId()));
}


inline void ImogenAudioProcessorEditor::makePresetMenu (juce::ComboBox& box)
{
    int id = 1;
    
    for  (juce::DirectoryEntry entry  :   juce::RangedDirectoryIterator (imgnProcessor.getPresetsFolder(), false))
    {
        box.addItem (entry.getFile().getFileName(), id);
        ++id;
    }
}


void ImogenAudioProcessorEditor::updateParameterDefaults()
{
    
}
