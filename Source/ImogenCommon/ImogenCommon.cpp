
#include "ImogenCommon.h"

namespace Imogen
{

State::State()
    : SerializableData ("ImogenState")
{ }

void State::toValueTree (ValueTree& tree)
{
    parameters.serialize (tree);
    internals.serialize (tree);
}

void State::fromValueTree (const ValueTree& tree)
{
    parameters.deserialize (tree);
    internals.deserialize (tree);
}

void State::addTo (juce::AudioProcessor& p)
{
    parameters.addParametersTo (p);
    internals.addAllParametersAsInternal();
    meters.addAllParametersAsInternal();
}

void State::addAllAsInternal()
{
    parameters.addAllParametersAsInternal();
    internals.addAllParametersAsInternal();
    meters.addAllParametersAsInternal();
}

}  // namespace
