/*
  ==============================================================================

    EpochExtractor.h
    Created: 17 Nov 2020 3:37:46am
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

class EpochExtractor {
	
public:
	
	void findEpochs(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, const double samplerate, const float inputFreq, Array<int>* epochLocations)
	{
		// determine pitch epoch locations in time, referenced by sample # within input buffer, and write these sample number locations to an integer array
		
		epochLocations->clearQuick();
		
	};
	
private:
	
};
