#pragma once

#include <bv_plugin_gui/bv_plugin_gui.h>

#include <ImogenCommon/ImogenCommon.h>

#include <ImogenGUI/LookAndFeel/ImogenLookAndFeel.h>
#include <ImogenGUI/MainDialComponent/MainDialComponent.h>

namespace Imogen
{

class GUI : public juce::Component,
public gui::GUIInitializer
{
public:
    GUI (State& stateToUse);
    
    virtual ~GUI() override;
    
private:
    void paint (juce::Graphics& g) final;
    void resized() final;
    
    bool keyPressed (const juce::KeyPress& key) final;
    bool keyStateChanged (bool isKeyDown) final;
    void modifierKeysChanged (const juce::ModifierKeys& modifiers) final;
    void focusLost (FocusChangeType cause) final;
    
    ImogenLookAndFeel lookAndFeel;
    
    juce::TooltipWindow tooltipWindow {this, 700};
    
    State&      state;
    Parameters& parameters {state.parameters};
    Internals&  internals {state.internals};
    Meters&     meters {state.meters};
    
    MainDialComponent mainDial {state};
    
    PluginUndo undoManager {parameters};
    
    PresetManager presetManager {parameters, &undoManager};
    
    gui::DarkModeSentinel darkModeSentinel {internals.guiDarkMode, *this};
};

}
