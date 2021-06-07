
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

    dryWetMixer.setMixingRule (juce::dsp::DryWetMixingRule::balanced);
    dryWetMixer.setWetLatency (SampleType (0.0));

    // harmonizer.initialize (16, samplerate, newInternalBlocksize);
}

template < typename SampleType >
void Engine< SampleType >::renderChunk (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages, bool)
{
    updateAllParameters();

    const auto blockSize = input.getNumSamples();
    
    if (blockSize == 0)
        return;

    const bool leadIsBypassed       = parameters.leadBypass->get();
    const bool harmoniesAreBypassed = parameters.harmonyBypass->get();

    output.clear();

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

    if (parameters.noiseGateToggle->get())
    {
        SampleType gainRedux;
        gate.process (monoBuffer, &gainRedux);
        meters.gateRedux->set (static_cast< float > (gainRedux));
    }

    if (parameters.deEsserToggle->get())
    {
        SampleType gainRedux;
        deEsser.process (monoBuffer, &gainRedux);
        meters.deEssRedux->set (static_cast< float > (gainRedux));
    }

    if (parameters.compToggle->get())
    {
        SampleType gainRedux;
        compressor.process (monoBuffer, &gainRedux);
        meters.compRedux->set (static_cast< float > (gainRedux));
    }

    dryBuffer.clear();

    if (! leadIsBypassed)
        dryPanner.process (monoBuffer, dryBuffer);

    dryWetMixer.pushDrySamples (juce::dsp::AudioBlock< SampleType > (dryBuffer));

    wetBuffer.clear();

    if (harmoniesAreBypassed)
        harmonizer.bypassedBlock (blockSize, midiMessages);
    else
        harmonizer.render (monoBuffer, wetBuffer, midiMessages);  // renders the stereo output into wetBuffer

    dryWetMixer.mixWetSamples (juce::dsp::AudioBlock< SampleType > (wetBuffer));  // puts the mixed dry & wet samples into wetBuffer

    if (parameters.delayToggle->get())
    {
        SampleType level;
        delay.process (wetBuffer, &level);
        meters.delayLevel->set (static_cast< float > (level));
    }

    if (parameters.reverbToggle->get())
    {
        SampleType level;
        reverb.process (wetBuffer, &level);
        meters.reverbLevel->set (static_cast< float > (level));
    }

    outputGain.process (wetBuffer);

    if (parameters.limiterToggle->get())
    {
        SampleType gainRedux;
        limiter.process (wetBuffer, &gainRedux);
        meters.limRedux->set (static_cast< float > (gainRedux));
    }

    meters.outputLevelL->set (static_cast< float > (wetBuffer.getRMSLevel (0, 0, blockSize)));
    meters.outputLevelR->set (static_cast< float > (wetBuffer.getRMSLevel (1, 0, blockSize)));

    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, wetBuffer, chan, 0, blockSize);

    auto ccInfo = harmonizer.getLastMovedControllerInfo();
    internals.lastMovedMidiController->set (ccInfo.controllerNumber);
    internals.lastMovedCCValue->set (ccInfo.controllerValue);
    internals.mtsEspIsConnected->set (harmonizer.isConnectedToMtsEsp());
    // to do: intonation info...
}

template < typename SampleType >
void Engine< SampleType >::updateAllParameters()
{
    harmonizer.setMidiLatch (parameters.midiLatch->get());

    harmonizer.updateADSRsettings (parameters.adsrAttack->get(),
                                   parameters.adsrDecay->get(),
                                   parameters.adsrSustain->get(),
                                   parameters.adsrRelease->get());

    harmonizer.setNoteStealingEnabled (parameters.voiceStealing->get());
    harmonizer.setAftertouchGainOnOff (parameters.aftertouchToggle->get());
    harmonizer.updateLowestPannedNote (parameters.lowestPanned->get());
    harmonizer.updateMidiVelocitySensitivity (parameters.velocitySens->get());
    harmonizer.setPedalPitch (parameters.pedalToggle->get());
    harmonizer.setPedalPitchUpperThresh (parameters.pedalThresh->get());
    harmonizer.setPedalPitchInterval (parameters.pedalInterval->get());
    harmonizer.setDescant (parameters.descantToggle->get());
    harmonizer.setDescantLowerThresh (parameters.descantThresh->get());
    harmonizer.setDescantInterval (parameters.descantInterval->get());

    //   stereoReducer.setStereoReductionMode (parameters.inputMode->get());
    inputGain.setGain (parameters.inputGain->get());
    gate.setThreshold (static_cast< SampleType > (parameters.noiseGateThresh->get()));
    deEsser.setThresh (parameters.deEsserThresh->get());
    deEsser.setDeEssAmount (float (parameters.deEsserAmount->get()) * 0.01f);
    dryPanner.setMidiPan (parameters.leadPan->get());
    dryWetMixer.setWetMixProportion (float (parameters.dryWet->get()) * 0.01f);
    delay.setDryWet (parameters.delayDryWet->get());
    outputGain.setGain (parameters.outputGain->get());

    reverb.setDryWet (parameters.reverbDryWet->get());
    reverb.setDuckAmount (float (parameters.reverbDuck->get()) * 0.01f);
    reverb.setLoCutFrequency (parameters.reverbLoCut->get());
    reverb.setHiCutFrequency (parameters.reverbHiCut->get());
    updateReverbDecay (parameters.reverbDecay->get());

    updateCompressorAmount (parameters.compAmount->get());
    updateStereoWidth (parameters.stereoWidth->get());

    const auto st = parameters.pitchbendRange->get();
    harmonizer.updatePitchbendSettings (st, st);
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
    const auto a = float (amount) * 0.01f;
    compressor.setThreshold (juce::jmap (a, 0.0f, -60.0f));
    compressor.setRatio (juce::jmap (a, 1.0f, 10.0f));
}

template < typename SampleType >
void Engine< SampleType >::updateReverbDecay (int decay)
{
    const auto d = float (decay) * 0.01f;
    reverb.setDamping (1.0f - d);
    reverb.setRoomSize (d);
}

template < typename SampleType >
void Engine< SampleType >::onPrepare (int blocksize, double samplerate)
{
    if (! harmonizer.isInitialized())
        harmonizer.initialize (16, samplerate, blocksize);
    
    harmonizer.setCurrentPlaybackSampleRate (samplerate);
    harmonizer.prepare (blocksize);
    
    dsp::LatencyEngine<SampleType>::changeLatency (harmonizer.getLatencySamples());

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

    dspSpec.maximumBlockSize = static_cast< juce::uint32 > (blocksize);
    dspSpec.sampleRate       = samplerate;
    dspSpec.numChannels      = 2;

    dryWetMixer.prepare (dspSpec);
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
