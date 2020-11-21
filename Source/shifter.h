/*
  ==============================================================================

    shifter.h
    Created: 2 Nov 2020 4:55:40pm
    Author:  Ben Vining
 
 	This class hosts & performs the resynthesis at each of the desired output pitches.

  ==============================================================================
*/

#pragma once


class Shifter {
	
public:
	
	Shifter(): currentSampleRate(44100) {
		workingBuffer.setSize(0, 1024);
		workingBuffer.clear();
	}
	
	
	/*===============================================================================================================================================
	 a time-domain implementation of ESOLA : Epoch - Synchronous  Overlap - Add
	 
	 @param : inputBuffer	:	audio I/O buffer. The resynthesized signal is constructed in a new, locally hosted AudioBuffer before being transferred to the output.
	 
	 @param	: inputChan		:	input channel #
	 
	 		: numSamples	:	length of inputBuffer in samples
	 
	 		: peaks			:	pointer to an array containing sample index #s of pitch epoch locations within input signal { see InputAnalysis -> "EpochExtractor.h" }
	 
	 		: inputFreq		:	detected fundamental frequency of input in Hz
	 
	 		: desiredFreq	:	desired pitch to shift to, in Hz
	 
	 		: outputBuffer	:	AudioBuffer to write the final output signal to. This will be the HarmonyVoice instance's shiftedBuffer
	 
	 @return - writes output signal to HarmonyVoice's shiftedBuffer
	 
	 @see   : "Epoch-Synchronous Overlap-Add (ESOLA) for Time- and Pitch-Scale Modification of Speech Signals", by Sunil Rudresh, Aditya Vasisht, Karthika Vijayan, and Chandra Sekhar Seelamantula, 2018 : http://arxiv.org/pdf/1801.06492.pdf
	 
	 		: example of ESOLA implementation in Python by Sanna Wager : http://www.github.com/sannawag/TD-PSOLA/blob/master/td_psola.py
	 
	 		: example of ESOLA in C++ by Arjun Variar : http://www.github.com/viig99/esolafast/blob/master/src/esola.cpp
	 ==============================================================================================================================================*/
	
