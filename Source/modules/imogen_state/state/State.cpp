
namespace Imogen
{
void CustomStateData::serialize (TreeReflector&)
{
}

State::State() : plugin::CustomState<Parameters, CustomStateData> ("Imogen")
{
	internals.addToList (getParameters());
	meters.addToList (getParameters());
}

Parameters::Parameters()
	: ParameterList ("ImogenParameters")
{
	add (inputMode, dryWet, inputGain, outputGain, leadBypass, harmonyBypass, stereoWidth, lowestPanned, leadPan, noiseGateToggle, noiseGateThresh, deEsserToggle, deEsserThresh, deEsserAmount, compToggle, compAmount, delayToggle, delayDryWet, limiterToggle);
}


void Meters::addToList (plugin::ParameterList& list)
{
	list.add (inputLevel, outputLevelL, outputLevelR, gateRedux, compRedux, deEssRedux, limRedux, reverbLevel, delayLevel);
}

void Internals::addToList (plugin::ParameterList& list)
{
	list.addInternal (abletonLinkEnabled, abletonLinkSessionPeers, mtsEspIsConnected, lastMovedMidiController, lastMovedCCValue, guiDarkMode, currentInputNote, currentCentsSharp);
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
