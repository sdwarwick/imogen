#pragma once

namespace Imogen
{
struct ReverbState
{
    ReverbState (plugin::ParameterList& list);

    ToggleParam  reverbToggle {"Reverb Toggle", "Reverb toggle", false};
    PercentParam reverbDryWet {"Reverb Mix", "Reverb mix", 15};
    PercentParam reverbDecay {"Reverb Decay", "Reverb decay", 60};
    PercentParam reverbDuck {"Reverb Duck", "Reverb duck", 30};
    HzParam      reverbLoCut {"Reverb Lo cut", "Reverb lo cut", 80.f};
    HzParam      reverbHiCut {"Reverb Hi cut", "Reverb hi cut", 5500.f};
};

}  // namespace Imogen
