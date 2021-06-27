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

    const auto blockSize = input.getNumSamples();

    const bool leadIsBypassed       = parameters.leadBypass->get();
    const bool harmoniesAreBypassed = parameters.harmonyBypass->get();

    if (leadIsBypassed && harmoniesAreBypassed)
    {
        harmonizer.bypassedBlock (blockSize, midiMessages);
        return;
    }

    effects.processPreHarmony (input, leadIsBypassed);

    harmonizer.process (effects.getProcessedInputSignal(),
                        wetBuffer, midiMessages, harmoniesAreBypassed);

    effects.processPostHarmony (wetBuffer, output);
}

template < typename SampleType >
void Engine< SampleType >::updateStereoWidth (int width)
{
    harmonizer.updateStereoWidth (width);
    effects.updateStereoWidth (width);
}

template < typename SampleType >
void Engine< SampleType >::onPrepare (int blocksize, double samplerate)
{
    if (! harmonizer.isInitialized())
        harmonizer.initialize (16, samplerate, blocksize);

    harmonizer.prepare (samplerate, blocksize);

    if (const auto latency = harmonizer.getLatencySamples() > 0)
    {
        dsp::LatencyEngine< SampleType >::changeLatency (latency);
        return;
    }

    wetBuffer.setSize (2, blocksize, true, true, true);

    effects.prepare (samplerate, blocksize);
}

template < typename SampleType >
void Engine< SampleType >::onRelease()
{
    harmonizer.releaseResources();
    wetBuffer.setSize (0, 0);
}


template class Engine< float >;
template class Engine< double >;

}  // namespace Imogen
