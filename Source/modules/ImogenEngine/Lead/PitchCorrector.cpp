
namespace Imogen
{
template < typename SampleType >
PitchCorrection< SampleType >::PitchCorrection (Harmonizer& harm, Internals& internalsToUse)
: dsp::psola::PitchCorrectorBase<SampleType>(harm.getAnalyzer(), harm.getPitchAdjuster()), internals (internalsToUse)
{
}

template < typename SampleType >
void PitchCorrection< SampleType >::renderNextFrame()
{
    this->processNextFrame (correctedBuffer);
}

template < typename SampleType >
const juce::AudioBuffer< SampleType >& PitchCorrection< SampleType >::getCorrectedSignal() const
{
    return correctedBuffer;
}

template class PitchCorrection< float >;
template class PitchCorrection< double >;

}  // namespace Imogen
