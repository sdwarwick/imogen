#pragma once

#include "InputIcon.h"
#include "OutputLevel/OutputLevel.h"
#include "PresetBar/PresetBar.h"
#include "ScaleChooser.h"
#include "AboutPopup/LogoButton.h"
#include "MidiSettings/KeyboardButton.h"

namespace Imogen
{

class Header : public juce::Component
{
public:
    Header (State& stateToUse, UndoManager& undoToUse);
    
private:
    void paint (juce::Graphics& g) final;
    void resized() final;
    
    State& state;
    UndoManager& undo;
    
    LogoButton logo;
    
    KeyboardButton keyboardButton;
    
    InputIcon   inputIcon {state};
    OutputLevel outputLevel {state};
    
    PresetBar presetBar {state, undo};
    
    ScaleChooser scale {state.internals};
};

}
