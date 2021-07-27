
namespace Imogen
{
State::State() : plugin::State (parameters, "Imogen")
{
}

void State::addTo (juce::AudioProcessor& processor)
{
    parameters.addParametersTo (processor);
    meters.addParametersTo (processor);
    internals.addAllParametersAsInternal();
}

void State::addAllAsInternal()
{
    parameters.addAllParametersAsInternal();
    meters.addAllParametersAsInternal();
    internals.addAllParametersAsInternal();
}

void State::serialize (TreeReflector& ref)
{
    plugin::State::serialize (ref);

    ref.add ("Meters", meters);
    ref.add ("InternalSettings", internals);
}


Parameters::Parameters()
    : ParameterList ("ImogenParameters")
{
    add (inputMode, dryWet, inputGain, outputGain, leadBypass, harmonyBypass, stereoWidth, lowestPanned, leadPan, noiseGateToggle, noiseGateThresh, deEsserToggle, deEsserThresh, deEsserAmount, compToggle, compAmount, delayToggle, delayDryWet, limiterToggle);
}


Meters::Meters()
    : ParameterList ("ImogenMeters")
{
    add (inputLevel, outputLevelL, outputLevelR, gateRedux, compRedux, deEssRedux, limRedux, reverbLevel, delayLevel);
}


Internals::Internals()
    : ParameterList ("ImogenInternals")
{
    addInternal (abletonLinkEnabled, abletonLinkSessionPeers, mtsEspIsConnected, lastMovedMidiController, lastMovedCCValue, guiDarkMode, currentInputNote, currentCentsSharp);
    // mtsEspScaleName
}


EQState::EQState (plugin::ParameterList& list)
{
    list.add (eqToggle, eqLowShelfFreq, eqLowShelfQ, eqLowShelfGain, eqHighShelfFreq, eqHighShelfQ, eqHighShelfGain, eqHighPassFreq, eqHighPassQ, eqPeakFreq, eqPeakQ, eqPeakGain);
}


ReverbState::ReverbState (plugin::ParameterList& list)
{
    list.add (reverbToggle, reverbDryWet, reverbDecay, reverbDuck, reverbLoCut, reverbHiCut);
}


MidiState::MidiState (plugin::ParameterList& list)
{
    list.add (pitchbendRange, velocitySens, aftertouchToggle, voiceStealing, midiLatch, pitchGlide, glideTime, adsrAttack, adsrDecay, adsrSustain, adsrRelease, pedalToggle, pedalThresh, descantToggle, descantThresh, descantInterval);

    list.setPitchbendParameter (editorPitchbend);
}

}  // namespace Imogen
