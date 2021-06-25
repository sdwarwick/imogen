namespace Imogen
{
template < typename SampleType >
EffectsManager< SampleType >::EffectsManager (State& stateToUse)
    : state (stateToUse)
{
}

template < typename SampleType >
void EffectsManager< SampleType >::prepare (double samplerate, int blocksize)
{
    processedMonoBuffer.setSize (1, blocksize, true, true, true);
    pannedLeadBuffer.setSize (2, blocksize, true, true, true);

    stereoReducer.prepare (samplerate, blocksize);
    initialLoCut.prepare (samplerate, blocksize);
    inputGain.prepare (samplerate, blocksize);
    gate.prepare (samplerate, blocksize);
    dryPanner.prepare (samplerate, blocksize);

    EQ.prepare (samplerate, blocksize);
    compressor.prepare (samplerate, blocksize);
    deEsser.prepare (samplerate, blocksize);

    dryWetMixer.prepare (samplerate, blocksize);
    delay.prepare (samplerate, blocksize);
    reverb.prepare (samplerate, blocksize);
    outputGain.prepare (samplerate, blocksize);
    limiter.prepare (samplerate, blocksize);
}

template < typename SampleType >
void EffectsManager< SampleType >::processPreHarmony (const AudioBuffer& input, bool leadIsBypassed)
{
    stereoReducer.process (input, processedMonoBuffer);
    initialLoCut.process (processedMonoBuffer);
    inputGain.process (processedMonoBuffer);
    gate.process (processedMonoBuffer);

    dryPanner.process (processedMonoBuffer, pannedLeadBuffer, leadIsBypassed);
}

template < typename SampleType >
void EffectsManager< SampleType >::processPostHarmony (AudioBuffer& harmonySignal, AudioBuffer& output)
{
    EQ.process (pannedLeadBuffer, harmonySignal);
    compressor.process (pannedLeadBuffer, harmonySignal);
    deEsser.process (pannedLeadBuffer, harmonySignal);

    dryWetMixer.process (pannedLeadBuffer, harmonySignal);

    delay.process (harmonySignal);
    reverb.process (harmonySignal);
    outputGain.process (harmonySignal);
    limiter.process (harmonySignal);

    dsp::buffers::copy (harmonySignal, output);
}

template < typename SampleType >
void EffectsManager< SampleType >::updateStereoWidth (int width)
{
    reverb.setWidth (static_cast< float > (width) * 0.01f);
}

template < typename SampleType >
const juce::AudioBuffer< SampleType >& EffectsManager< SampleType >::getProcessedInputSignal() const
{
    return processedMonoBuffer;
}

template class EffectsManager< float >;
template class EffectsManager< double >;

}  // namespace Imogen
