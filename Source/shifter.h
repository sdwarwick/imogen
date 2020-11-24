/*
  ==============================================================================

    shifter.h
    Created: 2 Nov 2020 4:55:40pm
    Author:  Ben Vining
 
 	This class hosts & performs the resynthesis at each of the desired output pitches.
 
 	@dependencies
 		:  For the esola() function, you need an integer array containing sample #s corresponding to the locations of fundamental frequency pitch peaks. There is a function implementing these calculations in { InputAnalysis -> "EpochExtractor.h" }
 
 		:  Any -SOLA based time or pitch scaling algorithm will have some degree of dependancy on detection of the input's fundamental frequency. This implementation's pitch tracking uses a version of the YIN algorithm, implemented in { InputAnalysis -> "Yin.h" }

  ==============================================================================
*/

#pragma once

#ifndef MAX_BUFFERSIZE
#define MAX_BUFFERSIZE 1024
#endif


class Shifter {
	
public:
	
	Shifter() {
		synthesis.setSize(1, MAX_BUFFERSIZE);
		synthesis.clear();
		finalWindow.ensureStorageAllocated(MAX_BUFFERSIZE);
		window.ensureStorageAllocated(MAX_BUFFERSIZE); // the actual windows between frames of epochs will be smaller, but this is just for safety
	};
	
	
	/*===============================================================================================================================================
	 		time-domain implementation of ESOLA : Epoch-Synchronous Overlap-Add
	 
	 		@param	: inputBuffer			: reference to audio input buffer
	 				: inputChan				: the channel within inputBuffer to read from
	 				: numSamples			: size of inputBuffer, in samples
	 				: epochLocations		: integer array containing sample #s referencing epoch locations within the current input audio vector. This function is designed to be fed epoch locations from the extractEpochIndices() function within { InputAnalysis -> "EpochExtractor.h" }
	 				: inputFreq				: pitch of the input audio, in Hz
	 				: desiredFreq			: desired output pitch, in Hz
	 				: ouputBuffer			: reference to output buffer to write shifted samples to
	 				: numOfEpochsPerFrame	: number of epoch points contained within each analysis frame that the input signal will be broken down into. It is desirable for the analysis frames to contain between two and four pitch periods of the input's fundamental frequency.
	 
	 		@return - writes output signal to HarmonyVoice's shiftedBuffer
	 
			@see   	: "Epoch-Synchronous Overlap-Add (ESOLA) for Time- and Pitch-Scale Modification of Speech Signals", by Sunil Rudresh, Aditya Vasisht, Karthika Vijayan, and Chandra Sekhar Seelamantula, 2018 : http://arxiv.org/pdf/1801.06492.pdf
	 
	 				: ESOLA implementation in C++ by Arjun Variar : http://www.github.com/viig99/esolafast/blob/master/src/esola.cpp
	 
	 				: ESOLA in Python by BaronVladziu : http://www.github.com/BaronVladziu/ESOLA-Implementation/blob/master/ESOLA.py
	 
	 				: "fuzzy" ESOLA in MATLAB by Tim Roberts : http://www.github.com/zygurt/TSM/blob/master/Batch/FESOLA_batch.m
	 ==============================================================================================================================================*/
	void esola(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, Array<int> epochLocations, const float inputFreq, const float desiredFreq, AudioBuffer<float>& outputBuffer, const int numOfEpochsPerFrame) {
		
		int targetLength = 0;
		int highestIndexWrittenTo = -1;
		const float scalingFactor = 1.0f / (1.0f + ((inputFreq - desiredFreq)/desiredFreq)); 
		int lastEpochIndex = epochLocations.getUnchecked(0);
		const int numOfEpochs = epochLocations.size();
		
		if(synthesis.getNumSamples() != numSamples) {
			synthesis.setSize(1, numSamples, false, false, true);
		}
		finalWindow.clearQuick();
		
		for(int i = 0; i < numOfEpochs - numOfEpochsPerFrame; ++i) {
			const int hop = epochLocations.getUnchecked(i + 1) - epochLocations.getUnchecked(i);
			
			if(targetLength >= highestIndexWrittenTo) {
				const int frameLength = epochLocations.getUnchecked(i + numOfEpochsPerFrame) - epochLocations.getUnchecked(i) - 1;
				window.clearQuick();
				calcWindow(frameLength, window);
				const int bufferIncrease = frameLength - highestIndexWrittenTo + lastEpochIndex;
				
				if(bufferIncrease > 0) {
					const float* reading = inputBuffer.getReadPointer(inputChan);
					float* writing = synthesis.getWritePointer(0);
					int writingindex = highestIndexWrittenTo + 1;
					int readingindex = epochLocations.getUnchecked(i) + frameLength - 1 - bufferIncrease;
					int windowreading = frameLength - 1 - bufferIncrease;
					
					for(int s = 0; s < bufferIncrease; ++s) {
						writing[writingindex] = reading[readingindex] * window.getUnchecked(s);
						++writingindex;
						++readingindex;
						finalWindow.add(window.getUnchecked(windowreading));
						++windowreading;
					}
					highestIndexWrittenTo += frameLength - 1;
				}
				
				// OLA
				{
				 int olaindex = epochLocations.getUnchecked(i);
				 const float* olar = synthesis.getReadPointer(0);
				 float* olaw = synthesis.getWritePointer(0);
				 int wolaindex = 0;
				 
				 for(int s = lastEpochIndex; s < lastEpochIndex + frameLength - bufferIncrease; ++s) {
				 	olaw[s] = olar[s] + olar[olaindex];
				 	++olaindex;
				 	const float newfinalwindow = finalWindow.getUnchecked(s) + finalWindow.getUnchecked(wolaindex);
				 	finalWindow.set(s, newfinalwindow);
				 	++wolaindex;
				 }
				}
				
				lastEpochIndex += hop;
			}
			targetLength += ceil(hop * scalingFactor);
		}
		
		// normalize & write to output
		const float* r = synthesis.getReadPointer(0);
		float* w = outputBuffer.getWritePointer(0);
		
		for(int s = 0; s < numSamples; ++s) {
			w[s] = r[s] / std::max<float>(finalWindow.getUnchecked(s), 1e-4);
		}
		
	};
	
	
	
	
	
