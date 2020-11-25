/*
  ==============================================================================

    HarmonyVoice.h
    Created: 2 Nov 2020 7:35:03am
    Author:  Ben Vining

 
 	This class defines one instance of the harmony algorithm.
 
  ==============================================================================
*/

#pragma once

#include "shifter.h"
#include "GlobalDefinitions.h"


class HarmonyVoice {
	
	
public:
		
	bool voiceIsOn;
	
	HarmonyVoice(const int voiceNum): voiceIsOn(false), midiPan(64), pitchBendRangeUp(2), pitchBendRangeDown(2), thisVoiceNumber(voiceNum), prevPan(64), panningMultR(0.5), panningMultL(0.5), midiVelocitySensitivity(1.0f), desiredFrequency(440.0f), lastNoteRecieved(69), amplitudeMultiplier(0.0f)
	{
		shiftedBuffer.setSize(1, MAX_BUFFERSIZE);
		harmonyBuffer.setSize(2, MAX_BUFFERSIZE);
		panningMultipliers[0] = 0.5f;
		panningMultipliers[1] = 0.5f;
		
		adsrParams.attack = 30;
		adsrParams.decay = 75;
		adsrParams.sustain = 0.8;
		adsrParams.release = 15;
		adsrEnv.setParameters(adsrParams);
	};
	
	
	void startNote (const int midiPitch, const int velocity, const int midiPan, const int lastPitchBend)
	{
		lastNoteRecieved = midiPitch;
		desiredFrequency = mtof(returnMidiFloat(lastPitchBend));
		amplitudeMultiplier = calcVelocityMultiplier(velocity);
		voiceIsOn = true;
		adsrEnv.noteOn();
	};
	
	
	void stopNote () {
		adsrEnv.noteOff();
		// voiceIsOn is set to FALSE in the renderNextBlock function so that the bool changes only after the ADSR has actually reached 0.
	};
	
	
	void changeNote (const int midiPitch, const int velocity, const int midiPan, const int lastPitchBend) {  // run this function to change the assigned midi pitch without retriggering the ADSR
		lastNoteRecieved = midiPitch;
		desiredFrequency = mtof(returnMidiFloat(lastPitchBend));
		amplitudeMultiplier = calcVelocityMultiplier(velocity);
		voiceIsOn = true;
		if(adsrEnv.isActive() == false) {
			adsrEnv.noteOn();
		}
	};
	
	
	void updateDSPsettings(const double newSampleRate, const int newBlockSize) {
		adsrEnv.setSampleRate(newSampleRate);
		checkBufferSizes(newBlockSize);
	};
	
	
	void adsrSettingsListener(float* adsrAttackTime, float* adsrDecayTime, float* adsrSustainRatio, float* adsrReleaseTime) {
		// attack/decay/release time in SECONDS; sustain ratio 0.0 - 1.0
		adsrParams.attack = *adsrAttackTime;
		adsrParams.decay = *adsrDecayTime;
		adsrParams.sustain = *adsrSustainRatio;
		adsrParams.release = *adsrReleaseTime;
		adsrEnv.setParameters(adsrParams);
	};
	
	
	void midiVelocitySensitivityListener(float* midiVelocitySensListener)
	{
		midiVelocitySensitivity = *midiVelocitySensListener / 100.0f;
	};
	
	
	void pitchBendSettingsListener(float* rangeUp, float* rangeDown) {
		pitchBendRangeUp = *rangeUp;
		pitchBendRangeDown = *rangeDown;
	};
	
	
	void clearBuffers() {
		shiftedBuffer.clear();
		harmonyBuffer.clear();
	};
	
	
	void renderNextBlock (AudioBuffer <float>& inputBuffer, const int numSamples, const int inputChannel, const float modInputFreq, Array<int> epochLocations, const int numOfEpochsPerFrame, const bool adsrIsOn) {
		// this function needs to write shifted samples to the stereo harmonyBuffer
		
		if(adsrEnv.isActive() == false) {
			voiceIsOn = false;
			amplitudeMultiplier = 0.0f;
			clearBuffers();
		} else {
		
			checkBufferSizes(numSamples);
			
			// this function puts resynthesized shifted samples into the mono shiftedBuffer
			pitchShifter.esola(inputBuffer, inputChannel, numSamples, epochLocations, modInputFreq, desiredFrequency, shiftedBuffer, numOfEpochsPerFrame);
			
			shiftedBuffer.applyGain(0, 0, numSamples, amplitudeMultiplier); // apply MIDI velocity multiplier
			
			// transfer samples into the stereo harmonyBuffer, which is where the processBlock will grab them from
			const float* shiftedReader = shiftedBuffer.getReadPointer(0);
			for(int channel = 0; channel < 2; ++channel)
			{
				float adsrMultiplier = 1.0f;
				float* writingTo = harmonyBuffer.getWritePointer(channel);
				for(int sample = 0; sample < numSamples; ++sample)
				{
					if(adsrIsOn) {
						adsrMultiplier = adsrEnv.getNextSample();
					} else {
						adsrMultiplier = 1.0f;
					}
					writingTo[sample] = shiftedReader[sample] * adsrMultiplier * panningMultipliers[channel];
				}
			}
		}
		
	};
	
	
	void changePanning(const int newPanVal) {   // this function updates the voice's panning if it is active when the stereo width setting is changed
												// TODO: ramp this value???
		if(midiPan != newPanVal)
		{
			prevPan = midiPan;
			midiPan = newPanVal;
			calculatePanningChannelMultipliers(newPanVal);
		}
	};
	
	
	void pitchBend(const int pitchBend) {
		if (pitchBend > 64) {
			desiredFrequency = mtof(((pitchBendRangeUp * (pitchBend - 65)) / 62) + lastNoteRecieved);
		} else if (pitchBend < 64) {
			desiredFrequency = mtof((((1 - pitchBendRangeDown) * pitchBend) / 63) + lastNoteRecieved - pitchBendRangeDown);
		} else if (pitchBend == 64) {
			desiredFrequency = mtof(lastNoteRecieved);
		}
	};
	
