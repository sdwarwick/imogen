/*
  ==============================================================================

    shifter.h
    Created: 2 Nov 2020 4:55:40pm
    Author:  Ben Vining
 
 	This class will do the actual pitch shifting computations.
 
 	It should be fed the analysis data of the incoming modulator signal (grain creation) -- so all this class needs to do is the actual *manipulation* & playback of the grains, then resynthesis into the output audio signal.

  ==============================================================================
*/

#pragma once


class Shifter {
	
public:
	
	
	void doTheShifting(AudioBuffer<float>& inputBuffer, const int inputChan, AudioBuffer<float>& shiftedBuffer, const int numSamples, const double inputFreq, const float pitchShiftFactor) {
		// this function should fill shiftedBuffer with pitch shifted samples from inputBuffer
		// shiftedBuffer is MONO !! only use channel 0
		
		const float* readingfrom = inputBuffer.getReadPointer(0);
		float* writingto = shiftedBuffer.getWritePointer(0);
		
	};
	
	
	void updateDSPsettings(const double newSampleRate, const int newBlockSize) {
		
	};
	
	
private:
	
};
