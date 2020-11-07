/*
  ==============================================================================

    HarmonyVoice.h
    Created: 2 Nov 2020 7:35:03am
    Author:  Ben Vining

 
 	This class defines one instance of the harmony algorithm.
 
  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "shifter.h"


class HarmonyVoice {
	
	// STiLL NEED TO DEAL WITH:
	// pitch wheel / pitch bend
	// midiPan
	
public:
		
	bool voiceIsOn;
	
	HarmonyVoice(int thisVoiceNumber) {  
		voiceIsOn = false;
		this->thisVoiceNumber = thisVoiceNumber;
	};
	
	
	
	void startNote (int midiPitch, int velocity, int midiPan)
	{
		this->midiPan = midiPan;
		voiceIsOn = true;
		desiredFrequency = MidiMessage::getMidiNoteInHertz(midiPitch);
		amplitudeMultiplier = calcVelocityMultiplier(velocity);
		adsrEnv.noteOn();
	};
	
	
	void stopNote () {
		adsrEnv.noteOff();
	};
	
	
	float calcVelocityMultiplier(int midiVelocity) {
		float initialMutiplier = float(midiVelocity / 127); // what the multiplier would be without any sensitivity calculations...
		return ((1 - initialMutiplier) * (1 - midiVelocitySensitivity) + initialMutiplier);
	};
	
	
	void updateDSPsettings(double newSampleRate, int newBlockSize) {
		adsrEnv.setSampleRate(newSampleRate);
		pitchShifter.updateDSPsettings(newSampleRate, newBlockSize);  // passes settings thru to shifter instance 
	};
	
	
	void adsrSettingsListener(float* adsrAttackTime, float* adsrDecayTime, float* adsrSustainRatio, float* adsrReleaseTime, float* midiVelocitySensListener) {
		// attack/decay/release time in SECONDS; sustain ratio 0.0 - 1.0
		adsrParams.attack = *adsrAttackTime;
		adsrParams.decay = *adsrDecayTime;
		adsrParams.sustain = *adsrSustainRatio;
		adsrParams.release = *adsrReleaseTime;
		adsrEnv.setParameters(adsrParams);
		
		midiVelocitySensitivity = (float)(*midiVelocitySensListener / 100);
	};
	
	
	void renderNextBlock (AudioBuffer <float> &outputBuffer, int startSample, int numSamples, double modInputFreq) {
		
		float pitchShiftFactor = 1 + (modInputFreq - desiredFrequency) / desiredFrequency;  // maybe update this at sample rate too, instead of once per vector. depends how fast the input pitch detection updates...
		
		// need to pass dry input signal directly into shifter, to be used in pitchShifter.output
		
		// iterate through samples and write shifted samples to output buffer
		for(int sample = 0; sample < numSamples; ++sample) {
			
			if(adsrEnv.isActive() == false) {  // done while looping thru each sample...
				voiceIsOn = false;			// ... so that the voice itself doesn't turn off unti the ADSR actually *REACHES* zero
			} else {
				
				// shifted signal			 =   pitch shifter output										* mult. for MIDI velocity *  ADSR envelope
				double envelopedShiftedSignal = pitchShifter.output(pitchShiftFactor, startSample, numSamples) * amplitudeMultiplier * adsrEnv.getNextSample();
				
				
			
				for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
					outputBuffer.addSample(channel, startSample, envelopedShiftedSignal);
					// ADD TO THIS STEP: multiplying each channel's signal by the multiplier for that channel to create panning !!
				}
			}
		}
		
		// send all 12 harm. voice's output to one stereo buffer, so that they can be mixed as one "wet" signal...
		// need to make sure that I am ADDING to the output buffer signal, and not OVERWRITING it, otherwise you'll only hear the most recently played voice...
		
	};
	
	ADSR adsrEnv;
	ADSR::Parameters adsrParams;
	
private:
	int thisVoiceNumber;
	
	int midiPan;
	float midiVelocitySensitivity;  
	
	double desiredFrequency;
	float amplitudeMultiplier;
	int panning;
	
	Shifter pitchShifter;
};




// create a function for "panning changed" to reassign multiplier vals for each channel
