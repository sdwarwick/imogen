
#pragma once

namespace Imogen
{
template < typename SampleType >
struct InputGain
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    InputGain (State& stateToUse);

    void process (AudioBuffer& audio);

    void prepare (double samplerate, int blocksize);

private:
    State&      state;
    Parameters& parameters {state.parameters};
    Meters&     meters {state.meters};

    dsp::FX::SmoothedGain< SampleType, 1 > gain;
};

}  // namespace Imogen
