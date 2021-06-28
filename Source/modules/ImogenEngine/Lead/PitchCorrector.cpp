
namespace Imogen
{
template < typename SampleType >
PitchCorrection< SampleType >::PitchCorrection (Synth& harm, Internals& internalsToUse, Analyzer& analyzerToUse)
    : internals (internalsToUse), analyzer (analyzerToUse), harmonizer (harm)
{
}

template < typename SampleType >
void PitchCorrection< SampleType >::prepare (double samplerate, int blocksize)
{
    sampleRate = samplerate;

    correctedBuffer.setSize (1, blocksize, true, true, true);
}

template < typename SampleType >
void PitchCorrection< SampleType >::processNextFrame()
{
    shifter.setPitch (getTargetFrequency(), sampleRate);
    shifter.getSamples (correctedBuffer);
}

template < typename SampleType >
float PitchCorrection< SampleType >::getTargetFrequency()
{
    const auto inputPitch = harmonizer.getMidiForFrequency (analyzer.getFrequency());
    return harmonizer.getFrequencyForMidi (juce::roundToInt (inputPitch));
}

template < typename SampleType >
const juce::AudioBuffer< SampleType >& PitchCorrection< SampleType >::getCorrectedSignal() const
{
    return correctedBuffer;
}

template class PitchCorrection< float >;
template class PitchCorrection< double >;

}  // namespace Imogen
