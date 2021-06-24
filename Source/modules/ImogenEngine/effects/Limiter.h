#pragma once

namespace Imogen
{
template < typename SampleType >
struct Limiter
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    Limiter (State& stateToUse);

    void process (AudioBuffer& audio);

    void prepare (double samplerate, int blocksize);

private:
    State&      state;
    Parameters& parameters {state.parameters};
    Meters&     meters {state.meters};

    dsp::FX::Limiter< SampleType > limiter;
};

}  // namespace Imogen
