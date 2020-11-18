/*
  ==============================================================================

    inputPitchTracker.h
    Created: 16 Nov 2020 7:53:02pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#ifndef TOLERANCE
#define TOLERANCE 0.15
#endif


class PitchTracker
{
	
public:
	
	float returnPitch(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, const double samplerate)
	{
		if(bufferSize != numSamples) {
			yin.setSize(1, numSamples);
			bufferSize = numSamples;
		}
		
		float pitch = calculatePitch(inputBuffer, inputChan, numSamples, samplerate);
		
		if(pitch > 0.0f){
			pitch = samplerate / pitch;
		} else {
			pitch = 0.0f;
		}
		
		return pitch;
	};
	
	
	void clearBuffer() {
		yin.clear();
	};
	
	
private:
	
	int bufferSize;
	
	AudioBuffer<float> yin;

	
	float calculatePitch(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, const double samplerate)
	{
		int period;
		float delta = 0.0f;
		float runningSum = 0.0f;
		float* yinData = yin.getWritePointer(0);
		const float* inputData = inputBuffer.getReadPointer(inputChan);
		
		if(yin.getNumSamples() != numSamples) {
			yin.setSize(1, numSamples);
		}
		
		yinData[0] = 1.0;
		for(int tau = 1; tau < bufferSize; ++tau) {
			yinData[tau] = 0.0f;
			for(int j = 0; j < bufferSize; ++j) {
				delta = inputData[j] - inputData[j + tau];
				yinData[tau] += (delta * delta);
			}
			runningSum += yinData[tau];
			if(runningSum != 0) {
				yinData[tau] *= (tau/runningSum);
			}
			else {
				yinData[tau] = 1.0;
			}
			period = tau - 3;
			if (tau > 4 && yinData[period] < TOLERANCE && yinData[period] < yinData[period + 1]) {
				return quadraticPeakPosition(yin.getReadPointer(0), period);
			}
		}
		
		return quadraticPeakPosition(yin.getReadPointer(0), minElement(yin.getReadPointer(0)));
	};
	
	
	float quadraticPeakPosition(const float* data, unsigned int pos) noexcept
	{
		float s0, s1, s2;
		unsigned int x0, x2;
		if(pos == 0 || pos == bufferSize - 1) return pos;
		x0 = (pos < 1) ? pos : pos - 1;
		x2 = (pos + 1 < bufferSize) ? pos + 1 : pos;
		if(x0 == pos) return (data[pos] <= data[x2]) ? pos : x2;
		if(x2 == pos) return (data[pos] <= data[x0]) ? pos : x0;
		s0 = data[x0];
		s1 = data[pos];
		s2 = data[x2];
		return pos + 0.5 * (s0 - s2) / (s0 - 2.* s1 + s2);
	};
	
	
	
	unsigned int minElement (const float* data) noexcept
	{
		unsigned int j, pos = 0;
		float tmp = data[0];
		for(j = 0; j < bufferSize; ++j) {
			pos = (tmp < data[j]) ? pos : j;
			tmp = (tmp < data[j]) ? tmp : data[j];
		}
		return pos;
	};
	
};
