/*
  ==============================================================================

    inputPitchTracker.h
    Created: 16 Nov 2020 7:53:02pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once


class PitchTracker
{
	
public:
	
	float returnPitch(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples)
	{
		
		const float* input = inputBuffer.getReadPointer(inputChan);
		
		return 0.0f;
	};
	
	
private:
	
};
