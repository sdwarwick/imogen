
#include "ImogenEngine.h"

#include "Harmonizer/Harmonizer.cpp"
#include "Harmonizer/HarmonizerVoice/HarmonizerVoice.cpp"


namespace Imogen
{
template < typename SampleType >
Engine< SampleType >::Engine()
{
    limiterIsOn.store (false);

    dspSpec.numChannels      = 2;
    dspSpec.sampleRate       = 44100.0;
    dspSpec.maximumBlockSize = 512;

    leadBypass.store (false);
    harmonyBypass.store (false);

    deEsserIsOn.store (false);
    reverbIsOn.store (false);

    delayIsOn.store (false);

    dryWetMixer.setMixingRule (juce::dsp::DryWetMixingRule::balanced);
    dryWetMixer.setWetLatency (SampleType (0.0));
}


template < typename SampleType >
void Engine< SampleType >::resetSmoothedValues()
{
    harmonizer.resetRampedValues();
    inputGain.reset();
    outputGain.reset();
    dryPanner.reset();
}


template < typename SampleType >
void Engine< SampleType >::killAllMidi()
{
    harmonizer.allNotesOff (false);
}


template < typename SampleType >
void Engine< SampleType >::playChord (const juce::Array< int >& desiredNotes, const float velocity, const bool allowTailOffOfOld)
{
    harmonizer.playChord (desiredNotes, velocity, allowTailOffOfOld);
}


template < typename SampleType >
void Engine< SampleType >::returnActivePitches (juce::Array< int >& outputArray) const
{
    harmonizer.reportActiveNotes (outputArray);
}


template < typename SampleType >
void Engine< SampleType >::recieveExternalPitchbend (const int bend)
{
    harmonizer.processMidiEvent (juce::MidiMessage::pitchWheel (1, bend));
}


template < typename SampleType >
void Engine< SampleType >::initialized (int newInternalBlocksize, double samplerate)
{
    jassert (samplerate > 0 && newInternalBlocksize > 0);

    harmonizer.initialize (16, samplerate, newInternalBlocksize);

    monoBuffer.setSize (1, newInternalBlocksize);
    dryBuffer.setSize (2, newInternalBlocksize);
    wetBuffer.setSize (2, newInternalBlocksize);

    // constant limiter settings
    limiter.setRelease (limiterReleaseMs);
    limiter.setThreshold (limiterThreshDb);

    // constant noise gate settings
    gate.setRatio (noiseGateFloorRatio);
    gate.setAttack (noiseGateAttackMs);
    gate.setRelease (noiseGateReleaseMs);

    // constant compressor settings
    compressor.setAttack (compressorAttackMs);
    compressor.setRelease (compressorReleaseMs);

    deEsser.prepare (newInternalBlocksize, samplerate);

    reverb.prepare (newInternalBlocksize, samplerate, 2);

    delay.prepare (newInternalBlocksize, samplerate, 2);

    resetSmoothedValues();

    FIFOEngine::changeLatency (harmonizer.getLatencySamples());
}


template < typename SampleType >
void Engine< SampleType >::prepareToPlay (double samplerate)
{
    jassert (samplerate > 0);

    dspSpec.sampleRate  = samplerate;
    dspSpec.numChannels = 2;

    harmonizer.setCurrentPlaybackSampleRate (samplerate);

    if (harmonizer.getLatencySamples() != FIFOEngine::getLatency()) FIFOEngine::changeLatency (harmonizer.getLatencySamples());

    const auto blocksize = FIFOEngine::getLatency();

    harmonizer.prepare (blocksize);

    initialHiddenLoCut.prepare (dspSpec);

    gate.prepare (1, blocksize, samplerate);

    dryWetMixer.prepare (dspSpec);

    limiter.prepare (blocksize, samplerate, 2);

    compressor.prepare (blocksize, samplerate, 1);

    delay.prepare (blocksize, samplerate, 2);

    initialHiddenLoCut.coefficients = juce::dsp::IIR::Coefficients< SampleType >::makeLowPass (samplerate, initialHiddenHiPassFreq);
    initialHiddenLoCut.reset();

    deEsser.prepare (blocksize, samplerate);

    reverb.prepare (blocksize, samplerate, 2);

    stereoReducer.prepare (blocksize);

    inputGain.prepare (blocksize);
    outputGain.prepare (blocksize);

    dryPanner.prepare (blocksize);
}


template < typename SampleType >
void Engine< SampleType >::latencyChanged (int newInternalBlocksize)
{
    jassert (newInternalBlocksize == FIFOEngine::getLatency());

    harmonizer.prepare (newInternalBlocksize);

    dryBuffer.setSize (2, newInternalBlocksize, true, true, true);
    wetBuffer.setSize (2, newInternalBlocksize, true, true, true);
    monoBuffer.setSize (1, newInternalBlocksize, true, true, true);

    dspSpec.maximumBlockSize = uint32 (newInternalBlocksize);

    resetSmoothedValues();
}


template < typename SampleType >
void Engine< SampleType >::release()
{
    harmonizer.releaseResources();

    wetBuffer.setSize (0, 0, false, false, false);
    dryBuffer.setSize (0, 0, false, false, false);
    monoBuffer.setSize (0, 0, false, false, false);

    initialHiddenLoCut.reset();
    gate.reset();
    dryWetMixer.reset();
    limiter.reset();
    deEsser.reset();
    reverb.reset();

    inputGain.reset();
    outputGain.reset();
    dryPanner.reset();
}


template < typename SampleType >
void Engine< SampleType >::bypassedBlock (const AudioBuffer& input, MidiBuffer& midiMessages)
{
    const auto numSamples = input.getNumSamples();

    jassert (numSamples == FIFOEngine::getLatency());

    harmonizer.bypassedBlock (numSamples, midiMessages);
}


template < typename SampleType >
void Engine< SampleType >::renderBlock (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages)
{
    resetMeterData();

    const auto blockSize = input.getNumSamples();

    jassert (blockSize == FIFOEngine::getLatency() && blockSize == output.getNumSamples() && blockSize == wetBuffer.getNumSamples());

    const bool leadIsBypassed       = leadBypass.load();
    const bool harmoniesAreBypassed = harmonyBypass.load();

    output.clear();

    if (leadIsBypassed && harmoniesAreBypassed)
    {
        harmonizer.bypassedBlock (blockSize, midiMessages);
        return;
    }

    stereoReducer.convertStereoToMono (input, monoBuffer);

    meterData.inputLevel = static_cast< float > (monoBuffer.getMagnitude (0, blockSize));

    inputGain.process (monoBuffer);

    //    juce::dsp::AudioBlock<SampleType> monoBlock (monoBuffer);
    //    initialHiddenLoCut.process ( juce::dsp::ProcessContextReplacing<SampleType>(monoBlock) );

    if (noiseGateIsOn.load())
    {
        SampleType gainRedux;
        gate.process (monoBuffer, &gainRedux);
        meterData.noiseGateGainReduction = static_cast< float > (gainRedux);
    }

    if (deEsserIsOn.load())
    {
        SampleType gainRedux;
        deEsser.process (monoBuffer, &gainRedux);
        meterData.deEsserGainReduction = static_cast< float > (gainRedux);
    }

    if (compressorIsOn.load())
    {
        SampleType gainRedux;
        compressor.process (monoBuffer, &gainRedux);
        meterData.compressorGainReduction = static_cast< float > (gainRedux);
    }

    dryBuffer.clear();

    if (! leadIsBypassed) dryPanner.process (monoBuffer, dryBuffer);  // write to dry buffer & apply panning

    dryWetMixer.pushDrySamples (juce::dsp::AudioBlock< SampleType > (dryBuffer));

    wetBuffer.clear();

    if (harmoniesAreBypassed)
        harmonizer.bypassedBlock (blockSize, midiMessages);
    else
        harmonizer.render (monoBuffer, wetBuffer, midiMessages);  // renders the stereo output into wetBuffer

    dryWetMixer.mixWetSamples (juce::dsp::AudioBlock< SampleType > (wetBuffer));  // puts the mixed dry & wet samples into wetBuffer

    if (delayIsOn.load())
    {
        SampleType level;
        delay.process (wetBuffer, &level);
        meterData.delayLevel = static_cast< float > (level);
    }

    if (reverbIsOn.load())
    {
        SampleType level;
        reverb.process (wetBuffer, &level);
        meterData.reverbLevel = static_cast< float > (level);
    }

    outputGain.process (wetBuffer);

    if (limiterIsOn.load())
    {
        SampleType gainRedux;
        limiter.process (wetBuffer, &gainRedux);
        meterData.limiterGainReduction = static_cast< float > (gainRedux);
    }

    meterData.outputLevelL = static_cast< float > (wetBuffer.getMagnitude (0, 0, blockSize));
    meterData.outputLevelR = static_cast< float > (wetBuffer.getMagnitude (1, 0, blockSize));

    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, wetBuffer, chan, 0, blockSize);

    udpateInternalsData();
}

template < typename SampleType >
void Engine< SampleType >::udpateInternalsData()
{
    internalsData.mtsEspConnected = harmonizer.isConnectedToMtsEsp();
    internalsData.mtsEspScaleName = harmonizer.getScaleName();

    const auto intonationInfo       = harmonizer.getLatestIntonationInfo();
    internalsData.currentCentsSharp = intonationInfo.centsSharp;
    internalsData.currentPitch      = intonationInfo.pitch;

    const auto controllerData              = harmonizer.getLastMovedControllerInfo();
    internalsData.lastMovedMidiController  = controllerData.controllerNumber;
    internalsData.lastMovedControllerValue = controllerData.controllerValue;
}

template class Engine< float >;
template class Engine< double >;


}  // namespace Imogen
