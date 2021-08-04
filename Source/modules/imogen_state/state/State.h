#pragma once

#include "Parameters.h"
#include "Meters.h"
#include "Internals.h"


namespace Imogen
{
struct State : plugin::State< Parameters >
{
    State();

    Internals internals;
    Meters    meters;
};

}  // namespace Imogen
