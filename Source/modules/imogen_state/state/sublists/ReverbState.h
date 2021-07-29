#pragma once

namespace Imogen
{
struct ReverbState
{
    ReverbState (plugin::ParameterList& list);

    ToggleParam  reverbToggle {"Reverb toggle", false};
    PercentParam reverbDryWet {"Reverb mix", 15};
    PercentParam reverbDecay {"Reverb decay", 60};
    PercentParam reverbDuck {"Reverb duck", 30};
    HzParam      reverbLoCut {"Reverb lo cut", 80.f};
    HzParam      reverbHiCut {"Reverb hi cut", 5500.f};
};

}  // namespace Imogen
