/*
  ==============================================================================

    EpochExtractor.h
    Created: 17 Nov 2020 3:37:46am
    Author:  Ben Vining
 
 	This class detects the locations of maximum energy in the period of the input signal's fundamental frequency. The ESOLA algorithm attempts to center its analysis grains on these pitch marks, and stretch them closer together or farther apart to achieve the pitch scaling.

  ==============================================================================
*/

#pragma once


class EpochExtractor {
	
public:
	
	/*
	 EXTRACT EPOCH INDICES
	 
	 uses a ZFR approach to save sample index #s of fundamental pitch epochs to an integer array
	 
	 @see : "Epoch Extraction From Speech Signals", by K. Sri Rama Murty and B. Yegnanarayana, 2008 : http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=6D94C490DA889017DE4362D322E1A23C?doi=10.1.1.586.7214&rep=rep1&type=pdf
	 
	 @see : example of ESOLA in C++ by Arjun Variar : http://www.github.com/viig99/esolafast/blob/master/src/esola.cpp
	 */
	
	Array<int> extractEpochIndices(AudioBuffer<float>& inputAudio, const int inputChan, const int numSamples, const double samplerate)
	{
		const int window_length = round(0.015 * samplerate);
		
		Array<int> epochs;
		
		std::vector<float> y(numSamples);
		std::vector<float> y2(numSamples);
		std::vector<float> y3(numSamples);
		
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
	
	
private:
	
};
