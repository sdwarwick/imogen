namespace Imogen
{
template < typename SampleType >
PostHarmonyEffects< SampleType >::PostHarmonyEffects (State& stateToUse)
    : state (stateToUse)
{
}

template < typename SampleType >
void PostHarmonyEffects< SampleType >::prepare (double samplerate, int blocksize)
{
    eq.prepare (samplerate, blocksize);
    compressor.prepare (samplerate, blocksize);
    deEsser.prepare (samplerate, blocksize);

    dryWetMixer.prepare (samplerate, blocksize);
    delay.prepare (samplerate, blocksize);
    reverb.prepare (samplerate, blocksize);
    outputGain.prepare (samplerate, blocksize);
    limiter.prepare (samplerate, blocksize);
}

template < typename SampleType >
void PostHarmonyEffects< SampleType >::process (AudioBuffer& harmonySignal, AudioBuffer& drySignal, AudioBuffer& output)
{
    eq.process (drySignal, harmonySignal);
    compressor.process (drySignal, harmonySignal);
    deEsser.process (drySignal, harmonySignal);

    dryWetMixer.process (drySignal, harmonySignal);

    delay.process (harmonySignal);
    reverb.process (harmonySignal);
    outputGain.process (harmonySignal);
    limiter.process (harmonySignal);

    dsp::buffers::copy (harmonySignal, output);
}

template < typename SampleType >
void PostHarmonyEffects< SampleType >::updateStereoWidth (int width)
{
    reverb.setWidth (static_cast< float > (width) * 0.01f);
}

template class PostHarmonyEffects< float >;
template class PostHarmonyEffects< double >;

}  // namespace Imogen