	void esola(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, Array<int>* peaks, const float inputFreq, const float desiredFreq, AudioBuffer<float>& outputBuffer) {
		
		const int numPeaks = peaks->size();
		const float scalingFactor = 1.0f + ((inputFreq - desiredFreq)/desiredFreq);
		const int newNumPeaks = round(numPeaks * scalingFactor);
		
		AudioBuffer<float> newSignal(1, numSamples);
		
		// respace epochs / pitch peaks for the new desired synthesis pitch periods
		std::vector<float> newPeaksRef = LinearSpacedArray(0, numPeaks - 1, newNumPeaks);
		Array<int> newPeaks(newNumPeaks);
		for(int i = 0; i < newNumPeaks; ++i) {
			const float weight = float(long(newPeaksRef[i]) % long(1));
			const int left = floor(newPeaksRef[i]);
			const int right = ceil(newPeaksRef[i]);
			
			const int newPeak = (peaks->getUnchecked(left) * (1 - weight) + peaks->getUnchecked(right) * weight);
			newPeaks.set(i, newPeak);
		}
		
		std::vector<float> oldPeaks(numPeaks);
		Array<float>* windowing;
		
		// ESOLA
		for(int j = 0; j < newNumPeaks; ++j) {
			
			// first, find the corresponding old peak index
			for(int k = 0; k < numPeaks; ++k) {
				oldPeaks[k] = abs(peaks->getUnchecked(k) - newPeaks.getUnchecked(j));
			}
			int i = int(std::distance(oldPeaks.begin(), std::min_element(oldPeaks.begin(), oldPeaks.end())));
			
			// get the distances to adjacent peaks
			int P1_0, P1_1;
			if(j == 0) {
				P1_0 = newPeaks.getUnchecked(j);
			} else {
				P1_0 = newPeaks.getUnchecked(j) - newPeaks.getUnchecked(j - 1);
			}
			if(j == newNumPeaks - 1) {
				P1_1 = numSamples - 1 - newPeaks.getUnchecked(j);
			} else {
				P1_1 = newPeaks.getUnchecked(j + 1) - newPeaks.getUnchecked(j);
			}
			// edge case truncation
			if(peaks->getUnchecked(i) - P1_0 < 0) {
				P1_0 = peaks->getUnchecked(i);
			}
			if(peaks->getUnchecked(i) + P1_1 > numSamples - 1) {
				P1_1 = numSamples - 1 - peaks->getUnchecked(i);
			}
			
			// windowing function for OLA
			windowing = new Array<float>(P1_0 + P1_1);
			calcWindow(P1_0 + P1_1, windowing);
			
			const float* addingto = newSignal.getReadPointer(0);
			float* writingto = newSignal.getWritePointer(0);
			const float* input = inputBuffer.getReadPointer(0);
			int inputindex = peaks->getUnchecked(i) - P1_0;
			int windowIndex = 0;
			for (int s = newPeaks.getUnchecked(j) - P1_0; s < newPeaks.getUnchecked(j) + P1_1; ++s) {
				writingto[s] = addingto[s] + (windowing->getUnchecked(windowIndex) * input[inputindex]);
				++windowIndex;
				++inputindex;
			}
		}
		
		// write from working buffer to output
		const float* reading = newSignal.getReadPointer(0);
		float* writing = outputBuffer.getWritePointer(0);
		for(int i = 0; i < numSamples; ++i) {
			writing[i] = reading[i];
		}
	};
	
	
	
	
	/*===============================================================================================================================================
	 	a more basic TD-PSOLA implementation that does not map pitch peaks to the new synthesis windows
	 
	 @see	:	"Pitch-Synchronous Waveform Processing Techniques For Text-To-Speech Synthesis Using Diphones", by Eric Moulines and Francis Charpentier : http://courses.grainger.illinois.edu/ece420/sp2017/PSOLA.pdf
	 
	 		:	TD-PSOLA implementation in C++ by Terry Kong : http://www.github.com/terrykong/Phase-Vocoder/blob/master/PSOLA/PSOLA.cpp
	 ==============================================================================================================================================*/
	void doTheShifting(AudioBuffer<float>& inputBuffer, const int inputChan, AudioBuffer<float>& shiftedBuffer, const int numSamples, const double inputFreq, const float desiredFreq, const int analysisShift, const int analysisShiftHalved, const int analysisLimit, Array<float>* window) {
		// this function should fill shiftedBuffer with pitch shifted samples from inputBuffer
		// shiftedBuffer is MONO !! only use channel 0
		
		const float* inputReadingfrom = inputBuffer.getReadPointer(inputChan);
		
		// pitch shift factor = desired % change of fundamental frequency
		const float scalingFactor = 1.0f + ((inputFreq - desiredFreq)/desiredFreq);
		
		// PSOLA constants
		// analysisShift = ceil(lastSampleRate/voxCurrentPitch); the # of samples being processed in current frame
		const int synthesisShift = round(analysisShift * scalingFactor); // the # of samples synthesized with each OLA
		int analysisIndex = -1; // or analysisIndex = epochLocations->getUnchecked(0) + 1
		int synthesisIndex = 0;
		int analysisBlockStart;
		int analysisBlockEnd;
		int synthesisBlockEnd;
		
		// PSOLA algorithm
		while (analysisIndex < analysisLimit) {
			// analysis blocks are two pitch periods long
			analysisBlockStart = analysisIndex + 1 - analysisShiftHalved;
			if (analysisBlockStart < 0) { analysisBlockStart = 0; };
			analysisBlockEnd = analysisBlockStart + analysisShift;   // original was analysisBlockEnd = analysisBlockStart + analysisShiftHalved
			if (analysisBlockEnd > (numSamples - 1)) { analysisBlockEnd = numSamples - 1; };
			
			// overlap & add
			synthesisBlockEnd = synthesisIndex + analysisBlockEnd - analysisBlockStart;
			int inputIndex = analysisBlockStart;
			int windowIndex = 0;
			const float* workingReadingfrom = workingBuffer.getReadPointer(0);
			float* workingWritingto = workingBuffer.getWritePointer(0);
			for (int i = synthesisIndex; i <= synthesisBlockEnd; ++i) {
				workingWritingto[i] = workingReadingfrom[i] + (inputReadingfrom[inputIndex] * window->getUnchecked(windowIndex));
				++inputIndex;
				++windowIndex;
			}
			// update pointers
			analysisIndex += analysisShift; // or analysisIndex = epochLocations->getUnchecked(epochIndex) + 1
			synthesisIndex += synthesisShift;
		}
		
		
		const float* reading = workingBuffer.getReadPointer(0);
		float* ouputWritingto = shiftedBuffer.getWritePointer(0);
		for(int sample = 0; sample < numSamples; ++sample) {
			ouputWritingto[sample] = reading[sample];
		}
		
	};
	
	
	
private:
	
	AudioBuffer<float> workingBuffer;
	
	double currentSampleRate;
	

	//===============================================================================================
	// SOME HELPER FUNCTIONS :
	
	// returns a dynamically sized vector of evenly spaced values within the specified range
	std::vector<float> LinearSpacedArray(const int a, const int b, std::size_t N) const {
		float h = (b - a) / static_cast<float>(N-1);
		std::vector<float> xs(N);
		std::vector<float>::iterator x;
		float val;
		for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h) {
			*x = val;
		}
		return xs;
	};
	
	// calculates values for a variable-size Hanning window
	void calcWindow(const int length, Array<float>* window) {
		
		if(window->size() != length) { window->resize(length); }
		
		for(int i = 0; i < length; ++i)
		{
			window->set(i, 0.5 - (0.5 * ncos(2, i, length)));
		}
		
	};
	
	float ncos(const int order, const int i, const int size) const
	{
		return std::cos((order * i * MathConstants<float>::pi)/(size - 1));
	};

};
