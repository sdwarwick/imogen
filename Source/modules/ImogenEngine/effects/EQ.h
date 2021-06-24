
#pragma once

namespace Imogen
{
template < typename SampleType >
struct EQ
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    EQ (Parameters& params);

    void process (AudioBuffer& dry, AudioBuffer& wet);

    void prepare (double samplerate, int blocksize);

private:
    Parameters& parameters;

    dsp::FX::EQ< SampleType > dryEQ, wetEQ;
};

}  // namespace Imogen
