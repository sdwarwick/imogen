
namespace Imogen
{

State::State() : PluginState (parameters, "Imogen")
{
    addDataChild (parameters, internals, meters);
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
    addInternal (abletonLinkEnabled, abletonLinkSessionPeers, mtsEspIsConnected, lastMovedMidiController, lastMovedCCValue, guiDarkMode, currentInputNote, currentCentsSharp, mtsEspScaleName);
}


EQState::EQState (ParameterList& list)
{
    list.add (eqToggle, eqLowShelfFreq, eqLowShelfQ, eqLowShelfGain, eqHighShelfFreq, eqHighShelfQ, eqHighShelfGain, eqHighPassFreq, eqHighPassQ, eqPeakFreq, eqPeakQ, eqPeakGain);
}


ReverbState::ReverbState (ParameterList& list)
{
    list.add (reverbToggle, reverbDryWet, reverbDecay, reverbDuck, reverbLoCut, reverbHiCut);
}


MidiState::MidiState (ParameterList& list)
{
    list.add (pitchbendRange, velocitySens, aftertouchToggle, voiceStealing, midiLatch, pitchGlide, glideTime, adsrAttack, adsrDecay, adsrSustain, adsrRelease, pedalToggle, pedalThresh, descantToggle, descantThresh, descantInterval);
    
    list.setPitchbendParameter (editorPitchbend);
}


std::string PresetManager::getCompanyName() { return "Ben Vining Music Software"; }
std::string PresetManager::getProductName() { return "Imogen"; }
std::string PresetManager::getPresetFileExtension() { return ".xml"; }

}
