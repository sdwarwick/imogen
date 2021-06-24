
#include "ImogenEngine.h"

#include "Harmonizer/Harmonizer.cpp"
#include "Harmonizer/HarmonizerVoice/HarmonizerVoice.cpp"


namespace Imogen
{
template < typename SampleType >
Engine< SampleType >::Engine (State& stateToUse)
    : state (stateToUse)
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
        
        using FT = dsp::FX::FilterType;
        
        EQ.addBand (FT::LowPass, 10000.f);
        EQ.addBand (FT::HighPass, 80.f);
        EQ.addBand (FT::Peak, 2500.f);
}

template < typename SampleType >
void Engine< SampleType >::renderChunk (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages, bool)
{
    output.clear();

    const auto blockSize = input.getNumSamples();

    const bool leadIsBypassed       = parameters.leadBypass->get();
    const bool harmoniesAreBypassed = parameters.harmonyBypass->get();

    if (leadIsBypassed && harmoniesAreBypassed)
    {
        harmonizer.bypassedBlock (blockSize, midiMessages);
        return;
    }

    updateStereoReductionMode (parameters.inputMode->get());
    stereoReducer.convertStereoToMono (input, monoBuffer);
    
    initialLoCut.process (monoBuffer);

    inputGain.setGain (parameters.inputGain->get());
    inputGain.process (monoBuffer);
    
    meters.inputLevel->set (static_cast< float > (monoBuffer.getRMSLevel (0, 0, blockSize)));

    processNoiseGate (monoBuffer);
    processDeEsser (monoBuffer);
    processCompressor (monoBuffer);
    
    updateStereoWidth (parameters.stereoWidth->get());

    dryBuffer.clear();

    if (! leadIsBypassed)
    {
        dryPanner.setMidiPan (parameters.leadPan->get());
        dryPanner.process (monoBuffer, dryBuffer);
    }

    dryWetMixer.setWetMix (parameters.dryWet->get());
    dryWetMixer.pushDrySamples (dryBuffer);

    wetBuffer.clear();

    if (harmoniesAreBypassed)
        harmonizer.bypassedBlock (blockSize, midiMessages);
    else
    {
        updateHarmonizerParameters();
        harmonizer.render (monoBuffer, wetBuffer, midiMessages);
    }

    dryWetMixer.mixWetSamples (wetBuffer);

    processDelay (wetBuffer);
    processReverb (wetBuffer);
    processEQ (wetBuffer);

    outputGain.setGain (parameters.outputGain->get());
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
        gate.setThreshold (parameters.noiseGateThresh->get());
        
        gate.process (audio);
        meters.gateRedux->set (static_cast< float > (gate.getAverageGainReduction()));
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
        deEsser.setThresh (parameters.deEsserThresh->get());
        deEsser.setDeEssAmount (parameters.deEsserAmount->get());
        
        deEsser.process (audio);
        meters.deEssRedux->set (static_cast< float > (deEsser.getAverageGainReduction()));
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
        updateCompressorAmount (parameters.compAmount->get());
        
        compressor.process (audio);
        meters.compRedux->set (static_cast< float > (compressor.getAverageGainReduction()));
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
        delay.setDryWet (parameters.delayDryWet->get());
        
        delay.process (audio);
        meters.delayLevel->set (static_cast< float > (delay.getAverageGainReduction()));
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
        reverb.setDryWet (parameters.reverbDryWet->get());
        reverb.setDuckAmount (parameters.reverbDuck->get());
        reverb.setLoCutFrequency (parameters.reverbLoCut->get());
        reverb.setHiCutFrequency (parameters.reverbHiCut->get());
        updateReverbDecay (parameters.reverbDecay->get());
        
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
void Engine< SampleType >::processEQ (AudioBuffer& audio)
{
    using FT = dsp::FX::FilterType;
    
    if (parameters.eqToggle->get())
    {
        if (auto* lowPass = EQ.getBandOfType (FT::LowPass))
        {
            lowPass->setFilterFrequency (parameters.eqLowPassFreq->get());
            lowPass->setQfactor (parameters.eqLowPassQ->get());
        }
        
        if (auto* highPass = EQ.getBandOfType (FT::HighPass))
        {
            highPass->setFilterFrequency (parameters.eqHiPassFreq->get());
            highPass->setQfactor (parameters.eqHiPassQ->get());
        }
        
        if (auto* peak = EQ.getBandOfType (FT::Peak))
        {
            peak->setFilterFrequency (parameters.eqPeakFreq->get());
            peak->setQfactor (parameters.eqPeakQ->get());
            peak->setGain (parameters.eqPeakGain->get());
        }
        
        EQ.process (audio);
    }
}

template < typename SampleType >
void Engine< SampleType >::processLimiter (AudioBuffer& audio)
{
    if (parameters.limiterToggle->get())
    {
        limiter.process (audio);
        meters.limRedux->set (static_cast< float > (limiter.getAverageGainReduction()));
    }
    else
    {
        meters.limRedux->set (0.f);
    }
}

template < typename SampleType >
void Engine< SampleType >::updateHarmonizerParameters()
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

    harmonizer.updateLowestPannedNote (parameters.lowestPanned->get());
    
    harmonizer.togglePitchGlide (parameters.pitchGlide->get());
    harmonizer.setPitchGlideTime ((double) parameters.glideTime->get());
}

template < typename SampleType >
void Engine< SampleType >::updateInternals()
{
    auto ccInfo = harmonizer.getLastMovedControllerInfo();
    internals.lastMovedMidiController->set (ccInfo.controllerNumber);
    internals.lastMovedCCValue->set (ccInfo.controllerValue);
    internals.mtsEspIsConnected->set (harmonizer.isConnectedToMtsEsp());
    internals.mtsEspScaleName->set (harmonizer.getScaleName());
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

    if (const auto latency = harmonizer.getLatencySamples() > 0)
        dsp::LatencyEngine< SampleType >::changeLatency (latency);

    monoBuffer.setSize (1, blocksize, true, true, true);
    wetBuffer.setSize (1, blocksize, true, true, true);
    dryBuffer.setSize (1, blocksize, true, true, true);
    
    initialLoCut.prepare (samplerate, blocksize);

    stereoReducer.prepare (blocksize);
    inputGain.prepare (samplerate, blocksize);
    outputGain.prepare (samplerate, blocksize);
    gate.prepare (samplerate, blocksize);
    deEsser.prepare (samplerate, blocksize);
    compressor.prepare (samplerate, blocksize);
    dryPanner.prepare (samplerate, blocksize);
    delay.prepare (samplerate, blocksize);
    reverb.prepare (blocksize, samplerate, 2);
    EQ.prepare (samplerate, blocksize);
    limiter.prepare (samplerate, blocksize);
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
