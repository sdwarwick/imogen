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
		
		epochLocations->clear();
		
	//	const float* input = inputBuffer.getReadPointer(inputChan);
		
		for(int sample = 0; sample < numSamples; ++sample)
		{
			
			// check... and see if this sample is a pitch epoch
			
			// if an epoch is found at this sample val:
			epochLocations->add(sample);
		}
		
	};
	
private:
	
};
