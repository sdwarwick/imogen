#pragma once

namespace Imogen
{
template < typename SampleType >
class PitchCorrection
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using Analyzer    = dsp::psola::Analyzer< SampleType >;
    using Synth       = dsp::SynthBase< SampleType >;

    PitchCorrection (Synth& harm, Internals& internalsToUse, Analyzer& analyzerToUse);

    void processNextFrame();

    void prepare (double samplerate, int blocksize);

    const AudioBuffer& getCorrectedSignal() const;

private:
    float getTargetFrequency();

    Internals& internals;
    Analyzer&  analyzer;

    Synth& harmonizer;

    AudioBuffer correctedBuffer;

    dsp::psola::Shifter< SampleType > shifter {analyzer};

    double sampleRate {0.};
};

}  // namespace Imogen
