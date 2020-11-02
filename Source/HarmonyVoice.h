/*
  ==============================================================================

    HarmonyVoice.h
    Created: 2 Nov 2020 7:35:03am
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "PluginProcessor.h"


class HarmonyVoice {
	
public:
	
	bool voiceIsOn {
		
	};
	
	
	void startNote (int midiPitch, int velocity, int midiPan, int currentPitchWheelPosition) {
		desiredFrequency = MidiMessage::getMidiNoteInHertz(midiPitch);
		amplitudeMultiplier = float(velocity / 127);
		panning = midiPan;
		
		// still need to deal with pitch wheel
		
		// for glide -- maybe input "prev pitch" as argument to this function?
	}
	
	
	void stopNote () {
		
	}
	
	
	void pitchWheelMoved (int newPitchWheelValue) {
		
	}
	
	
	int reportNote () {  // can be called to poll any active voice -- "what pitch are you playing?"
		return int(desiredFrequency);
	}
	
	
	void updateDSPsettings(double newSampleRate, int newBlockSize) {
		
		// run this function to update internal DSP settings, etc
		
	}
	
	
	void renderNextBlock (AudioBuffer <float> &outputBuffer, int startSample, int numSamples) {
		
	}
	
	
private:
	double desiredFrequency;
	float amplitudeMultiplier;
	int panning;
	
};
