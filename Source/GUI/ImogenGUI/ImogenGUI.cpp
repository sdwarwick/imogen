
#include "ImogenGUI.h"

#include "LookAndFeel/ImogenLookAndFeel.cpp"
#include "MainDialComponent/MainDialComponent.cpp"


ImogenGUI::ImogenGUI()
    : GUIInitializer (getTopLevelComponent()), tooltipWindow (this, msBeforeTooltip)
{
    setInterceptsMouseClicks (false, true);
    
    state.addAllAsInternal();

    addAndMakeVisible (mainDial);

    internals.abletonLinkEnabled->onParameterChange = []() { };
    internals.abletonLinkSessionPeers->onParameterChange = [](){ };
    internals.mtsEspIsConnected->onParameterChange = [](){ };
    internals.lastMovedMidiController->onParameterChange = [](){ };
    internals.lastMovedCCValue->onParameterChange = [](){ };
    internals.guiDarkMode->onParameterChange = [this]() { repaint(); };
    internals.currentInputNote->onParameterChange = [](){ };
    internals.currentCentsSharp->onParameterChange = [](){ };
    
    meters.inputLevel->onParameterChange = [](){ };
    meters.outputLevelL->onParameterChange = [](){ };
    meters.outputLevelR->onParameterChange = [](){ };
    meters.gateRedux->onParameterChange = [](){ };
    meters.compRedux->onParameterChange = [](){ };
    meters.deEssRedux->onParameterChange = [](){ };
    meters.limRedux->onParameterChange = [](){ };
    meters.reverbLevel->onParameterChange = [](){ };
    meters.delayLevel->onParameterChange = [](){ };
    
    rescanPresetsFolder();
}


ImogenGUI::~ImogenGUI()
{
    this->setLookAndFeel (nullptr);
}

/*=========================================================================================================
 =========================================================================================================*/

void ImogenGUI::rescanPresetsFolder()
{
    //    availablePresets.clearQuick();
    //    const auto xtn = getPresetFileExtension();
    //
    //    for (auto entry  :   juce::RangedDirectoryIterator (getPresetsFolder(), false))
    //    {
    //        const auto filename = entry.getFile().getFileName();
    //
    //        if (filename.endsWith (xtn))
    //            availablePresets.add (filename.dropLastCharacters (xtn.length()));
    //    }

    repaint();
}


void ImogenGUI::savePreset (const juce::String& presetName)
{
    bav::toBinary (state, Imogen::presetNameToFilePath (presetName));
    rescanPresetsFolder();
}

void ImogenGUI::loadPreset (const juce::String& presetName)
{
    bav::fromBinary (Imogen::presetNameToFilePath (presetName), state);
    repaint();
}

void ImogenGUI::deletePreset (const juce::String& presetName)
{
    bav::deleteFile (Imogen::presetNameToFilePath (presetName));
    rescanPresetsFolder();
}

void ImogenGUI::renamePreset (const juce::String& previousName, const juce::String& newName)
{
    bav::renameFile (Imogen::presetNameToFilePath (previousName),
                     bav::addFileExtensionIfMissing (newName, Imogen::getPresetFileExtension()));

    rescanPresetsFolder();
}


/*=========================================================================================================
    juce::Component functions
 =========================================================================================================*/

void ImogenGUI::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    if (internals.guiDarkMode->get())
    {
    }
    else
    {
    }
}

void ImogenGUI::resized()
{
    //selectPreset.setBounds (x, y, w, h);
    //mainDial.setBounds (x, y, w, h);
}

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


inline void ImogenGUI::makePresetMenu (juce::ComboBox& box)
{
    juce::ignoreUnused (box);
}
