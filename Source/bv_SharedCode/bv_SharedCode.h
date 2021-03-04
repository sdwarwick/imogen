/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_SharedCode
 vendor:             Ben Vining
 version:            0.0.1
 name:               Ben Vining's codebase
 description:        General utilities useful for developing plugins.
 dependencies:       juce_audio_utils
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

// dependency
#include <juce_audio_utils/juce_audio_utils.h>

// the rest of this module
#include "dsp/AudioFIFO.h"
#include "dsp/Panner.h"
#include "midi/MidiFIFO.h"
#include "midi/MidiUtilities.h"


namespace bav
{

namespace gui
{
    
    static juce::Button::ButtonState buttonStateFromBool (const bool isOn)
    {
        if (isOn)
            return juce::Button::ButtonState::buttonDown;

        return juce::Button::ButtonState::buttonNormal;
    }
    
}  // namespace gui

}  // namespace bav
