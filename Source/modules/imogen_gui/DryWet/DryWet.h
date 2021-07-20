#pragma once

namespace Imogen
{
class DryWet : public juce::Component
{
public:
    DryWet (State& stateToUse);

private:
    State& state;

    plugin::IntParameter& pcntWet {*state.parameters.dryWet};
};

}  // namespace Imogen
