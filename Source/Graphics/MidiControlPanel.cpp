/*
 ==============================================================================
 
 MidiControlPanel.cpp
 Created: 29 Nov 2020 5:31:17pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#include <JuceHeader.h>
#include "Graphics/MidiControlPanel.h"

//==============================================================================
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
    {
        // attack
        {
            adsrAttack.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
            adsrAttack.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
            addAndMakeVisible(adsrAttack);
            adsrAttack.setValue(audioProcessor.adsrAttack->get());
            adsrAttack.onValueChange = [this] { audioProcessor.updateAdsr(); };
            lookAndFeel.initializeLabel(attackLabel, "Attack");
            addAndMakeVisible(attackLabel);
        }
        
        // decay
        {
            adsrDecay.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
            adsrDecay.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
            addAndMakeVisible(adsrDecay);
            adsrDecay.setValue(audioProcessor.adsrDecay->get());
            adsrDecay.onValueChange = [this] { audioProcessor.updateAdsr(); };
            lookAndFeel.initializeLabel(decayLabel, "Decay");
            addAndMakeVisible(decayLabel);
        }
        
        // sustain
        {
            adsrSustain.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
            adsrSustain.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
            addAndMakeVisible(adsrSustain);
            adsrSustain.setValue(audioProcessor.adsrSustain->get());
            adsrSustain.onValueChange = [this] { audioProcessor.updateAdsr(); };
            lookAndFeel.initializeLabel(sustainLabel, "Sustain");
            addAndMakeVisible(sustainLabel);
        }
        
        // release
        {
            adsrRelease.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
            adsrRelease.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
            addAndMakeVisible(adsrRelease);
            adsrRelease.setValue(audioProcessor.adsrRelease->get());
            adsrRelease.onValueChange = [this] { audioProcessor.updateAdsr(); };
            lookAndFeel.initializeLabel(releaseLabel, "Release");
            addAndMakeVisible(releaseLabel);
        }
        
        // on/off toggle
        {
            adsrOnOff.setButtonText("MIDI-triggered ADSR");
            addAndMakeVisible(adsrOnOff);
            adsrOnOff.onClick = [this] { audioProcessor.updateAdsr(); };
            
            adsrOnOff.setState (buttonStateFromBool (audioProcessor.adsrToggle->get()));
            adsrOnOff.setToggleState (audioProcessor.adsrToggle->get(), dontSendNotification);
        }
    }
    
    // Midi latch
    {
        latchToggle.setButtonText("MIDI latch");
        //addAndMakeVisible(latchToggle);
        latchToggle.onClick = [this]
                              {
                                  audioProcessor.updateMidiLatch();
                                  if (latchToggle.getToggleState() == true)
                                  {
                                      intervalLock.setState (buttonStateFromBool (false));
                                      intervalLock.setToggleState (false, dontSendNotification);
                                  }
                              };
        
        latchToggle.setState (buttonStateFromBool (audioProcessor.latchIsOn->get()));
        latchToggle.setToggleState (audioProcessor.latchIsOn->get(), dontSendNotification);
    }
    
    // interval lock
    {
        intervalLock.setButtonText("Interval lock");
        //addAndMakeVisible(intervalLock);
        intervalLock.onClick = [this]
                               {
                                   audioProcessor.updateIntervalLock();
                                   if (intervalLock.getToggleState() == true)
                                   {
                                       latchToggle.setState (buttonStateFromBool (false));
                                       latchToggle.setToggleState (false, dontSendNotification);
                                   }
                               };
        
        intervalLock.setState (buttonStateFromBool (audioProcessor.intervalLockIsOn->get()));
        intervalLock.setToggleState (audioProcessor.intervalLockIsOn->get(), dontSendNotification);
    }
    
    // kill all midi button
    {
        midiKill.setButtonText("Kill all MIDI");
        midiKill.onClick = [this] { audioProcessor.killAllMidi(); };
        addAndMakeVisible(midiKill);
    }
    // quick kill ms slider
    {
        quickKillMs.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
        quickKillMs.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
        addAndMakeVisible(quickKillMs);
        quickKillMs.setValue(audioProcessor.quickKillMs->get());
        quickKillMs.setNumDecimalPlacesToDisplay(0);
        quickKillMs.onValueChange = [this] { audioProcessor.updateQuickKillMs(); };
        lookAndFeel.initializeLabel(quickKillmsLabel, "Quick kill ms");
        addAndMakeVisible(quickKillmsLabel);
    }
    
    // stereo width
    {
        // stereo width dial
        {
            stereoWidth.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
            stereoWidth.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
            addAndMakeVisible(stereoWidth);
            stereoWidth.setValue(audioProcessor.stereoWidth->get());
            stereoWidth.onValueChange = [this] { audioProcessor.updateStereoWidth(); };
            lookAndFeel.initializeLabel(stereowidthLabel, "Stereo width");
            addAndMakeVisible(stereowidthLabel);
        }
        
        // lowest panned midiPitch
        {
            lowestPan.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
            lowestPan.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
            addAndMakeVisible(lowestPan);
            lowestPan.setValue(audioProcessor.lowestPanned->get());
            lowestPan.onValueChange = [this] { audioProcessor.updateStereoWidth(); };
            lookAndFeel.initializeLabel(lowestpanLabel, "Lowest panned pitch");
            addAndMakeVisible(lowestpanLabel);
        }
    }
    
    // MIDI velocity sensitivity
    {
        midiVelocitySens.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
        midiVelocitySens.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
        addAndMakeVisible(midiVelocitySens);
        midiVelocitySens.setValue(audioProcessor.velocitySens->get());
        midiVelocitySens.onValueChange = [this] { audioProcessor.updateMidiVelocitySensitivity(); };
        lookAndFeel.initializeLabel(midivelocitysensLabel, "MIDI velocity sensitivity");
        addAndMakeVisible(midivelocitysensLabel);
    }
    
    // pitch bend settings
    {
        {
            buildIntervalCombobox(pitchBendUp);
            pitchBendUp.setSelectedId(audioProcessor.pitchBendUp->get());
            addAndMakeVisible(pitchBendUp);
            pitchBendUp.onChange = [this] { audioProcessor.updatePitchbendSettings(); };
            lookAndFeel.initializeLabel(pitchbendUpLabel, "Pitch bend range up");
            addAndMakeVisible(pitchbendUpLabel);
        }
        {
            buildIntervalCombobox(pitchBendDown);
            pitchBendDown.setSelectedId(audioProcessor.pitchBendDown->get());
            addAndMakeVisible(pitchBendDown);
            pitchBendDown.onChange = [this] { audioProcessor.updatePitchbendSettings(); };
            lookAndFeel.initializeLabel(pitchbendDownLabel, "Pitch bend range down");
            addAndMakeVisible(pitchbendDownLabel);
        }
    }
    
    // voice stealing on/off
    {
        voiceStealing.setButtonText("Voice stealing");
        voiceStealing.onClick = [this] { audioProcessor.updateNoteStealing(); };
        addAndMakeVisible(voiceStealing);
        
        voiceStealing.setState (buttonStateFromBool (audioProcessor.voiceStealing->get()));
        voiceStealing.setToggleState (audioProcessor.voiceStealing->get(), dontSendNotification);
    }
    
    // concert pitch
    {
        concertPitch.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
        concertPitch.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
        addAndMakeVisible(concertPitch);
        concertPitch.setValue(audioProcessor.concertPitchHz->get());
        concertPitch.onValueChange = [this] { audioProcessor.updateConcertPitch(); };
        lookAndFeel.initializeLabel(concertPitchLabel, "Concert pitch (Hz)");
        addAndMakeVisible(concertPitchLabel);
    }
    
    // # of harmony voices
    {
        numberOfVoices.onChange = [this] { audioProcessor.updateNumVoices(numberOfVoices.getSelectedId()); };
        buildVoicesCombobox(numberOfVoices);
        numberOfVoices.setSelectedId(12);
        addAndMakeVisible(numberOfVoices);
        lookAndFeel.initializeLabel(numVoicesLabel, "Number of harmony voices");
        addAndMakeVisible(numVoicesLabel);
    }
    
    // pedal pitch
    {
        pedalPitchToggle.setButtonText("MIDI pedal pitch");
        //addAndMakeVisible(pedalPitchToggle);
        pedalPitchToggle.onClick = [this] { audioProcessor.updatePedalPitch(); };
        
        pedalPitchToggle.setState (buttonStateFromBool (audioProcessor.pedalPitchIsOn->get()));
        pedalPitchToggle.setToggleState (audioProcessor.pedalPitchIsOn->get(), dontSendNotification);
        
        pedalPitchThreshold.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
        pedalPitchThreshold.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
        //addAndMakeVisible(pedalPitchThreshold);
        pedalPitchThreshold.setValue(audioProcessor.pedalPitchThresh->get());
        pedalPitchThreshold.onValueChange = [this] { audioProcessor.updatePedalPitch(); };
        lookAndFeel.initializeLabel(pedalPitchThreshLabel, "Upper threshold");
        //addAndMakeVisible(pedalPitchThreshLabel);
        
        buildIntervalCombobox(pedalPitchInterval);
        pedalPitchInterval.setSelectedId(audioProcessor.pedalPitchInterval->get());
        //addAndMakeVisible(pedalPitchInterval);
        pedalPitchInterval.onChange = [this] { audioProcessor.updatePedalPitch(); };
        lookAndFeel.initializeLabel(pedalPitchIntervalLabel, "Interval");
        //addAndMakeVisible(pedalPitchIntervalLabel);
    }
    
    // descant
    {
        descantToggle.setButtonText("MIDI descant");
        //addAndMakeVisible(descantToggle);
        descantToggle.onClick = [this] { audioProcessor.updateDescant(); };
        
        descantToggle.setState (buttonStateFromBool (audioProcessor.descantIsOn->get()));
        descantToggle.setToggleState (audioProcessor.descantIsOn->get(), dontSendNotification);
        
        descantThreshold.setSliderStyle(Slider::SliderStyle::LinearBarVertical);
        descantThreshold.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
        //addAndMakeVisible(descantThreshold);
        descantThreshold.setValue(audioProcessor.descantThresh->get());
        descantThreshold.onValueChange = [this] { audioProcessor.updateDescant(); };
        lookAndFeel.initializeLabel(descantThreshLabel, "Lower threshold");
        //addAndMakeVisible(descantThreshLabel);
        
        buildIntervalCombobox(descantInterval);
        descantInterval.setSelectedId(audioProcessor.descantInterval->get());
        //addAndMakeVisible(descantInterval);
        descantInterval.onChange = [this] { audioProcessor.updateDescant(); };
        lookAndFeel.initializeLabel(descantIntervalLabel, "Interval");
        //addAndMakeVisible(descantIntervalLabel);
    }
    
    
    // TO DO : SOFT PEDAL GAIN
    
};

MidiControlPanel::~MidiControlPanel()
{
    setLookAndFeel(nullptr);
};

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
    
};

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
    
};


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
};


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
};


void MidiControlPanel::updateNumVoicesCombobox(const int newNumVoices)
{
    numberOfVoices.setSelectedId(newNumVoices);
};


Button::ButtonState MidiControlPanel::buttonStateFromBool (const bool isOn)
{
    if (isOn)
        return Button::ButtonState::buttonDown;
    
    return Button::ButtonState::buttonNormal;
}
