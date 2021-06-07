
#include "ImogenEngine.h"

#include "Harmonizer/Harmonizer.cpp"
#include "Harmonizer/HarmonizerVoice/HarmonizerVoice.cpp"


namespace Imogen
{
template < typename SampleType >
Engine< SampleType >::Engine (Parameters& params, Meters& metersToUse, Internals& internalsToUse)
    : parameters (params), meters (metersToUse), internals (internalsToUse)
{
    //    static constexpr auto noiseGateAttackMs   = 25.0f;
    //    static constexpr auto noiseGateReleaseMs  = 100.0f;
    //    static constexpr auto noiseGateFloorRatio = 10.0f;  // ratio to one when the noise gate is activated
    //
    //    static constexpr auto compressorAttackMs  = 4.0f;
    //    static constexpr auto compressorReleaseMs = 200.0f;
    //
    //    static constexpr auto limiterThreshDb     = 0.0f;
    //    static constexpr auto limiterReleaseMs    = 35.0f;
}

template < typename SampleType >
void Engine< SampleType >::renderChunk (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages, bool)
{
    updateAllParameters();
    output.clear();

    const auto blockSize = input.getNumSamples();

    const bool leadIsBypassed       = parameters.leadBypass->get();
    const bool harmoniesAreBypassed = parameters.harmonyBypass->get();

    if (leadIsBypassed && harmoniesAreBypassed)
    {
        harmonizer.bypassedBlock (blockSize, midiMessages);
        return;
    }

    stereoReducer.convertStereoToMono (input, monoBuffer);

    meters.inputLevel->set (static_cast< float > (monoBuffer.getRMSLevel (0, 0, blockSize)));

    inputGain.process (monoBuffer);

    //    juce::dsp::AudioBlock<SampleType> monoBlock (monoBuffer);
    //    initialHiddenLoCut.process ( juce::dsp::ProcessContextReplacing<SampleType>(monoBlock) );

    processNoiseGate (monoBuffer);
    processDeEsser (monoBuffer);
    processCompressor (monoBuffer);

    dryBuffer.clear();

    if (! leadIsBypassed)
        dryPanner.process (monoBuffer, dryBuffer);
    
    dryWetMixer.pushDrySamples (dryBuffer);

    wetBuffer.clear();

    if (harmoniesAreBypassed)
        harmonizer.bypassedBlock (blockSize, midiMessages);
    else
        harmonizer.render (monoBuffer, wetBuffer, midiMessages);  // renders the stereo output into wetBuffer
    
    dryWetMixer.mixWetSamples (wetBuffer);

    processDelay (wetBuffer);
    processReverb (wetBuffer);

    outputGain.process (wetBuffer);

    processLimiter (wetBuffer);

    meters.outputLevelL->set (static_cast< float > (wetBuffer.getRMSLevel (0, 0, blockSize)));
    meters.outputLevelR->set (static_cast< float > (wetBuffer.getRMSLevel (1, 0, blockSize)));

    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, wetBuffer, chan, 0, blockSize);

    updateInternals();
}

template < typename SampleType >
void Engine< SampleType >::processNoiseGate (AudioBuffer& audio)
{
    if (parameters.noiseGateToggle->get())
    {
        SampleType gainRedux;
        gate.process (audio, &gainRedux);
        meters.gateRedux->set (static_cast< float > (gainRedux));
    }
    else
    {
        meters.gateRedux->set (0.f);
    }
}

template < typename SampleType >
void Engine< SampleType >::processDeEsser (AudioBuffer& audio)
{
    if (parameters.deEsserToggle->get())
    {
        SampleType gainRedux;
        deEsser.process (audio, &gainRedux);
        meters.deEssRedux->set (static_cast< float > (gainRedux));
    }
    else
    {
        meters.deEssRedux->set (0.f);
    }
}

template < typename SampleType >
void Engine< SampleType >::processCompressor (AudioBuffer& audio)
{
    if (parameters.compToggle->get())
    {
        SampleType gainRedux;
        compressor.process (audio, &gainRedux);
        meters.compRedux->set (static_cast< float > (gainRedux));
    }
    else
    {
        meters.compRedux->set (0.f);
    }
}

template < typename SampleType >
void Engine< SampleType >::processDelay (AudioBuffer& audio)
{
    if (parameters.delayToggle->get())
    {
        SampleType level;
        delay.process (audio, &level);
        meters.delayLevel->set (static_cast< float > (level));
    }
    else
    {
        meters.delayLevel->set (-60.f);
    }
}


template < typename SampleType >
void Engine< SampleType >::processReverb (AudioBuffer& audio)
{
    if (parameters.reverbToggle->get())
    {
        SampleType level;
        reverb.process (audio, &level);
        meters.reverbLevel->set (static_cast< float > (level));
    }
    else
    {
        meters.reverbLevel->set (-60.f);
    }
}

template < typename SampleType >
void Engine< SampleType >::processLimiter (AudioBuffer& audio)
{
    if (parameters.limiterToggle->get())
    {
        SampleType gainRedux;
        limiter.process (audio, &gainRedux);
        meters.limRedux->set (static_cast< float > (gainRedux));
    }
    else
    {
        meters.limRedux->set (0.f);
    }
}

