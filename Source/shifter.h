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

#include "../JuceLibraryCode/JuceHeader.h"


class Shifter {
	
public:
	double output(float pitchShiftRatio, int startingSample, int numSamples) {
		
		// dry input signal in from audio process block...
		
		// analysis of input signal in (?) -- grain lengths & peak locations for current signal vector
		
		// resynthesis -- play back grains at altered speed / density / repititions, using an OLA method
		
		// resample into an output signal -- which will be the stream of doubles output from shifter.output
		
		return double(0.0);
	};
	
	
	void updateDSPsettings(const double newSampleRate, const int newBlockSize) {
		
	};
	
	
private:
	
};
