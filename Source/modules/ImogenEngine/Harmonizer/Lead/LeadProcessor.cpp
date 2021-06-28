
namespace Imogen
{
template < typename SampleType >
LeadProcessor< SampleType >::LeadProcessor (Synth& harm, State& stateToUse, Analyzer& analyzerToUse)
    : state (stateToUse), pitchCorrector (harm, state.internals, analyzerToUse)
{
}

template < typename SampleType >
void LeadProcessor< SampleType >::prepare (double samplerate, int blocksize)
{
    pannedLeadBuffer.setSize (2, blocksize, true, true, true);

    dryPanner.prepare (samplerate, blocksize);
    pitchCorrector.prepare (samplerate, blocksize);
}

template < typename SampleType >
void LeadProcessor< SampleType >::process (bool leadIsBypassed)
{
    pitchCorrector.processNextFrame();
    dryPanner.process (pitchCorrector.getCorrectedSignal(), pannedLeadBuffer, leadIsBypassed);
}

template < typename SampleType >
juce::AudioBuffer< SampleType >& LeadProcessor< SampleType >::getProcessedSignal()
{
    return pannedLeadBuffer;
}

template class LeadProcessor< float >;
template class LeadProcessor< double >;

}  // namespace Imogen
