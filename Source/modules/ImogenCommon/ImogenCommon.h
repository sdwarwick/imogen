
/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 ImogenCommon
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenCommon
 description:        This Juce module declares some project-wide settings and data for Imogen as a whole.
 dependencies:       bv_midi bv_plugin bv_networking
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include <bv_midi/bv_midi.h>
#include <bv_plugin/bv_plugin.h>
#include <bv_networking/bv_networking.h>

#include "state/Parameters.h"
#include "state/Internals.h"
#include "state/Meters.h"


namespace Imogen
{
struct State : StateBase
{
    State() : StateBase ("ImogenState")
    {
        add (parameters, internals, meters);
    }

    Parameters parameters;
    Internals  internals;
    Meters     meters;
};


static inline juce::File presetsFolder()
{
    return getPresetsFolder ("Ben Vining Music Software", "Imogen");
}


static inline String getPresetFileExtension()
{
    return {".xml"};
}


static inline juce::File presetNameToFilePath (const String& presetName)
{
    return presetsFolder().getChildFile (addFileExtensionIfMissing (presetName, getPresetFileExtension()));
}


}  // namespace Imogen
