#pragma once

#include "MidiSettingsPopup.h"

namespace Imogen
{

class KeyboardButton : public juce::Component
{
public:
    KeyboardButton();
    
private:
    gui::Popup<MidiSettingsPopup> popup;
};

}
