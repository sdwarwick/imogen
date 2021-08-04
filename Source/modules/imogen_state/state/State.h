#pragma once

#include "Parameters.h"
#include "Meters.h"
#include "Internals.h"


namespace Imogen
{
struct State : plugin::State
{
    State();

    plugin::ParameterList& getParameters() final;
    void                   addTo (juce::AudioProcessor& processor) final;
    void                   addAllAsInternal();

    void serialize (TreeReflector& ref) final;

    Parameters parameters;
    Internals  internals;
    Meters     meters;
};

}  // namespace Imogen
