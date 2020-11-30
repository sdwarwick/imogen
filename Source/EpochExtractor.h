/*
  ==============================================================================

    EpochExtractor.h
    Created: 17 Nov 2020 3:37:46am
    Author:  Ben Vining
 
 	This class detects the locations of maximum energy in the period of the input signal's fundamental frequency. The ESOLA algorithm attempts to center its analysis grains on these pitch marks, and stretch them closer together or farther apart to achieve the pitch scaling.

  ==============================================================================
*/

#pragma once

#include "GlobalDefinitions.h"


class EpochExtractor {
	
public:
	
	EpochExtractor() {
		epochs.ensureStorageAllocated(MAX_BUFFERSIZE);
		epochs.clearQuick();
		y.reserve(MAX_BUFFERSIZE);
		y2.reserve(MAX_BUFFERSIZE);
		y3.reserve(MAX_BUFFERSIZE);
	};
	
	
	/*
	 EXTRACT EPOCH INDICES
	 
	 uses a ZFR approach to save sample index #s of epoch locations to an integer array
	 
	 the output array is designed to be fed into the esola() algorithm within { PSOLA -> "shifter.h" }
	 
	 @see : "Epoch Extraction From Speech Signals", by K. Sri Rama Murty and B. Yegnanarayana, 2008 : http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=6D94C490DA889017DE4362D322E1A23C?doi=10.1.1.586.7214&rep=rep1&type=pdf
	 
	 @see : example of ESOLA in C++ by Arjun Variar : http://www.github.com/viig99/esolafast/blob/master/src/esola.cpp
	 */
	
	Array<int> extractEpochIndices(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, const double samplerate)
	{
		const int window_length = round(0.015 * samplerate);
		
		epochs.clearQuick();
		y.resize(numSamples);
		y2.resize(numSamples);
		y3.resize(numSamples);
		
		float mean_val;
		float running_sum;
		
		const float* data = inputAudio.getReadPointer(inputChan);
		
		const float x0 = data[0];
		const float x1 = data[1] - x0;
		float y1_0 = x0;
		float y1_1 = x1 + (2.0f * y1_0);
		float x_i;
		float y1_i;
		y2[0] = y1_0;
		y2[1] = y1_1 + (2.0f * y1_0);
		for(int i = 2; i < numSamples; ++i) {
			x_i = data[i] - data[i - 1];
			y1_i = x_i + (2.0f * y1_1) - y1_0;
			y2[i] = y1_i + (2.0f * y2[i - 1]) - y2[i - 2];
			y1_0 = y1_1;
			y1_1 = y1_i;
		}
		
		// third stage
		running_sum = std::accumulate(y2.begin(), y2.begin() + 2 * window_length + 2, 0.0);
		mean_val = 0.0f;
		for(int i = 0; i < numSamples; ++i) {
			if((i - window_length < 0) || (i + window_length >= numSamples)) {
				mean_val = y2[i];
			} else if (i - window_length == 0) {
				mean_val = running_sum / (2.0f * window_length + 1.0f);
			} else {
				running_sum -= y2[i - window_length - 1] - y2[i + window_length];
				mean_val = running_sum / (2.0f * window_length + 1.0f);
			}
			y3[i] = y2[i] - mean_val;
		}
		
		// fourth stage
		running_sum = std::accumulate(y3.begin(), y3.begin() + 2 * window_length + 2, 0.0);
		mean_val = 0.0f;
		for(int i = 0; i < numSamples; ++i) {
			if((i - window_length < 0) || (i + window_length >= numSamples)) {
				mean_val = y3[i];
			} else if (i - window_length == 0) {
				mean_val = running_sum / (2.0f * window_length + 1.0f);
			} else {
				running_sum -= y3[i - window_length - 1] - y3[i + window_length];
				mean_val = running_sum / (2.0f * window_length + 1.0f);
			}
			y[i] = y3[i] - mean_val;
		}
		
		// last stage
		float last = y[0];
		float act;
		epochs.add(0);
		for(int i = 0; i < numSamples; ++i) {
			act = y[i];
			if(last < 0 and act > 0) {
				epochs.add(i);
			}
			last = act;
		}
		epochs.add(numSamples - 1);
		
		return epochs;
	};
	
	
	
