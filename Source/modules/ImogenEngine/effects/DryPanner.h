#pragma once

namespace Imogen
{
template < typename SampleType >
struct DryPanner
{
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    DryPanner (Parameters& params);

    void process (const AudioBuffer& monoIn, AudioBuffer& stereoOut, bool bypassed);

    void prepare (double samplerate, int blocksize);

private:
    Parameters& parameters;

    dsp::FX::MonoToStereoPanner< SampleType > panner;
};

}  // namespace Imogen
