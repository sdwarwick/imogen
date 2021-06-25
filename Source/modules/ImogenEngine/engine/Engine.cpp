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

    stereoReducer.process (input, monoBuffer);
    initialLoCut.process (monoBuffer);
    inputGain.process (monoBuffer);
    gate.process (monoBuffer);

    dryPanner.process (monoBuffer, dryBuffer, leadIsBypassed);

    harmonizer.process (monoBuffer, wetBuffer, midiMessages, harmoniesAreBypassed);

    EQ.process (dryBuffer, wetBuffer);
    compressor.process (dryBuffer, wetBuffer);
    deEsser.process (dryBuffer, wetBuffer);

    dryWetMixer.process (dryBuffer, wetBuffer);

    delay.process (wetBuffer);
    reverb.process (wetBuffer);
    outputGain.process (wetBuffer);
    limiter.process (wetBuffer);

    dsp::buffers::copy (wetBuffer, output);
}

template < typename SampleType >
void Engine< SampleType >::updateStereoWidth (int width)
{
    harmonizer.updateStereoWidth (width);
    reverb.setWidth (static_cast< float > (width) * 0.01f);
}

template < typename SampleType >
void Engine< SampleType >::onPrepare (int blocksize, double samplerate)
{
    if (! harmonizer.isInitialized())
        harmonizer.initialize (16, samplerate, blocksize);

    harmonizer.setCurrentPlaybackSampleRate (samplerate);
    harmonizer.prepare (blocksize);

    if (const auto latency = harmonizer.getLatencySamples() > 0)
        dsp::LatencyEngine< SampleType >::changeLatency (latency);

    monoBuffer.setSize (1, blocksize, true, true, true);
    wetBuffer.setSize (2, blocksize, true, true, true);
    dryBuffer.setSize (2, blocksize, true, true, true);

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
void Engine< SampleType >::onRelease()
{
    harmonizer.releaseResources();

    monoBuffer.setSize (0, 0);
    wetBuffer.setSize (0, 0);
    dryBuffer.setSize (0, 0);
}


template class Engine< float >;
template class Engine< double >;

}  // namespace Imogen
