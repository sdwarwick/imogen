#pragma once

namespace Imogen
{
template < typename SampleType >
struct DeEsser
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    DeEsser (State& stateToUse);

    void process (AudioBuffer& dry, AudioBuffer& wet);

    void prepare (double samplerate, int blocksize);

private:
    State&      state;
    Parameters& parameters {state.parameters};
    Meters&     meters {state.meters};

    dsp::FX::DeEsser< SampleType > dryDS, wetDS;
};

}  // namespace Imogen
