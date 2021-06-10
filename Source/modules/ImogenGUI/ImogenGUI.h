
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


namespace Imogen
{
class GUI : public juce::Component,
            public gui::GUIInitializer
{
public:
    GUI (Imogen::State& stateToUse);

    virtual ~GUI() override;

private:
    /*=========================================================================================*/
    /* juce::Component functions */

    void paint (juce::Graphics& g) final;
    void resized() final;

    bool keyPressed (const juce::KeyPress& key) final;
    bool keyStateChanged (bool isKeyDown) final;
    void modifierKeysChanged (const juce::ModifierKeys& modifiers) final;
    void focusLost (FocusChangeType cause) final;

    /*=========================================================================================*/

    inline void makePresetMenu (juce::ComboBox& box);

    /*=========================================================================================*/

    ImogenDialComponent mainDial;

    ImogenLookAndFeel lookAndFeel;

    juce::TooltipWindow tooltipWindow {this, 700};

    State&      state;
    Parameters& parameters;
    Internals&  internals;
    Meters&     meters;
    
    UndoManager   undoManager {state};
    
    PresetManager presetManager;

    gui::DarkModeSentinel darkModeSentinel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GUI)
};

}  // namespace Imogen
