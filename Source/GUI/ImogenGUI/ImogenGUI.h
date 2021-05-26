
/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 ImogenEngine
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenEngine
 description:        self-contained module representing Imogen's entire user interface
 dependencies:       bv_gui ImogenCommon
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include "bv_gui/bv_gui.h"
#include "ImogenCommon/ImogenCommon.h"

#include "LookAndFeel/ImogenLookAndFeel.h"
#include "MainDialComponent/MainDialComponent.h"


class ImogenGUI : public juce::Component,
                  public bav::GUIInitializer
{
public:
    ImogenGUI();

    virtual ~ImogenGUI() override;

private:
    /*=========================================================================================*/
    /* juce::Component functions */

    void paint (juce::Graphics& g) override;
    void resized() override;

    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;
    void modifierKeysChanged (const juce::ModifierKeys& modifiers) override;
    void focusLost (FocusChangeType cause) override;

    /*=========================================================================================*/

    inline void makePresetMenu (juce::ComboBox& box);

    void rescanPresetsFolder();
    void loadPreset (const juce::String& presetName);
    void savePreset (const juce::String& presetName);
    void renamePreset (const juce::String& previousName, const juce::String& newName);
    void deletePreset (const juce::String& presetName);

    /*=========================================================================================*/

    juce::UndoManager undoManager;

    ImogenDialComponent mainDial;

    bav::ImogenLookAndFeel lookAndFeel;

    juce::TooltipWindow  tooltipWindow {this, 700};

    Imogen::State       state;
    Imogen::Parameters& parameters {state.parameters};
    Imogen::Internals&  internals {state.internals};
    Imogen::Meters&     meters {state.meters};
    
    bav::gui::DarkModeSentinel darkModeUpdater {internals.guiDarkMode};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenGUI)
};
