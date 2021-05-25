
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
    State() : SerializableData ("ImogenState") { }

    void toValueTree (ValueTree& tree) override final
    {
        parameters.serialize (tree);
        internals.serialize (tree);
    }

    void fromValueTree (const ValueTree& tree) override final
    {
        parameters.deserialize (tree);
        internals.deserialize (tree);
    }
    
    void addTo (juce::AudioProcessor& p)
    {
        parameters.addParametersTo (p);
        internals.addAllParametersAsInternal();
        meters.addAllParametersAsInternal();
    }
    
    void addAllAsInternal()
    {
        parameters.addAllParametersAsInternal();
        internals.addAllParametersAsInternal();
        meters.addAllParametersAsInternal();
    }

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


}  // namespace Imogen
