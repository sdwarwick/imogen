/*
    This file defines a user interface control panel containing controls linked to the harmonizer's various MIDI settings, controls, and functions.
    Parent file: MidiControlPanel.h.
*/


#include "MidiControlPanel.h"


#undef bvi_ROTARY_SLIDER
#undef bvi_LINEAR_SLIDER


namespace bav

{
    
#define bvi_ROTARY_SLIDER Slider::SliderStyle::RotaryVerticalDrag
#define bvi_LINEAR_SLIDER Slider::SliderStyle::LinearBarVertical
    
    
MidiControlPanel::MidiControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l):
    audioProcessor(p), lookAndFeel(l),
    attackLink (std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "adsrAttack", adsrAttack)),
    decayLink  (std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "adsrDecay", adsrDecay)),
    sustainLink(std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "adsrSustain", adsrSustain)),
    releaseLink(std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "adsrRelease", adsrRelease)),
    adsrOnOffLink  (std::make_unique<APVTS::ButtonAttachment>(audioProcessor.tree, "adsrOnOff", adsrOnOff)),
    stereoWidthLink(std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "stereoWidth", stereoWidth)),
    lowestPanLink  (std::make_unique<APVTS::SliderAttachment>(audioProcessor.tree, "lowestPan", lowestPan)),
    midiVelocitySensLink(std::make_unique<APVTS::SliderAttachment> (audioProcessor.tree, "midiVelocitySensitivity", midiVelocitySens)),
    pitchBendUpLink  (std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.tree, "PitchBendUpRange", pitchBendUp)),
    pitchBendDownLink(std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.tree, "PitchBendDownRange", pitchBendDown)),
    voiceStealingLink(std::make_unique<APVTS::ButtonAttachment>(audioProcessor.tree, "voiceStealing", voiceStealing)),
    quickKillMsLink  (std::make_unique<APVTS::SliderAttachment>(audioProcessor.tree, "quickKillMs", quickKillMs)),
    concertPitchLink (std::make_unique<APVTS::SliderAttachment>(audioProcessor.tree, "concertPitch", concertPitch)),
    pedalPitchToggleLink(std::make_unique<APVTS::ButtonAttachment>(audioProcessor.tree, "pedalPitchToggle", pedalPitchToggle)),
    pedalPitchThreshLink(std::make_unique<APVTS::SliderAttachment>(audioProcessor.tree, "pedalPitchThresh", pedalPitchThreshold)),
    pedalPitchIntervalLink(std::make_unique<APVTS::ComboBoxAttachment>(audioProcessor.tree, "pedalPitchInterval", pedalPitchInterval)),
    descantToggleLink   (std::make_unique<APVTS::ButtonAttachment>(audioProcessor.tree, "descantToggle", descantToggle)),
    descantThresholdLink(std::make_unique<APVTS::SliderAttachment>(audioProcessor.tree, "descantThresh", descantThreshold)),
    descantIntervalLink (std::make_unique<APVTS::ComboBoxAttachment>(audioProcessor.tree, "descantInterval", descantInterval))
{
    lookAndFeel.initializeSlider (adsrAttack, bvi_ROTARY_SLIDER, audioProcessor.getAdsrAttack(), audioProcessor.getDefaultAdsrAttack(), true);
    addAndMakeVisible(adsrAttack);
    lookAndFeel.initializeLabel(attackLabel, "Attack");
    addAndMakeVisible(attackLabel);
    
    lookAndFeel.initializeSlider (adsrDecay, bvi_ROTARY_SLIDER, audioProcessor.getAdsrDecay(), audioProcessor.getDefaultAdsrDecay(), true);
    addAndMakeVisible(adsrDecay);
    lookAndFeel.initializeLabel(decayLabel, "Decay");
    addAndMakeVisible(decayLabel);
    
    lookAndFeel.initializeSlider (adsrSustain, bvi_ROTARY_SLIDER, audioProcessor.getAdsrSustain(), audioProcessor.getDefaultAdsrSustain(), true);
    addAndMakeVisible(adsrSustain);
    lookAndFeel.initializeLabel(sustainLabel, "Sustain");
    addAndMakeVisible(sustainLabel);
    
    lookAndFeel.initializeSlider (adsrRelease, bvi_ROTARY_SLIDER, audioProcessor.getAdsrRelease(), audioProcessor.getDefaultAdsrRelease(), true);
    addAndMakeVisible(adsrRelease);
    lookAndFeel.initializeLabel(releaseLabel, "Release");
    addAndMakeVisible(releaseLabel);
    
    adsrOnOff.setButtonText("MIDI-triggered ADSR");
    addAndMakeVisible(adsrOnOff);
    adsrOnOff.setState (bav::gui::buttonStateFromBool (audioProcessor.getIsAdsrOn()));
    adsrOnOff.setToggleState (audioProcessor.getIsAdsrOn(), juce::NotificationType::dontSendNotification);

    latchToggle.setButtonText("MIDI latch");
    latchToggle.onClick = [this] { audioProcessor.setMidiLatch(latchToggle.isDown()); };
    //addAndMakeVisible(latchToggle);
    latchToggle.setState (bav::gui::buttonStateFromBool (audioProcessor.getIsMidiLatchOn()));
    latchToggle.setToggleState (audioProcessor.getIsMidiLatchOn(), juce::NotificationType::dontSendNotification);

    intervalLock.setButtonText("Interval lock");
    intervalLock.onClick = [this] { audioProcessor.setIntervalLock(intervalLock.isDown()); };
    //addAndMakeVisible(intervalLock);
    intervalLock.setState (bav::gui::buttonStateFromBool (audioProcessor.getIsIntervalLockOn()));
    intervalLock.setToggleState (audioProcessor.getIsIntervalLockOn(), juce::NotificationType::dontSendNotification);
    
    midiKill.setButtonText("Kill all MIDI");
    midiKill.onClick = [this] { audioProcessor.killAllMidi(); };
    addAndMakeVisible(midiKill);
    
    lookAndFeel.initializeSlider (quickKillMs, bvi_LINEAR_SLIDER, audioProcessor.getQuickKillMs(), audioProcessor.getDefaultQuickKillMs(), true);
    quickKillMs.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(quickKillMs);
    lookAndFeel.initializeLabel(quickKillmsLabel, "Quick kill ms");
    addAndMakeVisible(quickKillmsLabel);
    
    lookAndFeel.initializeSlider (stereoWidth, bvi_ROTARY_SLIDER, audioProcessor.getStereoWidth(), audioProcessor.getDefaultStereoWidth(), true);
    addAndMakeVisible(stereoWidth);
    lookAndFeel.initializeLabel(stereowidthLabel, "Stereo width");
    addAndMakeVisible(stereowidthLabel);

    lookAndFeel.initializeSlider (lowestPan, bvi_LINEAR_SLIDER, audioProcessor.getLowestPannedNote(),
                                  audioProcessor.getDefaultLowestPannedNote(), true);
    addAndMakeVisible(lowestPan);
    lookAndFeel.initializeLabel(lowestpanLabel, "Lowest panned pitch");
    addAndMakeVisible(lowestpanLabel);

    lookAndFeel.initializeSlider (midiVelocitySens, bvi_LINEAR_SLIDER, audioProcessor.getMidiVelocitySensitivity(),
                                  audioProcessor.getDefaultMidiVelocitySensitivity(), true);
    addAndMakeVisible(midiVelocitySens);
    lookAndFeel.initializeLabel(midivelocitysensLabel, "MIDI velocity sensitivity");
    addAndMakeVisible(midivelocitysensLabel);
    
    buildIntervalCombobox(pitchBendUp);
    pitchBendUp.setSelectedId(audioProcessor.getPitchbendRangeUp(), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(pitchBendUp);
    lookAndFeel.initializeLabel(pitchbendUpLabel, "Pitch bend range up");
    addAndMakeVisible(pitchbendUpLabel);

    buildIntervalCombobox(pitchBendDown);
    pitchBendDown.setSelectedId(audioProcessor.getPitchbendRangeDown(), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(pitchBendDown);
    lookAndFeel.initializeLabel(pitchbendDownLabel, "Pitch bend range down");
    addAndMakeVisible(pitchbendDownLabel);

    voiceStealing.setButtonText("Voice stealing");
    addAndMakeVisible(voiceStealing);
    voiceStealing.setState (bav::gui::buttonStateFromBool (audioProcessor.getIsVoiceStealingEnabled()));
    voiceStealing.setToggleState (audioProcessor.getIsVoiceStealingEnabled(), juce::NotificationType::dontSendNotification);

    lookAndFeel.initializeSlider (concertPitch, bvi_LINEAR_SLIDER, audioProcessor.getConcertPitchHz(),
                                  audioProcessor.getDefaultConcertPitchHz(), true);
    addAndMakeVisible(concertPitch);
    lookAndFeel.initializeLabel(concertPitchLabel, "Concert pitch (Hz)");
    addAndMakeVisible(concertPitchLabel);

    numberOfVoices.onChange = [this] { audioProcessor.updateNumVoices (numberOfVoices.getSelectedId()); };
    buildVoicesCombobox(numberOfVoices);
    numberOfVoices.setSelectedId(12, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(numberOfVoices);
    lookAndFeel.initializeLabel(numVoicesLabel, "Number of harmony voices");
    addAndMakeVisible(numVoicesLabel);

    pedalPitchToggle.setButtonText("MIDI pedal pitch");
    //addAndMakeVisible(pedalPitchToggle);
    pedalPitchToggle.setState (bav::gui::buttonStateFromBool (audioProcessor.getIsPedalPitchOn()));
    pedalPitchToggle.setToggleState (audioProcessor.getIsPedalPitchOn(), dontSendNotification);
    
    lookAndFeel.initializeSlider (pedalPitchThreshold, bvi_LINEAR_SLIDER, audioProcessor.getPedalPitchThresh(),
                                  audioProcessor.getDefaultPedalPitchThresh(), true);
    //addAndMakeVisible(pedalPitchThreshold);
    lookAndFeel.initializeLabel(pedalPitchThreshLabel, "Upper threshold");
    //addAndMakeVisible(pedalPitchThreshLabel);
    
    buildIntervalCombobox(pedalPitchInterval);
    pedalPitchInterval.setSelectedId(audioProcessor.getPedalPitchInterval(), juce::NotificationType::dontSendNotification);
    //addAndMakeVisible(pedalPitchInterval);
    lookAndFeel.initializeLabel(pedalPitchIntervalLabel, "Interval");
    //addAndMakeVisible(pedalPitchIntervalLabel);

    descantToggle.setButtonText("MIDI descant");
    //addAndMakeVisible(descantToggle);
    descantToggle.setState (bav::gui::buttonStateFromBool (audioProcessor.getIsDescantOn()));
    descantToggle.setToggleState (audioProcessor.getIsDescantOn(), dontSendNotification);
    
    lookAndFeel.initializeSlider (descantThreshold, bvi_LINEAR_SLIDER, audioProcessor.getDescantThresh(),
                                  audioProcessor.getDefaultDescantThresh(), true);
    //addAndMakeVisible(descantThreshold);
    lookAndFeel.initializeLabel(descantThreshLabel, "Lower threshold");
    //addAndMakeVisible(descantThreshLabel);
    
    buildIntervalCombobox(descantInterval);
    descantInterval.setSelectedId(audioProcessor.getDescantInterval(), juce::NotificationType::dontSendNotification);
    //addAndMakeVisible(descantInterval);
    lookAndFeel.initializeLabel(descantIntervalLabel, "Interval");
    //addAndMakeVisible(descantIntervalLabel);
    
    // TO DO : SOFT PEDAL GAIN
}
    
#undef bvi_LINEAR_SLIDER
#undef bvi_ROTARY_SLIDER
    

MidiControlPanel::~MidiControlPanel()
{
    this->setLookAndFeel(nullptr);
}

void MidiControlPanel::paint (juce::Graphics& g)
{
    g.fillAll (lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::backgroundPanelColourId));
    
    g.setColour(lookAndFeel.findColour(ImogenLookAndFeel::uiColourIds::insetPanelColourId));
    
    juce::Rectangle<int> adsrPanel (5, 110, 290, 125);
    g.fillRect(adsrPanel);
    
    juce::Rectangle<int> stereoWidthPanel (150, 310, 145, 100);
    g.fillRect(stereoWidthPanel);
    
    juce::Rectangle<int> midiVelocitysensPanel (5, 5, 85, 100);
    g.fillRect(midiVelocitysensPanel);
    
    juce::Rectangle<int> pitchbendPanel (5, 240, 290, 65);
    g.fillRect(pitchbendPanel);
    
}

void MidiControlPanel::resized()
{
    attackLabel.setBounds	(5, 130, 75, 35);
    adsrAttack.setBounds	(5, 152, 75, 75);
    decayLabel.setBounds	(78, 130, 75, 35);
    adsrDecay.setBounds		(78, 152, 75, 75);
    sustainLabel.setBounds	(148, 130, 75, 35);
    adsrSustain.setBounds	(148, 152, 75, 75);
    releaseLabel.setBounds	(220, 130, 75, 35);
    adsrRelease.setBounds	(220, 152, 75, 75);
    adsrOnOff.setBounds		(70, 110, 175, 35);
    
    // midi latch
    //latchToggle.setBounds(x, y, w, h);
    //latchTailOff.setBounds(x, y, w, h);

    stereowidthLabel.setBounds	(165, 300, 50, 50);
    stereoWidth.setBounds		(153, 333, 75, 75);
    lowestpanLabel.setBounds	(240, 310, 50, 50);
    lowestPan.setBounds			(248, 365, 35, 35);

    midivelocitysensLabel.setBounds(5, 10, 85, 35);
    midiVelocitySens.setBounds(25, 50, 45, 45);

    pitchbendUpLabel.setBounds	(15, 235, 130, 35);
    pitchBendUp.setBounds		(15, 265, 130, 30);
    
    pitchbendDownLabel.setBounds(150, 235, 140, 35);
    pitchBendDown.setBounds		(155, 265, 130, 30);

    //pedalPitchToggle.setBounds(x, y, w, h);
    //pedalPitchThreshold.setBounds(x, y, w, h);
    //pedalPitchThreshLabel.setBounds(x, y, w, h);
    //pedalPitchInterval.setBounds(x, y, w, h);
    //pedalPitchIntervalLabel.setBounds(x, y, w, h);

    //descantToggle.setBounds(x, y, w, h);
    //descantThreshold.setBounds(x, y, w, h);
    //descantThreshLabel.setBounds(x, y, w, h);
    //descantInterval.setBounds(x, y, w, h);
    //descantIntervalLabel.setBounds(x, y, w, h);

    midiKill.setBounds(145, 5, 100, 35);
    quickKillmsLabel.setBounds(98, 35, 85, 35);
    quickKillMs.setBounds(118, 60, 45, 45);
    
    concertPitchLabel.setBounds(190, 35, 110, 35);
    concertPitch.setBounds(220, 60, 45, 45);
    
    voiceStealing.setBounds(16, 310, 125, 35);
    
    numVoicesLabel.setBounds(18, 348, 115, 35);
    numberOfVoices.setBounds(42, 385, 65, 20);
    
}


inline void MidiControlPanel::buildVoicesCombobox(ComboBox& box)
{
    box.addItem("1", 1);
    box.addItem("2", 2);
    box.addItem("3", 3);
    box.addItem("4", 4);
    box.addItem("5", 5);
    box.addItem("6", 6);
    box.addItem("7", 7);
    box.addItem("8", 8);
    box.addItem("9", 9);
    box.addItem("10", 10);
    box.addItem("11", 11);
    box.addItem("12", 12);
}


inline void MidiControlPanel::buildIntervalCombobox(ComboBox& box)
{
    box.addItem("Minor Second", 1);
    box.addItem("Major Second", 2);
    box.addItem("Minor Third", 3);
    box.addItem("Major Third", 4);
    box.addItem("Perfect Fourth", 5);
    box.addItem("Aug Fourth/Dim Fifth", 6);
    box.addItem("Perfect Fifth", 7);
    box.addItem("Minor Sixth", 8);
    box.addItem("Major Sixth", 9);
    box.addItem("Minor Seventh", 10);
    box.addItem("Major Seventh", 11);
    box.addItem("Octave", 12);
}


void MidiControlPanel::updateNumVoicesCombobox(const int newNumVoices)
{
    numberOfVoices.setSelectedId(newNumVoices);
}


}  // namespace