	/*===============================================================================================================================================
	 a time-domain implementation of PSOLA : Pitch - Synchronous  Overlap - Add
	 
	 This algorithm respaces pitch peaks to the new desired fundamental frequency.
	 
	 @param : inputBuffer	:	audio I/O buffer. The resynthesized signal is constructed in a new, locally hosted AudioBuffer before being transferred to the output.
	 		: inputChan		:	input channel #
	 		: numSamples	:	length of inputBuffer in samples
	 		: peaks			:	pointer to an array containing sample index #s of pitch peak locations within input signal. This function is designed to be fed pitch peak locations from the findPeaks() function within { InputAnalysis -> "EpochExtractor.h" }
	 		: inputFreq		:	detected fundamental frequency of input in Hz
	 		: desiredFreq	:	desired pitch to shift to, in Hz
	 		: outputBuffer	:	AudioBuffer to write the final output signal to. This will be the HarmonyVoice instance's shiftedBuffer
	 
	 @return - writes output signal to HarmonyVoice's shiftedBuffer
	 
	 @see	: example of PSOLA implementation in Python by Sanna Wager : http://www.github.com/sannawag/TD-PSOLA/blob/master/td_psola.py
	 ==============================================================================================================================================*/
	
	void psola(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, Array<int> peaks, const float inputFreq, const float desiredFreq, AudioBuffer<float>& outputBuffer) {
		
		// respace pitch peaks for the new desired synthesis pitch periods
		
		const int numPeaks = peaks.size();
		const float scalingFactor = 1.0f + ((inputFreq - desiredFreq)/desiredFreq);
		
		AudioBuffer<float> newSignal(1, numSamples);
		
		const int newNumPeaks = round(numPeaks * scalingFactor);
		std::vector<float> newPeaksRef = LinearSpacedArray(0, numPeaks - 1, newNumPeaks);
		Array<int> newPeaks(newNumPeaks);
		for(int i = 0; i < newNumPeaks; ++i) {
			const float weight = float(long(newPeaksRef[i]) % long(1));
			const int left = floor(newPeaksRef[i]);
			const int right = ceil(newPeaksRef[i]);
			
			const int newPeak = (peaks.getUnchecked(left) * (1 - weight) + peaks.getUnchecked(right) * weight);
			newPeaks.set(i, newPeak);
		}
		
		std::vector<float> oldPeaks(numPeaks);
		Array<float>* windowing;
		
		// PSOLA
		for(int j = 0; j < newNumPeaks; ++j) {
			
			// first, find the corresponding old peak index
			for(int k = 0; k < numPeaks; ++k) {
				oldPeaks[k] = abs(peaks.getUnchecked(k) - newPeaks.getUnchecked(j));
			}
			int i = int(std::distance(oldPeaks.begin(), std::min_element(oldPeaks.begin(), oldPeaks.end())));
			
			// get the distances to adjacent peaks to determine appropriate length for this synthesis frame
			int P1_0, P1_1; // left and right edge of synthesis frame
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
			if(peaks.getUnchecked(i) - P1_0 < 0) {
				P1_0 = peaks.getUnchecked(i);
			}
			if(peaks.getUnchecked(i) + P1_1 > numSamples - 1) {
				P1_1 = numSamples - 1 - peaks.getUnchecked(i);
			}
			
			// windowing function for OLA
			windowing = new Array<float>(P1_0 + P1_1);
		//	calcWindow(P1_0 + P1_1, windowing);
			
			// resynthesis: center the window from the original signal at the new peak
			const float* addingto = newSignal.getReadPointer(0);
			float* writingto = newSignal.getWritePointer(0);
			const float* input = inputBuffer.getReadPointer(0);
			int inputindex = peaks.getUnchecked(i) - P1_0;
			int windowIndex = 0;
			for (int s = newPeaks.getUnchecked(j) - P1_0; s < newPeaks.getUnchecked(j) + P1_1; ++s) {
				writingto[s] = addingto[s] + (windowing->getUnchecked(windowIndex) * input[inputindex]);
				++windowIndex;
				++inputindex;
			}
			delete windowing;
		}
		
		// write from working buffer to output
		const float* reading = newSignal.getReadPointer(0);
		float* writing = outputBuffer.getWritePointer(0);
		for(int i = 0; i < numSamples; ++i) {
			writing[i] = reading[i];
		}
	};
	
	
	
	
	
	
	