	/* USED FOR PSOLA
	 	locates sample indices of pitch peak locations in input audio vector. the output array is designed to be fed into the psola() function within { PSOLA -> "shifter.h" }
	 
	 	@TODO	complete implementation of computePeriodsPerSequence() with FFT nonsense & etc...
	 */
	Array<int> findPeaks(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, const double samplerate, const int minHz, const int maxHz, const int analysisWinMs, const float maxChange, const float minChange) {
		// finds sample indices of pitch peaks of input signal's fundamenal frequency
		
		int minPeriod = floor(samplerate/maxHz);
		int maxPeriod = floor(samplerate/minHz);
		
		int sequence = analysisWinMs / 1000 * samplerate;
		
		Array<int> periods = computePeriodsPerSequence(inputAudio, inputChan, numSamples, sequence, minPeriod, maxPeriod);
		
		// hack to avoid octave error : assume that period shouldn't vary widely; restrict range
		float runningsum = 0.0f;
		for(int i = 0; i < periods.size(); ++i) {
			runningsum += periods.getUnchecked(i);
		}
		const float meanPeriod = runningsum/periods.size();
		minPeriod = round(meanPeriod * 0.9f);
		maxPeriod = round(meanPeriod * 1.1f);
		
		periods = computePeriodsPerSequence(inputAudio, inputChan, numSamples, sequence, minPeriod, maxPeriod);
		
		// find the peaks
		const int tsmaxindex = round(periods.getUnchecked(0) * 1.1f) - 1;
		Array<float> testingsignal;
		const float* input = inputAudio.getReadPointer(inputChan);
		for(int i = 0; i < tsmaxindex; ++i) {
			testingsignal.add(input[i]);
		}
		
		Array<int> peaks(maxVal(testingsignal));
		
		while(true) {
			const int prev = peaks.getLast();
			const int idx = floor(prev / sequence);
			if (prev + round(periods.getUnchecked(idx) * maxChange) >= numSamples) { break; }
			
			Array<float> testingsig;
			const float* reading = inputAudio.getReadPointer(inputChan);
			for(int i = prev + round(periods.getUnchecked(idx) * minChange); i < prev + round(periods.getUnchecked(idx) * maxChange); ++i) {
				testingsig.add(reading[i]);
			}
			const float appendedVal = prev + int(periods[idx] * minChange) + maxVal(testingsig);
			peaks.add(appendedVal);
		}
		
		return peaks;
	};
	
	
private:
	
	Array<int> epochs;
	std::vector<float> y;
	std::vector<float> y2;
	std::vector<float> y3;
	
	
	//used for psola
	Array<int> computePeriodsPerSequence(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, const int sequenceLength, const int minPeriod, const int maxPeriod) {
		// computes periodicity of time domain signal using autocorrelation . helper function for findPeaks()
		
		int offset = 0;
		Array<int> periods;
		
		while (offset < numSamples)
		{
			
			//pseudocode from python:
//			fourier = fft(signal[offset: offset + sequence])
//			fourier[0] = 0  # remove DC component
//			autoc = ifft(fourier * np.conj(fourier)).real
//			autoc_peak = min_period + np.argmax(autoc[min_period: max_period])
//			periods.append(autoc_peak)
			
			offset += sequenceLength;
		}
		
		return periods;
	};
	
	
	// finds max value in an array
	float maxVal(Array<float>& input) const {
		
		float currentMax = 0.0f;
		
		for(int i = 0; i < input.size(); ++i) {
			if(input.getUnchecked(i) > currentMax) {
				currentMax = input.getUnchecked(i);
			}
		}
		return currentMax;
	};
	
};