template < typename SampleType >
void Engine< SampleType >::updateAllParameters()
{
    harmonizer.setMidiLatch (parameters.midiLatch->get());

    harmonizer.updateADSRsettings (parameters.adsrAttack->get(),
                                   parameters.adsrDecay->get(),
                                   parameters.adsrSustain->get(),
                                   parameters.adsrRelease->get());

    harmonizer.setPedalPitch (parameters.pedalToggle->get(),
                              parameters.pedalThresh->get(),
                              parameters.pedalInterval->get());

    harmonizer.setDescant (parameters.descantToggle->get(),
                           parameters.descantThresh->get(),
                           parameters.descantInterval->get());

    harmonizer.setNoteStealingEnabled (parameters.voiceStealing->get());
    harmonizer.setAftertouchGainOnOff (parameters.aftertouchToggle->get());
    harmonizer.updateMidiVelocitySensitivity (parameters.velocitySens->get());
    harmonizer.updatePitchbendRange (parameters.pitchbendRange->get());

    updateStereoWidth (parameters.stereoWidth->get());
    harmonizer.updateLowestPannedNote (parameters.lowestPanned->get());

    inputGain.setGain (parameters.inputGain->get());
    gate.setThreshold (parameters.noiseGateThresh->get());
    deEsser.setThresh (parameters.deEsserThresh->get());
    deEsser.setDeEssAmount (parameters.deEsserAmount->get());
    dryPanner.setMidiPan (parameters.leadPan->get());
    dryWetMixer.setWetMix (parameters.dryWet->get());
    delay.setDryWet (parameters.delayDryWet->get());
    outputGain.setGain (parameters.outputGain->get());

    reverb.setDryWet (parameters.reverbDryWet->get());
    reverb.setDuckAmount (parameters.reverbDuck->get());
    reverb.setLoCutFrequency (parameters.reverbLoCut->get());
    reverb.setHiCutFrequency (parameters.reverbHiCut->get());
    updateReverbDecay (parameters.reverbDecay->get());

    updateCompressorAmount (parameters.compAmount->get());
    updateStereoReductionMode (parameters.inputMode->get());
}

template < typename SampleType >
void Engine< SampleType >::updateInternals()
{
    auto ccInfo = harmonizer.getLastMovedControllerInfo();
    internals.lastMovedMidiController->set (ccInfo.controllerNumber);
    internals.lastMovedCCValue->set (ccInfo.controllerValue);
    internals.mtsEspIsConnected->set (harmonizer.isConnectedToMtsEsp());
    // to do: intonation info...
}

template < typename SampleType >
void Engine< SampleType >::updateStereoWidth (int width)
{
    harmonizer.updateStereoWidth (width);
    reverb.setWidth (static_cast< float > (width) * 0.01f);
}

template < typename SampleType >
void Engine< SampleType >::updateCompressorAmount (int amount)
{
    const auto a = static_cast< float > (amount) * 0.01f;
    compressor.setThreshold (juce::jmap (a, 0.f, -60.f));
    compressor.setRatio (juce::jmap (a, 1.f, 10.f));
}

template < typename SampleType >
void Engine< SampleType >::updateReverbDecay (int decay)
{
    const auto d = static_cast< float > (decay) * 0.01f;
    reverb.setDamping (1.f - d);
    reverb.setRoomSize (d);
}

template < typename SampleType >
void Engine< SampleType >::updateStereoReductionMode (int mode)
{
    using Mode = typename dsp::FX::MonoStereoConverter< SampleType >::StereoReductionMode;

    switch (mode)
    {
        case (1) : stereoReducer.setStereoReductionMode (Mode::rightOnly); return;
        case (2) : stereoReducer.setStereoReductionMode (Mode::mixToMono); return;
        default : stereoReducer.setStereoReductionMode (Mode::leftOnly); return;
    }
}

template < typename SampleType >
void Engine< SampleType >::onPrepare (int blocksize, double samplerate)
{
    if (! harmonizer.isInitialized())
        harmonizer.initialize (16, samplerate, blocksize);

    harmonizer.setCurrentPlaybackSampleRate (samplerate);
    harmonizer.prepare (blocksize);

    dsp::LatencyEngine< SampleType >::changeLatency (harmonizer.getLatencySamples());

    monoBuffer.setSize (1, blocksize, true, true, true);
    wetBuffer.setSize (1, blocksize, true, true, true);
    dryBuffer.setSize (1, blocksize, true, true, true);

    stereoReducer.prepare (blocksize);
    inputGain.prepare (blocksize);
    outputGain.prepare (blocksize);
    gate.prepare (1, blocksize, samplerate);
    deEsser.prepare (blocksize, samplerate);
    compressor.prepare (blocksize, samplerate, 1);
    dryPanner.prepare (blocksize);
    delay.prepare (blocksize, samplerate, 2);
    reverb.prepare (blocksize, samplerate, 2);
    limiter.prepare (blocksize, samplerate, 2);

    dryWetMixer.prepare (2, blocksize, samplerate);
}

template < typename SampleType >
void Engine< SampleType >::onRelease()
{
    harmonizer.releaseResources();

    monoBuffer.setSize (0, 0);
    wetBuffer.setSize (0, 0);
    dryBuffer.setSize (0, 0);

    inputGain.reset();
    outputGain.reset();
    gate.reset();
    deEsser.reset();
    compressor.reset();
    dryPanner.reset();
    dryWetMixer.reset();
    delay.reset();
    reverb.reset();
    limiter.reset();
}


template class Engine< float >;
template class Engine< double >;


}  // namespace Imogen