	/*===============================================================================================================================================
	 	a more basic TD-OLA implementation that does not map pitch peaks to the new synthesis windows
	 
	 	this algorithm may prove useful for unpitched frames, as no peak finding algorithm is required, and the synthesis grains can be created based on a consistent ratio, regardless of the pitch of the input...
	 
	 @see	:	"Pitch-Synchronous Waveform Processing Techniques For Text-To-Speech Synthesis Using Diphones", by Eric Moulines and Francis Charpentier : http://courses.grainger.illinois.edu/ece420/sp2017/PSOLA.pdf
	 
	 		:	TD-OLA implementation in C++ by Terry Kong : http://www.github.com/terrykong/Phase-Vocoder/blob/master/PSOLA/PSOLA.cpp
	 ==============================================================================================================================================*/
	void basicOLA(AudioBuffer<float>& inputBuffer, const int inputChan, AudioBuffer<float>& shiftedBuffer, const int numSamples, const double inputFreq, const float desiredFreq, const int analysisShift, const int analysisShiftHalved, const int analysisLimit, Array<float>* window) {
		// this function should fill shiftedBuffer with pitch shifted samples from inputBuffer
		// shiftedBuffer is MONO !! only use channel 0
		
		AudioBuffer<float> workingBuffer;
		
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
	
	AudioBuffer<float> synthesis;
	Array<float> window;
	Array<float> finalWindow;
	

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
	void calcWindow(const int length, Array<float>& window) {
		
	//	if(window.size() != length) { window.resize(length); }
		
		for(int i = 0; i < length; ++i)
		{
			window.add(0.5 - (0.5 * ncos(2, i, length)));
		}
		
	};
	
	float ncos(const int order, const int i, const int size) const
	{
		return std::cos((order * i * MathConstants<float>::pi)/(size - 1));
	};

};


