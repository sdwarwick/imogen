#pragma once

namespace Imogen
{
template < typename SampleType >
struct Compressor
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    Compressor (State& stateToUse);

    void process (AudioBuffer& dry, AudioBuffer& wet);

    void prepare (double samplerate, int blocksize);

private:
    void updateCompressorAmount (int amount);

    State&      state;
    Parameters& parameters {state.parameters};
    Meters&     meters {state.meters};

    dsp::FX::Compressor< SampleType > dryComp, wetComp;
};

}  // namespace Imogen
