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
	
	Array<int> returnEpochs(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, const double samplerate)
	{
		// determine pitch epoch locations in time, referenced by sample # within input buffer, and write these sample number locations to an integer array
		
		Array<int> epochs;
		return epochs;
	};
	
private:
	
};
