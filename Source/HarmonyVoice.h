/*
  ==============================================================================

    HarmonyVoice.h
    Created: 2 Nov 2020 7:35:03am
    Author:  Ben Vining

 
 	This class hosts each of the 12 harmony algorithms.
 
  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "PluginProcessor.h"
#include "shifter.h"



class HarmonyVoice {
	
public:
		
	bool voiceIsOn;
	int thisVoiceNumber;
	
	void startNote (int midiPitch, int velocity, int midiPan, int currentPitchWheelPosition) {
		voiceIsOn = true;
		desiredFrequency = MidiMessage::getMidiNoteInHertz(midiPitch);
		amplitudeMultiplier = float(velocity / 127);
		panning = midiPan;
		adsrEnv.noteOn();
		
		// still need to deal with pitch wheel
		
		// for glide -- maybe input "prev pitch" as argument to this function?
	}
	
	
	void stopNote () {
		adsrEnv.noteOff();
		voiceIsOn = false;
		amplitudeMultiplier = 0;
	}
	
	
	void pitchWheelMoved (int newPitchWheelValue) {
		
	}
	
	
	void updateDSPsettings(double newSampleRate, int newBlockSize) {
		adsrEnv.setSampleRate(newSampleRate);
	}
	
	
	void adsrSettingsListener(float* adsrAttackTime, float* adsrDecayTime, float* adsrSustainRatio, float* adsrReleaseTime) {
		// attack/decay/release time in SECONDS; sustain ratio 0.0 - 1.0
		adsrParams.attack = *adsrAttackTime;
		adsrParams.decay = *adsrDecayTime;
		adsrParams.sustain = *adsrSustainRatio;
		adsrParams.release = *adsrReleaseTime;
		adsrEnv.setParameters(adsrParams);
	}
	
	
	void renderNextBlock (AudioBuffer <float> &outputBuffer, int startSample, int numSamples, double modInputFreq) {
		
		float pitchShiftFactor = float(desiredFrequency / modInputFreq);  // maybe update this at sample rate too, instead of once per vector. depends how fast the input pitch detection updates...
		
		// iterate through samples and write shifted samples to output buffer
		for(int sample = 0; sample < numSamples; ++sample) {
			// shifted signal			 =   pitch shifter output										* mult. for MIDI velocity *  ADSR envelope
			double envelopedShiftedSignal = pitchShifter.output(pitchShiftFactor, startSample, numSamples) * amplitudeMultiplier * adsrEnv.getNextSample();
		
			for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
				outputBuffer.addSample(channel, startSample, envelopedShiftedSignal);  // ADD TO THIS STEP: multiplying each channel's signal by the multiplier for that channel to create panning !!
			}
		}
		
		// send all 12 harm. voice's output to one stereo buffer, so that they can be mixed as one "wet" signal...
	}
	
	ADSR adsrEnv;
	
private:
	double desiredFrequency;
	float amplitudeMultiplier;
	int panning;
	
	Shifter pitchShifter;
	
	ADSR::Parameters adsrParams;
};




// create a function for "panning changed" to reassign multiplier vals for each channel
