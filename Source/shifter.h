/*
  ==============================================================================

    shifter.h
    Created: 2 Nov 2020 4:55:40pm
    Author:  Ben Vining
 
 	This class will do the actual pitch shifting computations.
 
 	It should be fed the analysis data of the incoming modulator signal (grain creation) -- so all this class needs to do is the actual *manipulation* & playback of the grains, then resynthesis into the output audio signal.

  ==============================================================================
*/

#pragma once


class Shifter {
	
public:
	
	Shifter() {
		workingBuffer.setSize(0, 1024);
		workingBuffer.clear();
	}
	
	
	void doTheShifting(AudioBuffer<float>& inputBuffer, const int inputChan, AudioBuffer<float>& shiftedBuffer, const int numSamples, const double inputFreq, const float desiredFreq, const int analysisShift, const int analysisShiftHalved) {
		// this function should fill shiftedBuffer with pitch shifted samples from inputBuffer
		// shiftedBuffer is MONO !! only use channel 0
		
		const float* inputReadingfrom = inputBuffer.getReadPointer(inputChan);
		float* ouputWritingto = shiftedBuffer.getWritePointer(0);
		
		workingBuffer.clear();
		
		// pitch shift factor = desired % change of fundamental frequency
		const float scalingFactor = 1.0f + ((inputFreq - desiredFreq)/desiredFreq);
		// PSOLA constants
		const int synthesisShift = round(analysisShift * scalingFactor);
		int analysisIndex = -1;
		int synthesisIndex = 0;
		int analysisBlockStart;
		int analysisBlockEnd;
		int synthesisBlockEnd;
		const int analysisLimit = numSamples - analysisShift - 1;
		// window declaration
	//	int windowLength = analysisShift + analysisShiftHalved + 1;
		
		// PSOLA algorithm
		while (analysisIndex < analysisLimit) {
			// analysis blocks are two pitch periods long
			analysisBlockStart = (analysisIndex + 1) - analysisShiftHalved;
			if (analysisBlockStart < 0) {
				analysisBlockStart = 0;
			}
			analysisBlockEnd = analysisBlockStart + analysisShift + analysisShiftHalved;
			if (analysisBlockEnd > (numSamples - 1)) {
				analysisBlockEnd = numSamples - 1;
			}
			
			// overlap & add
			synthesisBlockEnd = synthesisIndex + analysisBlockEnd - analysisBlockStart;
			int inputIndex = analysisBlockStart;
			int windowIndex = 0;
			
			const float* workingReadingfrom = workingBuffer.getReadPointer(0);
			float* workingWritingto = workingBuffer.getWritePointer(0);
			
			for (int i = synthesisIndex; i <= synthesisBlockEnd; ++i) {
				workingWritingto[i] = workingReadingfrom[i] + ((inputReadingfrom[inputIndex])); // multiply the input by a windowing function within these parentheses in this step !!
				++inputIndex;
				++windowIndex;
			}
			// update pointers
			analysisIndex += analysisShift;
			synthesisIndex += synthesisShift;
		}
		
		
		const float* reading = workingBuffer.getReadPointer(0);
		float* writing = workingBuffer.getWritePointer(0);
		for(int sample = 0; sample < numSamples; ++sample) {
			ouputWritingto[sample] = reading[sample]; // write to output
			writing[sample] = 0.0f; 				   // clear out the working buffer
		}
		
	};
	
	
	void updateDSPsettings(const double newSampleRate, const int newBlockSize) {
		currentSampleRate = newSampleRate;
	};
	
	
	void checkBuffer(const int newBufferSize)
	{
		if(workingBuffer.getNumSamples() != newBufferSize * 2) {
			workingBuffer.setSize(0, newBufferSize * 2);
		}
	};
	
	
private:
	
	AudioBuffer<float> workingBuffer;
	
	double currentSampleRate;
};
