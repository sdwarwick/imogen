
namespace Imogen
{
template < typename SampleType >
Harmonizer< SampleType >::Harmonizer (State& stateToUse)
    : state (stateToUse)
{
    Base::setConcertPitchHz (440);

    Base::updateQuickReleaseMs (5);
    Base::setPlayingButReleasedMultiplier (0.4f);
    Base::setSoftPedalMultiplier (0.65f);
}

template < typename SampleType >
void Harmonizer< SampleType >::prepared (double samplerate, int blocksize)
{
    analyzer.prepare (samplerate, blocksize);
    leadProcessor.prepare (samplerate, blocksize);

    wetBuffer.setSize (2, blocksize, true, true, true);
}

template < typename SampleType >
void Harmonizer< SampleType >::process (const AudioBuffer& input, MidiBuffer& midi,
                                        bool harmoniesBypassed, bool leadBypassed)
{
    analyzer.analyzeInput (input);

    leadProcessor.process (leadBypassed);

    if (harmoniesBypassed)
    {
        wetBuffer.clear();
        this->bypassedBlock (input.getNumSamples(), midi);
    }
    else
    {
        updateParameters();
        this->renderVoices (midi, wetBuffer);
    }

    updateInternals();
}

template < typename SampleType >
void Harmonizer< SampleType >::updateParameters()
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
}

template < typename SampleType >
void Harmonizer< SampleType >::updateInternals()
{
    auto ccInfo = this->getLastMovedControllerInfo();
    internals.lastMovedMidiController->set (ccInfo.controllerNumber);
    internals.lastMovedCCValue->set (ccInfo.controllerValue);
    internals.mtsEspIsConnected->set (this->isConnectedToMtsEsp());
    internals.mtsEspScaleName->set (this->getScaleName());
}

template < typename SampleType >
HarmonizerVoice< SampleType >* Harmonizer< SampleType >::createVoice()
{
    return new Voice (*this, analyzer);
}

template < typename SampleType >
int Harmonizer< SampleType >::getLatencySamples() const
{
    return analyzer.getLatencySamples();
}

template < typename SampleType >
juce::AudioBuffer< SampleType >& Harmonizer< SampleType >::getHarmonySignal()
{
    return wetBuffer;
}

template < typename SampleType >
juce::AudioBuffer< SampleType >& Harmonizer< SampleType >::getLeadSignal()
{
    return leadProcessor.getProcessedSignal();
}


template class Harmonizer< float >;
template class Harmonizer< double >;


}  // namespace Imogen
