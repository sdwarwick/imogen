
namespace Imogen
{
template < typename SampleType >
PitchCorrection< SampleType >::PitchCorrection (Harmonizer< SampleType >& harm, Internals& internalsToUse)
    : Base (harm.analyzer, harm.getPitchAdjuster()), internals (internalsToUse)
{
}

template < typename SampleType >
void PitchCorrection< SampleType >::renderNextFrame (int numSamples)
{
    alias.setDataToReferTo (correctedBuffer.getArrayOfWritePointers(), 1, numSamples);

    this->processNextFrame (alias);

    internals.currentInputNote->set (this->getOutputMidiPitch());
    internals.currentCentsSharp->set (this->getCentsSharp());
}

template < typename SampleType >
const juce::AudioBuffer< SampleType >& PitchCorrection< SampleType >::getCorrectedSignal() const
{
    return alias;
}

template < typename SampleType >
void PitchCorrection< SampleType >::prepare (double samplerate, int blocksize)
{
    correctedBuffer.setSize (1, blocksize, true, true, true);
    Base::prepare (samplerate);
}

template class PitchCorrection< float >;
template class PitchCorrection< double >;

}  // namespace Imogen
