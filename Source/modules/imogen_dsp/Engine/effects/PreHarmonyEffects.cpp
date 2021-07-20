
namespace Imogen
{
template < typename SampleType >
PreHarmonyEffects< SampleType >::PreHarmonyEffects (State& stateToUse)
    : state (stateToUse)
{
}

template < typename SampleType >
void PreHarmonyEffects< SampleType >::prepare (double samplerate, int blocksize)
{
    processedMonoBuffer.setSize (1, blocksize, true, true, true);

    stereoReducer.prepare (samplerate, blocksize);
    initialLoCut.prepare (samplerate, blocksize);
    inputGain.prepare (samplerate, blocksize);
    gate.prepare (samplerate, blocksize);
}

template < typename SampleType >
void PreHarmonyEffects< SampleType >::process (const AudioBuffer& input)
{
    stereoReducer.process (input, processedMonoBuffer);
    initialLoCut.process (processedMonoBuffer);
    inputGain.process (processedMonoBuffer);
    gate.process (processedMonoBuffer);
}

template < typename SampleType >
const SampleType* PreHarmonyEffects< SampleType >::getProcessedInputSignal() const
{
    return processedMonoBuffer.getReadPointer (0);
}

template class PreHarmonyEffects< float >;
template class PreHarmonyEffects< double >;

}  // namespace Imogen
