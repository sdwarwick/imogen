/*
    Part of module: bv_ImogenEngine
    Direct parent file: bv_ImogenEngine.h
*/


#include "bv_ImogenEngineParameters.cpp"


#define bvie_LIMITER_THRESH_DB 0.0
#define bvie_LIMITER_RELEASE_MS 35

#define bvie_INIT_MIN_HZ 80
#define bvie_INIT_MAX_HZ 2400


namespace bav

{
    

template<typename SampleType>
ImogenEngine<SampleType>::ImogenEngine(): FIFOEngine(1),
    resourcesReleased(true), initialized(false)
{
    modulatorInput.store(0);
    
    wetMixPercent.store(SampleType(1.0));
    
    limiterIsOn.store(false);
    limiterThresh.store(1.0f);
    limiterRelease.store(20.0f);
    
    inputGain.store(1.0f);
    prevInputGain.store(1.0f);
    
    outputGain.store(1.0f);
    prevOutputGain.store(1.0f);
    
    dspSpec.numChannels = 2;
    dspSpec.sampleRate = 44100.0;
    dspSpec.maximumBlockSize = 512;
}

    
template<typename SampleType>
void ImogenEngine<SampleType>::initialize (const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices)
{
    jassert (initSamplerate > 0 && initSamplesPerBlock > 0 && initNumVoices > 0);
#if ! JUCE_DEBUG
    juce::ignoreUnused(initSamplesPerBlock);
#endif
    
    const ScopedLock sl (lock);
    
    harmonizer.initialize (initNumVoices, initSamplerate, initSamplesPerBlock);
    
    monoBuffer.setSize (1, initSamplesPerBlock);
    dryBuffer.setSize (2, initSamplesPerBlock);
    wetBuffer.setSize (2, initSamplesPerBlock);
    
    limiter.setRelease (SampleType(bvie_LIMITER_RELEASE_MS));
    limiter.setThreshold (SampleType(bvie_LIMITER_THRESH_DB));
    
    updatePitchDetectionHzRange (bvie_INIT_MIN_HZ, bvie_INIT_MAX_HZ);
    
    FIFOEngine::prepare (initSamplerate, initSamplesPerBlock);
    
    initialized = true;
}
    
#undef bvie_INIT_MIN_HZ
#undef bvie_INIT_MAX_HZ
    

template<typename SampleType>
void ImogenEngine<SampleType>::reset()
{
    const ScopedLock sl (lock);
    
    harmonizer.allNotesOff(false);
    
    dryWetMixer.reset();
    limiter.reset();
    
    monoBuffer.clear();
    
    prevOutputGain.store(outputGain.load());
    prevInputGain.store(inputGain.load());
}
    

template<typename SampleType>
void ImogenEngine<SampleType>::killAllMidi()
{
    const ScopedLock sl (lock);
    harmonizer.allNotesOff(false);
}


template<typename SampleType>
void ImogenEngine<SampleType>::playChord (const Array<int>& desiredNotes, const float velocity, const bool allowTailOffOfOld)
{
    const ScopedLock sl (lock);
    harmonizer.playChord (desiredNotes, velocity, allowTailOffOfOld);
}
    

template<typename SampleType>
void ImogenEngine<SampleType>::playIntervalSet (const Array<int>& desiredIntervals, const float velocity, const bool allowTailOffOfOld)
{
    const ScopedLock sl (lock);
    harmonizer.playIntervalSet (desiredIntervals, velocity, allowTailOffOfOld, false);
}
    

template<typename SampleType>
void ImogenEngine<SampleType>::returnActivePitches (Array<int>& outputArray) const
{
    const ScopedLock sl (lock);
    harmonizer.reportActiveNotes(outputArray);
}
    
    
template<typename SampleType>
void ImogenEngine<SampleType>::recieveExternalPitchbend (const int bend)
{
    const ScopedLock sl (lock);
    harmonizer.handlePitchWheel (bend);
}
    

template <typename SampleType>
void ImogenEngine<SampleType>::prepareToPlay (double samplerate, int blocksize)
{
    jassert (samplerate > 0);
    jassert (blocksize > 0);
#if ! JUCE_DEBUG
    juce::ignoreUnused (blocksize);
#endif
    
    const ScopedLock sl (lock);
    
    dspSpec.sampleRate = samplerate;
    dspSpec.numChannels = 2;
    
    const int before = FIFOEngine::getLatency();
    
    harmonizer.setCurrentPlaybackSampleRate (samplerate);
    
    FIFOEngine::changeLatency (harmonizer.getLatencySamples());
    
    if (before == FIFOEngine::getLatency())  // if the latency *didn't* change, still force a refresh of everything by calling this manually
        latencyChanged (before);
    
    prevOutputGain.store(outputGain.load());
    prevInputGain.store (inputGain.load());
}
    

template <typename SampleType>
void ImogenEngine<SampleType>::latencyChanged (int newInternalBlocksize)
{
    jassert (newInternalBlocksize == FIFOEngine::getLatency());
    
    harmonizer.prepare (newInternalBlocksize);
    
    dryBuffer.setSize  (2, newInternalBlocksize, true, true, true);
    wetBuffer.setSize  (2, newInternalBlocksize, true, true, true);
    monoBuffer.setSize (1, newInternalBlocksize, true, true, true);
    
    dspSpec.maximumBlockSize = uint32(newInternalBlocksize);
    dspSpec.numChannels = 2;
    
    limiter.prepare (dspSpec);
    dryWetMixer.prepare (dspSpec);
    dryWetMixer.setWetLatency(0);
    
    limiter.setRelease (SampleType(bvie_LIMITER_RELEASE_MS));
    limiter.setThreshold (SampleType(bvie_LIMITER_THRESH_DB));
    
    resourcesReleased = false;
}
    
#undef bvie_LIMITER_RELEASE_MS
#undef bvie_LIMITER_THRESH_DB

    
template <typename SampleType>
void ImogenEngine<SampleType>::release()
{
    const ScopedLock sl (lock);
    
    harmonizer.releaseResources();
    
    wetBuffer.setSize (0, 0, false, false, false);
    dryBuffer.setSize (0, 0, false, false, false);
    monoBuffer.setSize(0, 0, false, false, false);
    
    dryWetMixer.reset();
    limiter.reset();
    
    resourcesReleased = true;
    initialized       = false;
}

    
template <typename SampleType>
void ImogenEngine<SampleType>::renderBlock (const AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages)
{
    const ScopedLock sl (lock);
    
    const int blockSize = input.getNumSamples();
    
    jassert (blockSize == FIFOEngine::getLatency());
    jassert (blockSize == output.getNumSamples());
    
    switch (modulatorInput.load()) // isolate a mono input buffer from the input bus, mixing to mono if necessary
    {
        case (0): // take only the left channel
        {
            monoBuffer.copyFrom (0, 0, input, 0, 0, blockSize);
            break;
        }

        case (1):  // take only the right channel
        {
            monoBuffer.copyFrom (0, 0, input, (input.getNumChannels() > 1), 0, blockSize);
            break;
        }

        case (2):  // mix all input channels to mono
        {
            monoBuffer.copyFrom (0, 0, input, 0, 0, blockSize);
            
            const int totalNumChannels = input.getNumChannels();
            
            if (totalNumChannels == 1)
                break;

            for (int channel = 1; channel < totalNumChannels; ++channel)
                monoBuffer.addFrom (0, 0, input, channel, 0, blockSize);

            monoBuffer.applyGain (1.0f / totalNumChannels);
        }
            
        default:
        {
            monoBuffer.copyFrom (0, 0, input, 0, 0, blockSize);
            modulatorInput.store (0);
            break;
        }
    }
    
    // master input gain
    const float currentInGain = inputGain.load();
    monoBuffer.applyGainRamp (0, blockSize, prevInputGain.load(), currentInGain);
    prevInputGain.store(currentInGain);
    
    const AudioBuffer<SampleType> realInput = AudioBuffer<SampleType> (monoBuffer.getArrayOfWritePointers(), 1, blockSize);

    // write to dry buffer & apply panning
    for (int chan = 0; chan < 2; ++chan)
        dryBuffer.copyFromWithRamp (chan, 0, realInput.getReadPointer(0), blockSize,
                                    dryPanner.getPrevGain(chan), dryPanner.getGainMult(chan));
    
    dryWetMixer.setWetMixProportion (wetMixPercent.load());
    dryWetMixer.pushDrySamples ( juce::dsp::AudioBlock<SampleType>(dryBuffer) );

    // puts the harmonizer's rendered stereo output into wetBuffer & returns its midi output into midiMessages
    harmonizer.renderVoices (realInput, wetBuffer, midiMessages);
    
    dryWetMixer.mixWetSamples ( juce::dsp::AudioBlock<SampleType>(wetBuffer) ); // puts the mixed dry & wet samples into wetBuffer

    // master output gain
    const float currentOutGain = outputGain.load();
    wetBuffer.applyGainRamp (0, blockSize, prevOutputGain.load(), currentOutGain);
    prevOutputGain.store(currentOutGain);

    if (limiterIsOn.load())
    {
        juce::dsp::AudioBlock<SampleType> limiterBlock (wetBuffer);
        limiter.process (juce::dsp::ProcessContextReplacing<SampleType>(limiterBlock));
    }
    
    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, wetBuffer, chan, 0, blockSize);
}

    
template class ImogenEngine<float>;
template class ImogenEngine<double>;


} // namespace
