
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
    using FT = dsp::FX::FilterType;

    void updateLowShelf (float freq, float Q, float gain);
    void updateHighShelf (float freq, float Q, float gain);
    void updatePeak (float freq, float Q, float gain);
    void updateHighPass (float freq, float Q);

    Parameters& parameters;

    dsp::FX::EQ< SampleType > dryEQ, wetEQ;
};

}  // namespace Imogen
