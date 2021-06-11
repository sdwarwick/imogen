
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


struct PresetManager : PresetManagerBase
{
    using PresetManagerBase::PresetManagerBase;

    std::string getCompanyName() final { return "Ben Vining Music Software"; }
    std::string getProductName() final { return "Imogen"; }
    std::string getPresetFileExtension() final { return ".xml"; }
};


}  // namespace Imogen
