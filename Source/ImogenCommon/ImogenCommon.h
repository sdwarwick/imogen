
/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 ImogenCommon
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenCommon
 description:        This Juce module declares some project-wide settings and data for Imogen as a whole.
 dependencies:       bv_midi
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include "bv_midi/bv_midi.h"

#include "Parameters.h"
#include "Internals.h"
#include "Meters.h"


namespace Imogen
{

struct State : bav::SerializableData
{
    State();
    
    void toValueTree (ValueTree& tree) override final;
    void fromValueTree (const ValueTree& tree) override final;
    
    void addTo (juce::AudioProcessor& p);
    void addAllAsInternal();

    Parameters parameters;
    Internals  internals;
    Meters     meters;
};


static inline juce::File presetsFolder()
{
    return bav::getPresetsFolder ("Ben Vining Music Software", "Imogen");
}


static inline juce::String getPresetFileExtension()
{
    return {".xml"};
}


static inline juce::File presetNameToFilePath (const juce::String& presetName)
{
    return presetsFolder().getChildFile (bav::addFileExtensionIfMissing (presetName, getPresetFileExtension()));
}


}  // namespace Imogen
