/*
  ==============================================================================

    InputAnalysis.h
    Created: 14 Dec 2020 6:32:56pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GlobalDefinitions.h"


class PitchTracker
{
public:
	PitchTracker();
	~PitchTracker();
	
//	float findPitch(AudioBuffer<float>& inputAudio, const int inputChan, const double samplerate);
	
	float getPitch(AudioBuffer<float>& inputAudio, const int inputChan, const double samplerate);
	
	void setTolerence(const float newTolerence) noexcept { tolerence = newTolerence; }
	
	void setHzLimits(const float newMin, const float newMax) noexcept { minHz = newMin; maxHz = newMax; }
	
	
	
private:
	AudioBuffer<float> yinBuffer;
	float prevDetectedPitch;
//	Array<float> powerTerms;
//	int yinBufferSize;
	
	float tolerence;
	float minHz, maxHz;
	
	float simpleYin(AudioBuffer<float>& inputAudio, const int inputChan) noexcept;
	
	unsigned int minElement(const float* data, const int dataSize) noexcept;
	
	float quadraticPeakPosition (const float *data, unsigned int pos, const int dataSize) noexcept;
	
//	void difference(AudioBuffer<float>& inputBuffer, const int inputChan, const int inputBufferLength);
//
//	void fastDifference(AudioBuffer<float>& inputAudio, const int inputChan);
//
//	void cumulativeMeanNormalizedDifference() const;
//
//	int absoluteThreshold();
//
//	float parabolicInterpolation(int tauEstimate) const;
	
	JUCE_LEAK_DETECTOR(PitchTracker)
};




class EpochFinder
{
public:
	EpochFinder();
	~EpochFinder();
	
	Array<int> extractEpochSampleIndices(AudioBuffer<float>& inputAudio, const int inputChan, const double samplerate);
	
private:
	CriticalSection lock;
	
	Array<float> y;
	Array<float> y2;
	Array<float> y3;
	
	Array<int> epochs;
	
	JUCE_LEAK_DETECTOR(EpochFinder)
};
