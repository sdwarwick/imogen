/*
    This file defines a user interface control panel containing controls linked to the harmonizer's various MIDI settings, controls, and functions.
    Parent file: MidiControlPanel.h.
*/


#include "../../Source/GUI/MidiControlPanel/MidiControlPanel.h"


#undef bvi_UPDATE_ADSR
#undef bvi_UPDATE_PEDAL
#undef bvi_UPDATE_DESCANT
#undef bvi_ROTARY_SLIDER
#undef bvi_LINEAR_SLIDER


namespace bav

{
    
#define bvi_ROTARY_SLIDER Slider::SliderStyle::RotaryVerticalDrag
#define bvi_LINEAR_SLIDER Slider::SliderStyle::LinearBarVertical
    
    
MidiControlPanel::MidiControlPanel(ImogenAudioProcessor& p, ImogenLookAndFeel& l):
    audioProcessor(p), lookAndFeel(l),
    attackLink (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrAttack", adsrAttack)),
    decayLink  (std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrDecay", adsrDecay)),
    sustainLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrSustain", adsrSustain)),
    releaseLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "adsrRelease", adsrRelease)),
    adsrOnOffLink  (std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "adsrOnOff", adsrOnOff)),
    latchToggleLink(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "latchIsOn", latchToggle)),
    intervalLockLink(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "intervalLock", intervalLock)),
    stereoWidthLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "stereoWidth", stereoWidth)),
    lowestPanLink  (std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "lowestPan", lowestPan)),
    midiVelocitySensLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.tree, "midiVelocitySensitivity", midiVelocitySens)),
    pitchBendUpLink  (std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.tree, "PitchBendUpRange", pitchBendUp)),
    pitchBendDownLink(std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.tree, "PitchBendDownRange", pitchBendDown)),
    voiceStealingLink(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "voiceStealing", voiceStealing)),
    quickKillMsLink  (std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "quickKillMs", quickKillMs)),
    concertPitchLink (std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "concertPitch", concertPitch)),
    pedalPitchToggleLink(std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "pedalPitchToggle", pedalPitchToggle)),
    pedalPitchThreshLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "pedalPitchThresh", pedalPitchThreshold)),
    pedalPitchIntervalLink(std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.tree, "pedalPitchInterval", pedalPitchInterval)),
    descantToggleLink   (std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, "descantToggle", descantToggle)),
    descantThresholdLink(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, "descantThresh", descantThreshold)),
    descantIntervalLink (std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.tree, "descantInterval", descantInterval))
{
    // ADSR
#define bvi_UPDATE_ADSR [this] { audioProcessor.updateAdsr(); }
    lookAndFeel.initializeSlider (adsrAttack, bvi_ROTARY_SLIDER, audioProcessor.adsrAttack->get());
    addAndMakeVisible(adsrAttack);
    adsrAttack.onValueChange = bvi_UPDATE_ADSR;
    lookAndFeel.initializeLabel(attackLabel, "Attack");
    addAndMakeVisible(attackLabel);
    
    lookAndFeel.initializeSlider (adsrDecay, bvi_ROTARY_SLIDER, audioProcessor.adsrDecay->get());
    addAndMakeVisible(adsrDecay);
    adsrDecay.onValueChange = bvi_UPDATE_ADSR;
    lookAndFeel.initializeLabel(decayLabel, "Decay");
    addAndMakeVisible(decayLabel);
    
    lookAndFeel.initializeSlider (adsrSustain, bvi_ROTARY_SLIDER, audioProcessor.adsrSustain->get());
    addAndMakeVisible(adsrSustain);
    adsrSustain.onValueChange = bvi_UPDATE_ADSR;
    lookAndFeel.initializeLabel(sustainLabel, "Sustain");
    addAndMakeVisible(sustainLabel);
    
    lookAndFeel.initializeSlider (adsrRelease, bvi_ROTARY_SLIDER, audioProcessor.adsrRelease->get());
    addAndMakeVisible(adsrRelease);
    adsrRelease.onValueChange = bvi_UPDATE_ADSR;
    lookAndFeel.initializeLabel(releaseLabel, "Release");
    addAndMakeVisible(releaseLabel);
    
    adsrOnOff.setButtonText("MIDI-triggered ADSR");
    addAndMakeVisible(adsrOnOff);
    adsrOnOff.onClick = bvi_UPDATE_ADSR;
#undef bvi_UPDATE_ADSR
    
    adsrOnOff.setState (bav::GuiUtils::buttonStateFromBool (audioProcessor.adsrToggle->get()));
    adsrOnOff.setToggleState (audioProcessor.adsrToggle->get(), juce::NotificationType::dontSendNotification);

    // Midi latch
    latchToggle.setButtonText("MIDI latch");
    //addAndMakeVisible(latchToggle);
    latchToggle.onClick = [this] { audioProcessor.updateMidiLatch(); };
    latchToggle.setState (bav::GuiUtils::buttonStateFromBool (audioProcessor.latchIsOn->get()));
    latchToggle.setToggleState (audioProcessor.latchIsOn->get(), juce::NotificationType::dontSendNotification);

    // interval lock
    intervalLock.setButtonText("Interval lock");
    //addAndMakeVisible(intervalLock);
    intervalLock.onClick = [this] { audioProcessor.updateIntervalLock(); };
    intervalLock.setState (bav::GuiUtils::buttonStateFromBool (audioProcessor.intervalLockIsOn->get()));
    intervalLock.setToggleState (audioProcessor.intervalLockIsOn->get(), juce::NotificationType::dontSendNotification);
    
    // kill all midi button
    midiKill.setButtonText("Kill all MIDI");
    midiKill.onClick = [this] { audioProcessor.killAllMidi(); };
    addAndMakeVisible(midiKill);
    
    // quick kill ms slider
    lookAndFeel.initializeSlider (quickKillMs, bvi_LINEAR_SLIDER, audioProcessor.quickKillMs->get());
    quickKillMs.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(quickKillMs);
    quickKillMs.onValueChange = [this] { audioProcessor.updateQuickKillMs(); };
    lookAndFeel.initializeLabel(quickKillmsLabel, "Quick kill ms");
    addAndMakeVisible(quickKillmsLabel);
    
    lookAndFeel.initializeSlider (stereoWidth, bvi_ROTARY_SLIDER, audioProcessor.stereoWidth->get());
    addAndMakeVisible(stereoWidth);
    stereoWidth.onValueChange = [this] { audioProcessor.updateStereoWidth(); };
    lookAndFeel.initializeLabel(stereowidthLabel, "Stereo width");
    addAndMakeVisible(stereowidthLabel);

    lookAndFeel.initializeSlider (lowestPan, bvi_LINEAR_SLIDER, audioProcessor.lowestPanned->get());
    addAndMakeVisible(lowestPan);
    lowestPan.onValueChange = [this] { audioProcessor.updateStereoWidth(); };
    lookAndFeel.initializeLabel(lowestpanLabel, "Lowest panned pitch");
    addAndMakeVisible(lowestpanLabel);

    lookAndFeel.initializeSlider (midiVelocitySens, bvi_LINEAR_SLIDER, audioProcessor.velocitySens->get());
    addAndMakeVisible(midiVelocitySens);
    midiVelocitySens.onValueChange = [this] { audioProcessor.updateMidiVelocitySensitivity(); };
    lookAndFeel.initializeLabel(midivelocitysensLabel, "MIDI velocity sensitivity");
    addAndMakeVisible(midivelocitysensLabel);
    
    // pitch bend settings
    buildIntervalCombobox(pitchBendUp);
    pitchBendUp.setSelectedId(audioProcessor.pitchBendUp->get(), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(pitchBendUp);
    pitchBendUp.onChange = [this] { audioProcessor.updatePitchbendSettings(); };
    lookAndFeel.initializeLabel(pitchbendUpLabel, "Pitch bend range up");
    addAndMakeVisible(pitchbendUpLabel);

    buildIntervalCombobox(pitchBendDown);
    pitchBendDown.setSelectedId(audioProcessor.pitchBendDown->get(), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(pitchBendDown);
    pitchBendDown.onChange = [this] { audioProcessor.updatePitchbendSettings(); };
    lookAndFeel.initializeLabel(pitchbendDownLabel, "Pitch bend range down");
    addAndMakeVisible(pitchbendDownLabel);

    voiceStealing.setButtonText("Voice stealing");
    voiceStealing.onClick = [this] { audioProcessor.updateNoteStealing(); };
    addAndMakeVisible(voiceStealing);
    voiceStealing.setState (bav::GuiUtils::buttonStateFromBool (audioProcessor.voiceStealing->get()));
    voiceStealing.setToggleState (audioProcessor.voiceStealing->get(), juce::NotificationType::dontSendNotification);

    lookAndFeel.initializeSlider (concertPitch, bvi_LINEAR_SLIDER, audioProcessor.concertPitchHz->get());
    addAndMakeVisible(concertPitch);
    concertPitch.onValueChange = [this] { audioProcessor.updateConcertPitch(); };
    lookAndFeel.initializeLabel(concertPitchLabel, "Concert pitch (Hz)");
    addAndMakeVisible(concertPitchLabel);

    numberOfVoices.onChange = [this] { audioProcessor.updateNumVoices (numberOfVoices.getSelectedId()); };
    buildVoicesCombobox(numberOfVoices);
    numberOfVoices.setSelectedId(12, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(numberOfVoices);
    lookAndFeel.initializeLabel(numVoicesLabel, "Number of harmony voices");
    addAndMakeVisible(numVoicesLabel);

    // pedal pitch
#define bvi_UPDATE_PEDAL [this] { audioProcessor.updatePedalPitch(); }
    pedalPitchToggle.setButtonText("MIDI pedal pitch");
    //addAndMakeVisible(pedalPitchToggle);
    pedalPitchToggle.onClick = bvi_UPDATE_PEDAL;
    pedalPitchToggle.setState (bav::GuiUtils::buttonStateFromBool (audioProcessor.pedalPitchIsOn->get()));
    pedalPitchToggle.setToggleState (audioProcessor.pedalPitchIsOn->get(), dontSendNotification);
    
    lookAndFeel.initializeSlider (pedalPitchThreshold, bvi_LINEAR_SLIDER, audioProcessor.pedalPitchThresh->get());
    //addAndMakeVisible(pedalPitchThreshold);
    pedalPitchThreshold.onValueChange = bvi_UPDATE_PEDAL;
    lookAndFeel.initializeLabel(pedalPitchThreshLabel, "Upper threshold");
    //addAndMakeVisible(pedalPitchThreshLabel);
    
    buildIntervalCombobox(pedalPitchInterval);
    pedalPitchInterval.setSelectedId(audioProcessor.pedalPitchInterval->get(), juce::NotificationType::dontSendNotification);
    //addAndMakeVisible(pedalPitchInterval);
    pedalPitchInterval.onChange = bvi_UPDATE_PEDAL;
    lookAndFeel.initializeLabel(pedalPitchIntervalLabel, "Interval");
    //addAndMakeVisible(pedalPitchIntervalLabel);
#undef bvi_UPDATE_PEDAL

    // descant
#define bvi_UPDATE_DESCANT [this] { audioProcessor.updateDescant(); }
    descantToggle.setButtonText("MIDI descant");
    //addAndMakeVisible(descantToggle);
    descantToggle.onClick = bvi_UPDATE_DESCANT;
    
    descantToggle.setState (bav::GuiUtils::buttonStateFromBool (audioProcessor.descantIsOn->get()));
    descantToggle.setToggleState (audioProcessor.descantIsOn->get(), dontSendNotification);
    
    lookAndFeel.initializeSlider (descantThreshold, bvi_LINEAR_SLIDER, audioProcessor.descantThresh->get());
    //addAndMakeVisible(descantThreshold);
    descantThreshold.onValueChange = bvi_UPDATE_DESCANT;
    lookAndFeel.initializeLabel(descantThreshLabel, "Lower threshold");
    //addAndMakeVisible(descantThreshLabel);
    
    buildIntervalCombobox(descantInterval);
    descantInterval.setSelectedId(audioProcessor.descantInterval->get(), juce::NotificationType::dontSendNotification);
    //addAndMakeVisible(descantInterval);
    descantInterval.onChange = bvi_UPDATE_DESCANT;
    lookAndFeel.initializeLabel(descantIntervalLabel, "Interval");
    //addAndMakeVisible(descantIntervalLabel);
#undef bvi_UPDATE_DESCANT
    
    // TO DO : SOFT PEDAL GAIN
    
}
    
#undef bvi_LINEAR_SLIDER
#undef bvi_ROTARY_SLIDER
    

MidiControlPanel::~MidiControlPanel()
{
    setLookAndFeel(nullptr);
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
    // adsr
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
    }
    
    // midi latch
    //latchToggle.setBounds(x, y, w, h);
    //latchTailOff.setBounds(x, y, w, h);
    
    // stereo width
    {
        stereowidthLabel.setBounds	(165, 300, 50, 50);
        stereoWidth.setBounds		(153, 333, 75, 75);
        
        lowestpanLabel.setBounds	(240, 310, 50, 50);
        lowestPan.setBounds			(248, 365, 35, 35);
    }
    
    // midi velocity sensitivity
    {
        midivelocitysensLabel.setBounds(5, 10, 85, 35);
        midiVelocitySens.setBounds(25, 50, 45, 45);
    }
    
    // pitch bend
    {
        pitchbendUpLabel.setBounds	(15, 235, 130, 35);
        pitchBendUp.setBounds		(15, 265, 130, 30);
        
        pitchbendDownLabel.setBounds(150, 235, 140, 35);
        pitchBendDown.setBounds		(155, 265, 130, 30);
    }
    
    // pedal pitch
    {
        //pedalPitchToggle.setBounds(x, y, w, h);
        //pedalPitchThreshold.setBounds(x, y, w, h);
        //pedalPitchThreshLabel.setBounds(x, y, w, h);
        //pedalPitchInterval.setBounds(x, y, w, h);
        //pedalPitchIntervalLabel.setBounds(x, y, w, h);
    }
    
    // descant
    {
        //descantToggle.setBounds(x, y, w, h);
        //descantThreshold.setBounds(x, y, w, h);
        //descantThreshLabel.setBounds(x, y, w, h);
        //descantInterval.setBounds(x, y, w, h);
        //descantIntervalLabel.setBounds(x, y, w, h);
    }
    
    midiKill.setBounds(145, 5, 100, 35);
    quickKillmsLabel.setBounds(98, 35, 85, 35);
    quickKillMs.setBounds(118, 60, 45, 45);
    
    concertPitchLabel.setBounds(190, 35, 110, 35);
    concertPitch.setBounds(220, 60, 45, 45);
    
    voiceStealing.setBounds(16, 310, 125, 35);
    
    numVoicesLabel.setBounds(18, 348, 115, 35);
    numberOfVoices.setBounds(42, 385, 65, 20);
    
}


void MidiControlPanel::buildVoicesCombobox(ComboBox& box)
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


void MidiControlPanel::buildIntervalCombobox(ComboBox& box)
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
