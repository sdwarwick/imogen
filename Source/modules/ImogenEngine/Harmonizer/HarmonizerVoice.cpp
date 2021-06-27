
namespace Imogen
{
template < typename SampleType >
HarmonizerVoice< SampleType >::HarmonizerVoice (Harmonizer< SampleType >& h, dsp::psola::Analyzer< SampleType >& analyzerToUse)
    : dsp::SynthVoiceBase< SampleType > (&h), shifter (analyzerToUse)
{
}

template < typename SampleType >
void HarmonizerVoice< SampleType >::renderPlease (AudioBuffer& output, float desiredFrequency, double currentSamplerate)
{
    jassert (desiredFrequency > 0 && currentSamplerate > 0);

    shifter.setPitch (desiredFrequency, currentSamplerate);
    shifter.getSamples (output.getWritePointer (0), output.getNumSamples());
}

template class HarmonizerVoice< float >;
template class HarmonizerVoice< double >;


}  // namespace Imogen
