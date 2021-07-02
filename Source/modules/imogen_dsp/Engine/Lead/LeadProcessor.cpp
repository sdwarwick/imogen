
namespace Imogen
{
template < typename SampleType >
LeadProcessor< SampleType >::LeadProcessor (Harmonizer< SampleType >& harm, State& stateToUse)
    : pitchCorrector (harm, stateToUse.internals), dryPanner(stateToUse.parameters)
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
void LeadProcessor< SampleType >::process (bool leadIsBypassed, int numSamples)
{
    pitchCorrector.renderNextFrame (numSamples);
    dryPanner.process (pitchCorrector.getCorrectedSignal(), pannedLeadBuffer, leadIsBypassed);
    lastBlocksize = numSamples;
}

template < typename SampleType >
juce::AudioBuffer< SampleType >& LeadProcessor< SampleType >::getProcessedSignal()
{
    alias.setDataToReferTo (pannedLeadBuffer.getArrayOfWritePointers(), 2, lastBlocksize);
    return alias;
}

template class LeadProcessor< float >;
template class LeadProcessor< double >;

}  // namespace Imogen
