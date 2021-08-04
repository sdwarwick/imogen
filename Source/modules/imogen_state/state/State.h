#pragma once

#include "Parameters.h"
#include "Meters.h"
#include "Internals.h"


namespace Imogen
{
struct State : plugin::State< Parameters >
{
    State();

    void addTo (juce::AudioProcessor& processor) final;
    void addAllAsInternal();

    void serialize (TreeReflector& ref) final;

    Internals internals;
    Meters    meters;
};

}  // namespace Imogen
