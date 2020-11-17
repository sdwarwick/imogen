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
	
	PitchTracker(const int levels, const int windowLen): oldFreq(0.0f), dLength(0), oldMode(0) {
		// levels = number of levels for the FLWT algorithm
		// windowLen = length of the analysis windows
		
		window = new int[windowLen];
		this->levels = levels;
		maxCount = new int[levels];
		minCount = new int[levels];
		maxIndices = new int[windowLen];
		minIndices = new int[windowLen];
		mode = new int[levels];
		windowLength = windowLen;
		differs = new int[windowLen];
	};
	
	~PitchTracker() {
		delete[] window;
		delete[] maxCount;
		delete[] minCount;
		delete[] maxIndices;
		delete[] minIndices;
		delete[] mode;
		delete[] differs;
	}
	
	
	float returnPitch(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, const double samplerate)
	{
		
		const float* input = inputBuffer.getReadPointer(inputChan);
		
		int newWidth = (numSamples > windowLength) ? windowLength : numSamples;
		long average = 0;
		float globalMax = 0.0f;
		float globalMin = 0.0f;
		float maxThresh;
		float minThresh;
		
		for(int i = 0; i < numSamples; ++i) {
			window[i] = input[i];
			average += input[i];
			if(input[i] > globalMax) {
				globalMax = input[i];
			}
			if(input[i] < globalMin) {
				globalMin = input[i];
			}
		}
		average /= numSamples;
		
		int minDist;
		int climber;
		bool isSearching;
		int tooClose;
		int test;
		
		for (int lev = 0; lev < levels; ++lev) {
			// re initialize level parameters
			mode[lev] = 0;
			maxCount[lev] = 0;
			minCount[lev] = 0;
			isSearching = true;
			tooClose = 0;
			dLength = 0;
			
			newWidth = newWidth >> 1;
	//		minDist = max(floor((samplerate/maxFreq) >> (lev + 1)), 1);
			
			if((window[3] + window[2] - window[1] - window[0]) > 0) {
				climber = 1;
			} else {
				climber = -1;
			}
			
			window[0] = (window[1] + window[0]) >> 1;
			for(int j = 1; j < newWidth; ++j) {
				window[j] = (window[2*j+1] + window[2*j]) >> 1;
				
				test = window[j] - window[j-1];
				
				if(climber >= 0 && test < 0) {
					if(window[j-1] >= maxThresh && isSearching && !tooClose) {
						maxIndices[maxCount[lev]] = j - 1;
						maxCount[lev]++;
						isSearching = false;
						tooClose = minDist;
					}
					climber = -1;
				} else if (climber <= 0 && test > 0) {
					if(window[j-1]<= minThresh && isSearching && !tooClose) {
						minIndices[minCount[lev]] = j - 1;
						minCount[lev]++;
						isSearching = false;
						tooClose = minDist;
					}
					climber = 1;
				}
				
				if((window[j] <= average && window[j-1] > average) || (window[j]>= average && window[j-1] < average)) {
					isSearching = true;
				}
				
				if(tooClose) {
					tooClose--;
				}
			}
			
			if(maxCount[lev] >= 2 && minCount[lev] >= 2) {
				
			}
			
		}
		
		return 0.0f;
	};
	
	
private:
	
	int windowLength;
	
	int* window;
	int* maxCount;
	int* minCount;
	int* maxIndices;
	int* minIndices;
	int* mode;
	int* differs;
	
	int levels;
	
	int dLength;
	
	float oldFreq;
	int oldMode;
								 
	float maxFreq = 44100.0f;
	
	int max(const int x, const int y) {
		if(x > y) {
			return x;
		} else {
			return y;
		}
	}
};
