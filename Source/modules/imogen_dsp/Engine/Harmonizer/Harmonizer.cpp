
namespace Imogen
{
template < typename SampleType >
Harmonizer< SampleType >::Harmonizer (State& stateToUse, Analyzer& analyzerToUse)
    : analyzer (analyzerToUse), state (stateToUse)
{
    this->updateQuickReleaseMs (5);

    this->playingButReleased.gain = 0.4f;
    this->softPedal.gain          = 0.65f;
}

template < typename SampleType >
void Harmonizer< SampleType >::prepared (double, int blocksize)
{
    wetBuffer.setSize (2, blocksize, true, true, true);
}

template < typename SampleType >
void Harmonizer< SampleType >::process (int numSamples, MidiBuffer& midiMessages,
                                        bool harmoniesBypassed)
{
    if (harmoniesBypassed)
    {
        wetBuffer.clear();
        this->bypassedBlock (numSamples, midiMessages);
    }
    else
    {
        updateParameters();
        this->renderVoices (midiMessages, wetBuffer);
    }

    updateInternals();
    lastBlocksize = numSamples;
}

template < typename SampleType >
void Harmonizer< SampleType >::updateParameters()
{
    this->setMidiLatch (midi.midiLatch->get());

    this->updateADSRsettings (midi.adsrAttack->get(),
                              midi.adsrDecay->get(),
                              static_cast< float > (midi.adsrSustain->get()) * 0.01f,
                              midi.adsrRelease->get());

    this->pedal.setParams (midi.pedalToggle->get(),
                           midi.pedalThresh->get(),
                           midi.pedalInterval->get());

    this->descant.setParams (midi.descantToggle->get(),
                             midi.descantThresh->get(),
                             midi.descantInterval->get());

    this->setNoteStealingEnabled (midi.voiceStealing->get());
    this->setAftertouchGainOnOff (midi.aftertouchToggle->get());

    this->updateMidiVelocitySensitivity (midi.velocitySens->get());

    this->updatePitchbendRange (midi.pitchbendRange->get());

    this->panner.setLowestNote (parameters.lowestPanned->get());

    this->togglePitchGlide (midi.pitchGlide->get());
    this->setPitchGlideTime (static_cast< double > (midi.glideTime->get()));
}

template < typename SampleType >
void Harmonizer< SampleType >::updateInternals()
{
    auto ccInfo = this->getLastMovedControllerInfo();
    internals.lastMovedMidiController->set (ccInfo.controllerNumber);
    internals.lastMovedCCValue->set (ccInfo.controllerValue);
    internals.mtsEspIsConnected->set (this->isConnectedToMtsEsp());
    //    internals.mtsEspScaleName->set (this->getScaleName());
}

template < typename SampleType >
HarmonizerVoice< SampleType >* Harmonizer< SampleType >::createVoice()
{
    return new Voice (*this, analyzer);
}

template < typename SampleType >
juce::AudioBuffer< SampleType >& Harmonizer< SampleType >::getHarmonySignal()
{
    alias.setDataToReferTo (wetBuffer.getArrayOfWritePointers(), 2, lastBlocksize);
    return alias;
}


template class Harmonizer< float >;
template class Harmonizer< double >;


}  // namespace Imogen
