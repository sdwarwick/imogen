/*
  ==============================================================================

    Duplicator.h
    Created: 22 Nov 2020 2:37:50pm
    Author:  Ben Vining
 
 	In order to allow Imogen to create the sound of an actual *choir*, it is desirable for each harmony part to sound like it is being sung by more than one voice. To enable computationnal effeciency, the actual ESOLA pitch shifting algorithm is only performed once for each desired output pitch, and these resulting shifted signals are duplicated and given various modulatable parameters to create the choir effect.
 
 the parameters are:
 		* number of copies: the number of times to duplicate the input shifted signal
 		* time spread 	: short delay on copies of the shifted signal
 		* panning spread: variation of placement of shifted signal's copies in stereo field of master output

  ==============================================================================
*/

#pragma once

#include "GlobalDefinitions.h"


class Duplicator {
	
public:
	
	void process(AudioBuffer<float>& inputBuffer, const int numSamples, AudioBuffer<float>& outputBuffer, const int numberOfDuplications, const int originalPanning)
	{
		
		AudioBuffer<float> choirSignal(2, numSamples);
		
		// first, write all duplications' signals to choirSignal buffer
		for(int i = 0; i < numberOfDuplications; ++i)
		{
			const int thisDelay = getDelayValue(i);
			const int thisPan = getPanningValue(i, originalPanning);
			
			float mults[2];
			mults[1] = thisPan/127.0f;
			mults[0] = 1.0f - mults[1];
			
			const float* r = inputBuffer.getReadPointer(0);
			for(int chan = 0; chan < 2; ++chan) {
				float* w = choirSignal.getWritePointer(chan);
				for(int s = 0; s < numSamples; ++s) {
					w[s] = r[s] * mults[chan]; // apply panning multipliers
				}
			}
		}
		
		for(int channel = 0; channel < 2; ++channel) {
			const float* reading = choirSignal.getReadPointer(channel);
			float* writing = outputBuffer.getWritePointer(channel);
			for(int n = 0; n < numSamples; ++n) {
				writing[n] = reading[n] / numberOfDuplications / 2; // divide choirSignal by # of duplications while adding to outputBuffer & dividing final signal by 2
			}
		}
	};
	
	
private:
	
	int getDelayValue(const int instance) {
		// returns an individual time delay value for a certain duplication, based on the global base delay & delay spread parameters
		return 0;
	};
	
	int getPanningValue(const int instance, const int originalPan) {
		// returns an individual midiPanning value for a certain duplication, based on the global panning spread parameter
		// for panning, it should be a spread L or R of the original shifted voice's position in the stereo field, so this function should also be fed the original HarmonyVoice's panning value as a parameter
		return 64;
	};
	
};
