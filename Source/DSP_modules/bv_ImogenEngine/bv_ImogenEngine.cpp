
/*======================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 bv_ImogenEngine.cpp: This file defines the ImogenEngine's core implementation details: audio rendering, prepare/release methods, etc.
 
======================================================================================================================================================*/



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
    
    delayIsOn.store (false);
    
    dryWetMixer.setMixingRule (juce::dsp::DryWetMixingRule::balanced);
    dryWetMixer.setWetLatency (SampleType(0.0));
}
    

bvie_VOID_TEMPLATE::resetTriggered()
{
    harmonizer.reset();
    
    initialHiddenLoCut.reset();
    gate.reset();
    dryWetMixer.reset();
    limiter.reset();
    deEsser.reset();
    reverb.reset();
    delay.reset();
    
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


bvie_VOID_TEMPLATE::playChord (const juce::Array<int>& desiredNotes, const float velocity, const bool allowTailOffOfOld)
{
    harmonizer.playChord (desiredNotes, velocity, allowTailOffOfOld);
}


bvie_VOID_TEMPLATE::returnActivePitches (juce::Array<int>& outputArray) const
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

    harmonizer.initialize (16, samplerate, newInternalBlocksize);
    
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
    
    delay.prepare (newInternalBlocksize, samplerate, 2);
    
    resetSmoothedValues (newInternalBlocksize);
           
    harmonizer.updatePitchDetectionHzRange (bvie_INIT_MIN_HZ, bvie_INIT_MAX_HZ);

    FIFOEngine::changeLatency (harmonizer.getLatencySamples());       
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
    
    const auto blocksize = FIFOEngine::getLatency();
    
    harmonizer.prepare (blocksize);
    
    initialHiddenLoCut.prepare(dspSpec);
    
    gate.prepare (1, blocksize, samplerate);
    
    dryWetMixer.prepare (dspSpec);
    
    limiter.prepare (blocksize, samplerate, 2);
    
    compressor.prepare (blocksize, samplerate, 1);
    
    delay.prepare (blocksize, samplerate, 2);
    
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
    

bvie_VOID_TEMPLATE::bypassedBlock (const AudioBuffer& input, MidiBuffer& midiMessages)
{
    const auto numSamples = input.getNumSamples();
    
    jassert (numSamples == FIFOEngine::getLatency());
    
    inputGain.skip (numSamples);
    outputGain.skip (numSamples);
    dryLgain.skip (numSamples);
    dryRgain.skip (numSamples);
    
    harmonizer.bypassedBlock (numSamples, midiMessages);
}
         

    
bvie_VOID_TEMPLATE::renderBlock (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages)
{
    const auto blockSize = input.getNumSamples();

    jassert (blockSize == FIFOEngine::getLatency() && blockSize == output.getNumSamples() && blockSize == wetBuffer.getNumSamples());

    const bool leadIsBypassed = leadBypass.load();
    const bool harmoniesAreBypassed = harmonyBypass.load();

    output.clear();

    if (leadIsBypassed && harmoniesAreBypassed)
    {
        harmonizer.bypassedBlock (blockSize, midiMessages);
        return;
    }
    
    switch (modulatorInput.load())  //  isolate a mono input buffer from the input bus, mixing to mono if necessary
    {
        case (2):  // take only the right channel
        {
            monoBuffer.copyFrom (0, 0, input, (input.getNumChannels() > 1), 0, blockSize);
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
        }

        default:  // take only the left channel
        {
            monoBuffer.copyFrom (0, 0, input, 0, 0, blockSize);
        }
    }
    
    inputGain.applyGain (monoBuffer, blockSize);

//    juce::dsp::AudioBlock<SampleType> monoBlock (monoBuffer);
//    initialHiddenLoCut.process ( juce::dsp::ProcessContextReplacing<SampleType>(monoBlock) );

    if (noiseGateIsOn.load())
        gate.process (monoBuffer);

    if (deEsserIsOn.load())
        deEsser.process (monoBuffer);

    if (compressorIsOn.load())
        compressor.process (monoBuffer);

    dryBuffer.clear();

    if (! leadIsBypassed)  //  write to dry buffer & apply panning
    {
        dryBuffer.copyFrom (0, 0, monoBuffer, 0, 0, blockSize);
        dryBuffer.copyFrom (1, 0, monoBuffer, 0, 0, blockSize);
        dryLgain.applyGain (dryBuffer.getWritePointer(0), blockSize);
        dryRgain.applyGain (dryBuffer.getWritePointer(1), blockSize);
    }

    dryWetMixer.pushDrySamples ( juce::dsp::AudioBlock<SampleType>(dryBuffer) );

    wetBuffer.clear();

    if (harmoniesAreBypassed)
        harmonizer.bypassedBlock (blockSize, midiMessages);
    else
        harmonizer.render (monoBuffer, wetBuffer, midiMessages);  // renders the stereo output into wetBuffer

    dryWetMixer.mixWetSamples ( juce::dsp::AudioBlock<SampleType>(wetBuffer) ); // puts the mixed dry & wet samples into wetBuffer
    
    if (delayIsOn.load())
        delay.process (wetBuffer);

    if (reverbIsOn.load())
        reverb.process (wetBuffer);

    outputGain.applyGain (wetBuffer, blockSize);

    if (limiterIsOn.load())
        limiter.process (wetBuffer);
    
    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, wetBuffer, chan, 0, blockSize);
}
    

#undef bvie_VOID_TEMPLATE
    
template class ImogenEngine<float>;
template class ImogenEngine<double>;


} // namespace
