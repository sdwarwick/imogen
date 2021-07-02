namespace Imogen
{
template < typename SampleType >
Engine< SampleType >::Engine (State& stateToUse)
: state (stateToUse)
{
}

template < typename SampleType >
void Engine< SampleType >::renderChunk (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages, bool)
{
    output.clear();
    updateStereoWidth (parameters.stereoWidth->get());

    const bool leadIsBypassed       = parameters.leadBypass->get();
    const bool harmoniesAreBypassed = parameters.harmonyBypass->get();
    
    const auto numSamples = input.getNumSamples();

    if (leadIsBypassed && harmoniesAreBypassed)
    {
        harmonizer.bypassedBlock (numSamples, midiMessages);
        return;
    }

    preHarmonyEffects.process (input);

    analyzer.analyzeInput (preHarmonyEffects.getProcessedInputSignal(), numSamples);

    harmonizer.process (numSamples, midiMessages, harmoniesAreBypassed);

    leadProcessor.process (leadIsBypassed, numSamples);

    postHarmonyEffects.process (harmonizer.getHarmonySignal(), leadProcessor.getProcessedSignal(), output);
}

template < typename SampleType >
void Engine< SampleType >::updateStereoWidth (int width)
{
    harmonizer.updateStereoWidth (width);
    postHarmonyEffects.updateStereoWidth (width);
}

template < typename SampleType >
void Engine< SampleType >::onPrepare (int blocksize, double samplerate)
{
    if (! harmonizer.isInitialized())
        harmonizer.initialize (16, samplerate, blocksize);

    analyzer.prepare (samplerate, blocksize);

    if (const auto latency = analyzer.getLatencySamples() > 0)
    {
        dsp::LatencyEngine< SampleType >::changeLatency (latency);
        return;
    }

    harmonizer.prepare (samplerate, blocksize);
    leadProcessor.prepare (samplerate, blocksize);
    preHarmonyEffects.prepare (samplerate, blocksize);
    postHarmonyEffects.prepare (samplerate, blocksize);
}


template class Engine< float >;
template class Engine< double >;

}  // namespace Imogen
