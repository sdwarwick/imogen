#pragma once

#include "Parameters.h"
#include "Meters.h"
#include "Internals.h"


namespace Imogen
{
struct State : plugin::State
{
    State();
    
    void addTo (juce::AudioProcessor& processor) final;
    void addAllAsInternal();
    
    void serialize (TreeReflector& ref) final;
    
    Parameters parameters;
    Internals  internals;
    Meters     meters;
};


struct PresetManager : plugin::PresetManagerBase
{
    using PresetManagerBase::PresetManagerBase;
    
    std::string getCompanyName() final;
    std::string getProductName() final;
    std::string getPresetFileExtension() final;
};


}  // namespace Imogen
