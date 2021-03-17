/*
    Part of module: bv_ImogenEngine
    Direct parent file: bv_ImogenEngine.h
*/


#include "bv_ImogenEngineParameters.cpp"


#define bvie_LIMITER_THRESH_DB 0.0f
#define bvie_LIMITER_RELEASE_MS 35.0f

#define bvie_NOISE_GATE_ATTACK_MS 25.0f
#define bvie_NOISE_GATE_RELEASE_MS 100.0f
#define bvie_NOISE_GATE_FLOOR_RATIO_TO_ONE 10.0f

#define bvie_INIT_MIN_HZ 80
#define bvie_INIT_MAX_HZ 2400

#define bvie_INITIAL_HIDDEN_HI_PASS_FREQ 65

#define bvie_COMPRESSOR_ATTACK_MS 4.0f
#define bvie_COMPRESSOR_RELEASE_MS 200.0f


#define bvie_VOID_TEMPLATE template<typename SampleType> void ImogenEngine<SampleType>


namespace bav

{
    

template<typename SampleType>
ImogenEngine<SampleType>::ImogenEngine(): FIFOEngine()
{
    modulatorInput.store(0);
    
    limiterIsOn.store(false);
    
    inputGain.store(1.0f);
    prevInputGain.store(1.0f);
    
    outputGain.store(1.0f);
    prevOutputGain.store(1.0f);
    
    dspSpec.numChannels = 2;
    dspSpec.sampleRate = 44100.0;
    dspSpec.maximumBlockSize = 512;
    
    leadBypass.store (false);
    harmonyBypass.store (false);
    
    deEsserIsOn.store (false);
    reverbIsOn.store (false);
}
    

bvie_VOID_TEMPLATE::resetTriggered()
{
    const ScopedLock sl (lock);
    
    harmonizer.allNotesOff(false);
    
    initialHiddenLoCut.reset();
    gate.reset();
    dryWetMixer.reset();
    limiter.reset();
    deEsser.reset();
    reverb.reset();
    
    monoBuffer.clear();
    
    prevOutputGain.store(outputGain.load());
    prevInputGain.store(inputGain.load());
}
    

bvie_VOID_TEMPLATE::killAllMidi()
{
    const ScopedLock sl (lock);
    harmonizer.allNotesOff(false);
}


bvie_VOID_TEMPLATE::playChord (const Array<int>& desiredNotes, const float velocity, const bool allowTailOffOfOld)
{
    const ScopedLock sl (lock);
    harmonizer.playChord (desiredNotes, velocity, allowTailOffOfOld);
}


bvie_VOID_TEMPLATE::returnActivePitches (Array<int>& outputArray) const
{
    const ScopedLock sl (lock);
    harmonizer.reportActiveNotes(outputArray);
}
    
    
bvie_VOID_TEMPLATE::recieveExternalPitchbend (const int bend)
{
    const ScopedLock sl (lock);
    harmonizer.handlePitchWheel (bend);
}
    

bvie_VOID_TEMPLATE::initialized (int newInternalBlocksize, double samplerate)
{
    jassert (samplerate > 0 && newInternalBlocksize > 0);

    const ScopedLock sl (lock);
    
    harmonizer.initialize (12, samplerate, newInternalBlocksize);
    
    monoBuffer.setSize (1, newInternalBlocksize);
    dryBuffer.setSize (2, newInternalBlocksize);
    wetBuffer.setSize (2, newInternalBlocksize);
    
    // constant limiter settings
    limiter.setRelease (bvie_LIMITER_RELEASE_MS);
    limiter.setThreshold (bvie_LIMITER_THRESH_DB);
    
    // constant noise gate settings
    gate.setRatio (bvie_NOISE_GATE_FLOOR_RATIO_TO_ONE);
    gate.setAttack (bvie_NOISE_GATE_ATTACK_MS);
    gate.setRelease (bvie_NOISE_GATE_RELEASE_MS);
    
    // constant compressor settings
    compressor.setAttack (bvie_COMPRESSOR_ATTACK_MS);
    compressor.setRelease (bvie_COMPRESSOR_RELEASE_MS);
    
    deEsser.prepare (newInternalBlocksize, samplerate);
    
    reverb.prepare (newInternalBlocksize, samplerate, 2);
    
    updatePitchDetectionHzRange (bvie_INIT_MIN_HZ, bvie_INIT_MAX_HZ);
}
    
#undef bvie_LIMITER_RELEASE_MS
#undef bvie_LIMITER_THRESH_DB
#undef bvie_NOISE_GATE_FLOOR_RATIO_TO_ONE
#undef bvie_NOISE_GATE_ATTACK_MS
#undef bvie_NOISE_GATE_RELEASE_MS
#undef bvie_INIT_MIN_HZ
#undef bvie_INIT_MAX_HZ
#undef bvie_COMPRESSOR_ATTACK_MS
#undef bvie_COMPRESSOR_RELEASE_MS
    

bvie_VOID_TEMPLATE::prepareToPlay (double samplerate)
{
    jassert (samplerate > 0);
    
    const ScopedLock sl (lock);
    
    dspSpec.sampleRate = samplerate;
    dspSpec.numChannels = 2;
    
    harmonizer.setCurrentPlaybackSampleRate (samplerate);
    
    if (harmonizer.getLatencySamples() != FIFOEngine::getLatency())
        FIFOEngine::changeLatency (harmonizer.getLatencySamples());
    
    const int blocksize = FIFOEngine::getLatency();
    
    initialHiddenLoCut.prepare(dspSpec);
    
    gate.prepare (1, blocksize, samplerate);
    
    dryWetMixer.setWetLatency(0);
    dryWetMixer.prepare (dspSpec);
    
    limiter.prepare (blocksize, samplerate, 2);
    
    compressor.prepare (blocksize, samplerate, 1);
    
    prevOutputGain.store(outputGain.load());
    prevInputGain.store (inputGain.load());
    
    initialHiddenLoCut.coefficients = juce::dsp::IIR::Coefficients<SampleType>::makeLowPass (samplerate,
                                                                                             SampleType(bvie_INITIAL_HIDDEN_HI_PASS_FREQ));
    initialHiddenLoCut.reset();
    
    deEsser.prepare (blocksize, samplerate);
    
    reverb.prepare (blocksize, samplerate, 2);
}
    
#undef bvie_INITIAL_HIDDEN_HI_PASS_FREQ
    

bvie_VOID_TEMPLATE::latencyChanged (int newInternalBlocksize)
{
    jassert (newInternalBlocksize == FIFOEngine::getLatency());
    
    harmonizer.prepare (newInternalBlocksize);
    
    dryBuffer.setSize  (2, newInternalBlocksize, true, true, true);
    wetBuffer.setSize  (2, newInternalBlocksize, true, true, true);
    monoBuffer.setSize (1, newInternalBlocksize, true, true, true);
    
    dspSpec.maximumBlockSize = uint32(newInternalBlocksize);
}
    
#undef bvie_LIMITER_RELEASE_MS
#undef bvie_LIMITER_THRESH_DB

    
bvie_VOID_TEMPLATE::release()
{
    const ScopedLock sl (lock);
    
    harmonizer.releaseResources();
    
    wetBuffer.setSize (0, 0, false, false, false);
    dryBuffer.setSize (0, 0, false, false, false);
    monoBuffer.setSize(0, 0, false, false, false);
    
    initialHiddenLoCut.reset();
    gate.reset();
    dryWetMixer.reset();
    limiter.reset();
    deEsser.reset();
    reverb.reset();
}
    

bvie_VOID_TEMPLATE::bypassedBlock (const AudioBuffer<SampleType>& input, MidiBuffer& midiMessages)
{
    harmonizer.processMidi (midiMessages);
    
    const int numSamples = input.getNumSamples();
    
    jassert (numSamples == FIFOEngine::getLatency());
}

    
bvie_VOID_TEMPLATE::renderBlock (const AudioBuffer<SampleType>& input,
                                 AudioBuffer<SampleType>& output,
                                 MidiBuffer& midiMessages)
{
    const ScopedLock sl (lock);
    
    const int blockSize = input.getNumSamples();
    
    jassert (blockSize == FIFOEngine::getLatency());
    jassert (blockSize == output.getNumSamples());
    
    const bool leadIsBypassed = leadBypass.load();
    const bool harmoniesAreBypassed = harmonyBypass.load();
    
    output.clear();
    
    if (leadIsBypassed && harmoniesAreBypassed)
    {
        harmonizer.processMidi (midiMessages);
        return;
    }
    
    switch (modulatorInput.load()) // isolate a mono input buffer from the input bus, mixing to mono if necessary
    {
        case (1): // take only the left channel
        {
            monoBuffer.copyFrom (0, 0, input, 0, 0, blockSize);
            break;
        }

        case (2):  // take only the right channel
        {
            monoBuffer.copyFrom (0, 0, input, (input.getNumChannels() > 1), 0, blockSize);
            break;
        }

        case (3):  // mix all input channels to mono
        {
            monoBuffer.copyFrom (0, 0, input, 0, 0, blockSize);
            
            const int totalNumChannels = input.getNumChannels();
            
            if (totalNumChannels == 1)
                break;

            for (int channel = 1; channel < totalNumChannels; ++channel)
                monoBuffer.addFrom (0, 0, input, channel, 0, blockSize);

            monoBuffer.applyGain (1.0f / totalNumChannels);
            break;
        }
            
        default:
        {
            monoBuffer.copyFrom (0, 0, input, 0, 0, blockSize);
            modulatorInput.store (1);
            break;
        }
    }
    
    // master input gain
    const float currentInGain = inputGain.load();
    monoBuffer.applyGainRamp (0, blockSize, prevInputGain.load(), currentInGain);
    prevInputGain.store(currentInGain);
    
    // initial hi-pass filter (hidden from the user)
    juce::dsp::AudioBlock<SampleType> inblock (monoBuffer);
    initialHiddenLoCut.process ( juce::dsp::ProcessContextReplacing<SampleType> (inblock) );
    
    if (noiseGateIsOn.load())
        gate.process (monoBuffer, nullptr);
    
    if (deEsserIsOn.load())
        deEsser.process (monoBuffer);

    if (compressorIsOn.load())
        compressor.process (monoBuffer, nullptr);

    dryBuffer.clear();
    
    if (! leadIsBypassed)  // write to dry buffer & apply panning
        for (int chan = 0; chan < 2; ++chan)
//            dryBuffer.copyFromWithRamp (chan, 0, monoBuffer.getReadPointer(0), blockSize,
//                                        dryPanner.getPrevGain(chan), dryPanner.getGainMult(chan));
    
    dryWetMixer.pushDrySamples ( juce::dsp::AudioBlock<SampleType>(dryBuffer) );

    wetBuffer.clear();
    
    if (harmoniesAreBypassed)
        harmonizer.processMidi (midiMessages);
    else
        harmonizer.renderVoices (monoBuffer, wetBuffer, midiMessages);
    
    dryWetMixer.mixWetSamples ( juce::dsp::AudioBlock<SampleType>(wetBuffer) ); // puts the mixed dry & wet samples into wetBuffer
    
    if (reverbIsOn.load())
        reverb.process (wetBuffer);

    // master output gain
    const float currentOutGain = outputGain.load();
    wetBuffer.applyGainRamp (0, blockSize, prevOutputGain.load(), currentOutGain);
    prevOutputGain.store(currentOutGain);

    if (limiterIsOn.load())
        limiter.process (wetBuffer);
    
    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, wetBuffer, chan, 0, blockSize);
}
    

#undef bvie_VOID_TEMPLATE
    
template class ImogenEngine<float>;
template class ImogenEngine<double>;


} // namespace
