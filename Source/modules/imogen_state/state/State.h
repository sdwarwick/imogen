#pragma once

#include "Parameters.h"
#include "Meters.h"
#include "Internals.h"


namespace Imogen
{
struct State : PluginState
{
    State();
    
    void addTo (juce::AudioProcessor& processor) final;
    
    void addAllAsInternal();
    
    Parameters parameters;
    Internals  internals;
    Meters     meters;
};


struct PresetManager : PluginPresetManager
{
    using PluginPresetManager::PluginPresetManager;
    
    std::string getCompanyName() final;
    std::string getProductName() final;
    std::string getPresetFileExtension() final;
};


}  // namespace Imogen
