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
		// index 0 should be lowest sample #
		
		epochLocations->clearQuick();
		
		std::vector<int> epochs;
		
	//	const float* input = inputBuffer.getReadPointer(inputChan);
		
		for(int sample = 0; sample < numSamples; ++sample)
		{
			
			// check... and see if this sample is a pitch epoch
			
			// if an epoch is found at this sample val:
			epochs.push_back(sample);
		}
		
		
		// write from temporary vector to output array
		const int size = epochs.size();
		epochLocations->resize(size);
		for(int i = 0; i < size; ++i)
		{
			epochLocations->set(i, epochs[i]);
		}
		
	};
	
private:
	
};