	AudioBuffer<float> shiftedBuffer; // this audio buffer will store the shifted signal. MONO BUFFER [step 1] - only use channel 0
	AudioBuffer<float> harmonyBuffer; // this buffer stores the harmony voice's actual output. STEREO BUFFER [step 2]
	
	
	ADSR adsrEnv;
	ADSR::Parameters adsrParams;
	int midiPan;
	
private:
	
	int pitchBendRangeUp;
	int pitchBendRangeDown;
	
	const int thisVoiceNumber;
	
	int prevPan;
	int panning;
	float panningMultR;
	float panningMultL;
	float panningMultipliers[2];
	
	float midiVelocitySensitivity;  
	
	float desiredFrequency;
	int lastNoteRecieved;
	
	float amplitudeMultiplier;
	
	Shifter pitchShifter;
	
	void calculatePanningChannelMultipliers(const int midipanning) {
		panningMultR = midipanning / 127.0;
		panningMultL = 1.0 - panningMultR;
		panningMultipliers[0] = panningMultL;
		panningMultipliers[1] = panningMultR;
	};
	
	
	float returnMidiFloat(const int bend) const {
		if (bend > 64) {
			return ((pitchBendRangeUp * (bend - 65)) / 62) + lastNoteRecieved;
		} else if (bend < 64) {
			return (((1 - pitchBendRangeDown) * bend) / 63) + lastNoteRecieved - pitchBendRangeDown;
		} else {
			return lastNoteRecieved;
		}
	};
	
	
	double mtof(const float midiNote) const {  // converts midiPitch to frequency in Hz
		return 440.0 * std::pow(2.0, ((midiNote - 69.0) / 12.0));
	};
	
	
	float calcVelocityMultiplier(const int midiVelocity) const {
		const float initialMutiplier = midiVelocity / 127.0; // what the multiplier would be without any sensitivity calculations...
		return ((1 - initialMutiplier) * (1 - midiVelocitySensitivity) + initialMutiplier);
	};
	
	
	void checkBufferSizes(const int newNumSamples) {
		if(shiftedBuffer.getNumSamples() != newNumSamples) {
			shiftedBuffer.setSize(1, newNumSamples, false, false, true);
		}
		if(harmonyBuffer.getNumSamples() != newNumSamples) {
			harmonyBuffer.setSize(2, newNumSamples, false, false, true);
		}
	};
	
};


