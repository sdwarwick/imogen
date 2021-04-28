
/*===================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 ImogenGUI.cpp: This file defines implementation details for Imogen's top-level GUI component.

===================================================================================================================================================*/


#include "bv_SharedCode/BinaryDataHelpers.h"

#include "Holders/ImogenGuiHolder.h"


ImogenGUI::ImogenGUI (ImogenEventSender* h): mainDial(h), holder(h), tooltipWindow(this, msBeforeTooltip)
{
    jassert (holder != nullptr);
    
    createParameters();
    jassert (parameters.size() == numParams);
           
    this->setBufferedToImage (true);
    
    makePresetMenu (selectPreset);
    selectPreset.onChange = [this] { holder->sendLoadPreset (selectPreset.getText()); };
    
    //addAndMakeVisible(selectPreset);
    addAndMakeVisible (mainDial);
    
    setSize (940, 435);
  
#if JUCE_MAC
    darkMode.store (juce::Desktop::isOSXDarkModeActive());
#else
    darkMode.store (true);
#endif
    
    mainDial.showPitchCorrection();
}


void ImogenGUI::createParameters()
{
    parameters.clear();
    parameters.reserve (numParams);
    
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (inputSourceID,          TRANS ("Input source")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (mainBypassID,           TRANS ("Main bypass")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (leadBypassID,           TRANS ("Lead bypass")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (harmonyBypassID,        TRANS ("Harmony bypass")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (dryPanID,               TRANS ("Dry pan")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (dryWetID,               TRANS ("% wet")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (adsrAttackID,           TRANS ("ADSR attack")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (adsrDecayID,            TRANS ("ADSR decay")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (adsrSustainID,          TRANS ("ADSR sustain")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (adsrReleaseID,          TRANS ("ADSR release")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (stereoWidthID,          TRANS ("Stereo width")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (lowestPannedID,         TRANS ("Lowest panned note")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (velocitySensID,         TRANS ("MIDI velocity sensitivity")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (pitchBendRangeID,       TRANS ("Pitchbend range (st)")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (pedalPitchIsOnID,       TRANS ("Pedal pitch toggle")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (pedalPitchThreshID,     TRANS ("Pedal pitch threshold")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (pedalPitchIntervalID,   TRANS ("Pedal pitch interval")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (descantIsOnID,          TRANS ("Descant toggle")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (descantThreshID,        TRANS ("Descant threshold")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (descantIntervalID,      TRANS ("Descant interval")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (voiceStealingID,        TRANS ("Voice stealing")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (inputGainID,            TRANS ("Input gain")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (outputGainID,           TRANS ("Output gain")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (limiterToggleID,        TRANS ("Limiter toggle")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (noiseGateToggleID,      TRANS ("Noise gate toggle")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (noiseGateThresholdID,   TRANS ("Noise gate thresh")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (compressorToggleID,     TRANS ("Compressor toggle")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (compressorAmountID,     TRANS ("Compressor amount")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (aftertouchGainToggleID, TRANS ("Aftertouch gain on/off")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (deEsserToggleID,        TRANS ("De-esser toggle")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (deEsserThreshID,        TRANS ("De-esser thresh")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (deEsserAmountID,        TRANS ("De-esser amount")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (reverbToggleID,         TRANS ("Reverb toggle")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (reverbDryWetID,         TRANS ("Reverb dry/wet")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (reverbDecayID,          TRANS ("Reverb decay")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (reverbDuckID,           TRANS ("Reverb duck")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (reverbLoCutID,          TRANS ("Reverb low cut")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (reverbHiCutID,          TRANS ("Reverb high cut")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (delayToggleID,          TRANS ("Delay toggle")));
    parameters.emplace_back (std::make_unique<ImogenGUIParameter> (delayToggleID,          TRANS ("Delay dry/wet")));
}


ImogenGUI::~ImogenGUI()
{
    this->setLookAndFeel (nullptr);
}

void ImogenGUI::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    
    if (darkMode.load())
    {
        
    }
    else
    {
        
    }
}


void ImogenGUI::recieveErrorCode (ErrorCode code)
{
    switch (code)
    {
        case (loadingPresetFailed): return;
    }
}


void ImogenGUI::setDarkMode (bool shouldUseDarkMode)
{
    darkMode.store (shouldUseDarkMode);
    // inform all child components of the change...
    this->repaint();
}


void ImogenGUI::resized()
{
    //selectPreset.setBounds (x, y, w, h);
    //mainDial.setBounds (x, y, w, h);
}


void ImogenGUI::recieveMidiLatchEvent (bool isNowLatched)
{
    juce::ignoreUnused (isNowLatched);
}


void ImogenGUI::recieveKillAllMidiEvent()
{
    
}


void ImogenGUI::recieveParameterChange (ParameterID paramID, float newValue)
{
    juce::ignoreUnused (paramID, newValue);
    
    // getComponentForParameter (paramID) ...
}


void ImogenGUI::recieveParameterChangeGestureStart (ParameterID paramID)
{
    mainDial.showParameter (paramID);
}

void ImogenGUI::recieveParameterChangeGestureEnd (ParameterID paramID)
{
    juce::ignoreUnused (paramID);
    mainDial.showPitchCorrection();
}


inline void ImogenGUI::makePresetMenu (juce::ComboBox& box)
{
    juce::ignoreUnused (box);
}


void ImogenGUI::recieveLoadPreset (const juce::String& newPresetName)
{
    if (newPresetName.isEmpty())
    {
        
    }
    else
    {
        const auto displayName = TRANS(newPresetName);
        juce::ignoreUnused (displayName);
    }
}


void ImogenGUI::recieveMTSconnectionChange (bool isNowConnected)
{
    juce::ignoreUnused (isNowConnected);
}

void ImogenGUI::recieveMTSscaleChange (const juce::String& newScaleName)
{
    const auto displayName = TRANS(newScaleName);
    juce::ignoreUnused (displayName);
}


/*===========================================================================================================================
 ============================================================================================================================*/

bool ImogenGUI::keyPressed (const juce::KeyPress& key)
{
    juce::ignoreUnused (key);
    return false;
}

bool ImogenGUI::keyStateChanged (bool isKeyDown)
{
    juce::ignoreUnused (isKeyDown);
    return false;
}

void ImogenGUI::modifierKeysChanged (const juce::ModifierKeys& modifiers)
{
    juce::ignoreUnused (modifiers);
}

void ImogenGUI::focusLost (FocusChangeType cause)
{
    juce::ignoreUnused (cause);
}


/*===========================================================================================================================
 ============================================================================================================================*/

ImogenGUIParameter* ImogenGUI::getParameter (ParameterID paramID) const
{
    for (auto& parameter : parameters)
    {
        auto* param = parameter.get();
        if (param->getID() == paramID)
            return param;
    }
    
    return nullptr;
}
