
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

#include "ImogenParameters.h"


namespace Imogen
{


struct State  :     bav::SerializableData
{
    State() : SerializableData ("ImogenState") { }
    
    void toValueTree (ValueTree& tree) override final
    {
        parameters.serialize (tree);
    }
    
    void fromValueTree (const ValueTree& tree) override final
    {
        parameters.deserialize (tree);
    }
    
    Parameters parameters;
    Meters meters;
};


static inline juce::File presetsFolder()
{
    return bav::getPresetsFolder ("Ben Vining Music Software", "Imogen");
}


static inline juce::File findAppropriateTranslationFile()
{
    // things to test:
    // juce::SystemStats::getDisplayLanguage()
    // juce::SystemStats::getUserLanguage()
    // juce::SystemStats::getUserRegion()
    return {};
}


static inline juce::String getPresetFileExtension()
{
    return {".xml"};
}


} // namespace Imogen
