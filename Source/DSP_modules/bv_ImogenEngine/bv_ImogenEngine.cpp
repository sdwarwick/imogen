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
    harmonizer.allNotesOff(false);
    
    initialHiddenLoCut.reset();
    gate.reset();
    dryWetMixer.reset();
    limiter.reset();
    deEsser.reset();
    reverb.reset();
    
    monoBuffer.clear();
    
    resetSmoothedValues (FIFOEngine::getLatency());
}
    

bvie_VOID_TEMPLATE::resetSmoothedValues (int blocksize)
{
    inputGain.reset (blocksize);
    outputGain.reset (blocksize);
    dryLgain.reset (blocksize);
    dryRgain.reset (blocksize);
    harmonizer.resetRampedValues (blocksize);
}
    

bvie_VOID_TEMPLATE::killAllMidi()
{
    harmonizer.allNotesOff(false);
}


bvie_VOID_TEMPLATE::playChord (const Array<int>& desiredNotes, const float velocity, const bool allowTailOffOfOld)
{
    harmonizer.playChord (desiredNotes, velocity, allowTailOffOfOld);
}


bvie_VOID_TEMPLATE::returnActivePitches (Array<int>& outputArray) const
{
    harmonizer.reportActiveNotes(outputArray);
}
    
    
bvie_VOID_TEMPLATE::recieveExternalPitchbend (const int bend)
{
    harmonizer.handlePitchWheel (bend);
}
    

bvie_VOID_TEMPLATE::initialized (int newInternalBlocksize, double samplerate)
{
    jassert (samplerate > 0 && newInternalBlocksize > 0);

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
    
    resetSmoothedValues (newInternalBlocksize);
    
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
    
    resetSmoothedValues (newInternalBlocksize);
}
    
#undef bvie_LIMITER_RELEASE_MS
#undef bvie_LIMITER_THRESH_DB

    
bvie_VOID_TEMPLATE::release()
{
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
    
    inputGain.skip (numSamples);
    outputGain.skip (numSamples);
    dryLgain.skip (numSamples);
    dryRgain.skip (numSamples);
    
    harmonizer.bypassedBlock (input, midiMessages);
}

    
bvie_VOID_TEMPLATE::renderBlock (const AudioBuffer<SampleType>& input,
                                 AudioBuffer<SampleType>& output,
                                 MidiBuffer& midiMessages)
{
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
    
    auto* inputSamples = monoBuffer.getWritePointer(0);
    
    for (int s = 0; s < blockSize; ++s)
    {
        *(inputSamples + s) = inputSamples[s] * inputGain.getNextValue(); // master input gain
        *(inputSamples + s) = initialHiddenLoCut.processSample (inputSamples[s]);  //  initial lo cut
    }
    
    if (noiseGateIsOn.load())
        gate.process (monoBuffer);
    
    if (deEsserIsOn.load())
        deEsser.process (monoBuffer);

    if (compressorIsOn.load())
        compressor.process (monoBuffer, nullptr);

    dryBuffer.clear();
    
    if (! leadIsBypassed)  // write to dry buffer & apply panning
    {
        auto* writingL = dryBuffer.getWritePointer(0);
        auto* writingR = dryBuffer.getWritePointer(1);
        const auto* inputSamps = monoBuffer.getReadPointer(0);
        
        for (int s = 0; s < blockSize; ++s)
        {
            const auto sample = inputSamps[s];
            *(writingL + s) = sample * dryLgain.getNextValue();
            *(writingR + s) = sample * dryRgain.getNextValue();
        }
    }
    
    dryWetMixer.pushDrySamples ( juce::dsp::AudioBlock<SampleType>(dryBuffer) );

    wetBuffer.clear();
    
    if (harmoniesAreBypassed)
        harmonizer.bypassedBlock (monoBuffer, midiMessages);
    else
        harmonizer.renderVoices (monoBuffer, wetBuffer, midiMessages);
    
    dryWetMixer.mixWetSamples ( juce::dsp::AudioBlock<SampleType>(wetBuffer) ); // puts the mixed dry & wet samples into wetBuffer
    
    if (reverbIsOn.load())
        reverb.process (wetBuffer);
    
    auto* samples = monoBuffer.getWritePointer(0);
    
    // master output gain
    for (int s = 0; s < blockSize; ++s)
        *(samples + s) = samples[s] * outputGain.getNextValue();

    if (limiterIsOn.load())
        limiter.process (wetBuffer);
    
    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, wetBuffer, chan, 0, blockSize);
}
    

#undef bvie_VOID_TEMPLATE
    
template class ImogenEngine<float>;
template class ImogenEngine<double>;


} // namespace
