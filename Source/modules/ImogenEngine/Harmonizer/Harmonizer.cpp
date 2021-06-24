
namespace Imogen
{
template < typename SampleType >
Harmonizer< SampleType >::Harmonizer(State& stateToUse): state(stateToUse)
{
    Base::setConcertPitchHz (440);

    Base::updateQuickReleaseMs (adsrQuickReleaseMs);

    Base::setPlayingButReleasedMultiplier (playingButReleasedGainMultiplier);
    Base::setSoftPedalMultiplier (softPedalGainMultiplier);
}


template < typename SampleType >
void Harmonizer< SampleType >::initialized (const double, const int)
{
    //analyzer.initialize();
}


template < typename SampleType >
void Harmonizer< SampleType >::prepared (int)
{
}


template < typename SampleType >
void Harmonizer< SampleType >::resetTriggered()
{
   // analyzer.reset();
}


template < typename SampleType >
void Harmonizer< SampleType >::samplerateChanged (double /*newSamplerate*/)
{
   // analyzer.setSamplerate (newSamplerate);
}


template < typename SampleType >
void Harmonizer< SampleType >::release()
{
   // analyzer.releaseResources();
}

template < typename SampleType >
void Harmonizer< SampleType >::process (AudioBuffer& /*input*/, AudioBuffer& output, MidiBuffer& midi, bool bypassed)
{
    if (bypassed)
    {
        output.clear();
        this->bypassedBlock (output.getNumSamples(), midi);
    }
    else
    {
        this->setMidiLatch (parameters.midiLatch->get());
        
        this->updateADSRsettings (parameters.adsrAttack->get(),
                                       parameters.adsrDecay->get(),
                                       parameters.adsrSustain->get(),
                                       parameters.adsrRelease->get());
        
        this->setPedalPitch (parameters.pedalToggle->get(),
                                  parameters.pedalThresh->get(),
                                  parameters.pedalInterval->get());
        
        this->setDescant (parameters.descantToggle->get(),
                               parameters.descantThresh->get(),
                               parameters.descantInterval->get());
        
        this->setNoteStealingEnabled (parameters.voiceStealing->get());
        this->setAftertouchGainOnOff (parameters.aftertouchToggle->get());
        
        this->updateMidiVelocitySensitivity (parameters.velocitySens->get());
        
        this->updatePitchbendRange (parameters.pitchbendRange->get());
        
        this->updateLowestPannedNote (parameters.lowestPanned->get());
        
        this->togglePitchGlide (parameters.pitchGlide->get());
        this->setPitchGlideTime ((double) parameters.glideTime->get());
        
        this->renderVoices (midi, output);
    }
    
    auto ccInfo = this->getLastMovedControllerInfo();
    internals.lastMovedMidiController->set (ccInfo.controllerNumber);
    internals.lastMovedCCValue->set (ccInfo.controllerValue);
    internals.mtsEspIsConnected->set (this->isConnectedToMtsEsp());
    internals.mtsEspScaleName->set (this->getScaleName());
}


template class Harmonizer< float >;
template class Harmonizer< double >;


}  // namespace bav
