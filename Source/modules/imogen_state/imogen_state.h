
#pragma once

#if 0

 BEGIN_JUCE_MODULE_DECLARATION

 ID:                 imogen_state
 vendor:             Ben Vining
 version:            0.0.1
 name:               imogen_state
 description:        Imogen's shared state
 dependencies:       bv_plugin bv_networking

 END_JUCE_MODULE_DECLARATION

#endif


#include <bv_plugin/bv_plugin.h>
#include <bv_networking/bv_networking.h>

#include "state/Parameters.h"
#include "state/Internals.h"
#include "state/Meters.h"


namespace Imogen
{
struct State : PluginState
{
    State() : PluginState ("ImogenState")
    {
        addDataChild (parameters, internals, meters);
    }
    
    ParameterList& getParameters() final { return parameters; }
    
    void addTo (juce::AudioProcessor& processor) final
    {
        parameters.addParametersTo (processor);
        meters.addParametersTo (processor);
        internals.addAllParametersAsInternal();
    }
    
    void addAllAsInternal()
    {
        parameters.addAllParametersAsInternal();
        meters.addAllParametersAsInternal();
        internals.addAllParametersAsInternal();
    }
    
    Parameters parameters;
    Internals  internals;
    Meters     meters;
};


struct PresetManager : PluginPresetManager
{
    using PluginPresetManager::PluginPresetManager;

    std::string getCompanyName() final { return "Ben Vining Music Software"; }
    std::string getProductName() final { return "Imogen"; }
    std::string getPresetFileExtension() final { return ".xml"; }
};


}  // namespace Imogen
