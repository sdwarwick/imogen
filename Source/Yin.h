/*
  ==============================================================================

    Yin.h
    Created: 20 Nov 2020 2:38:03am
    Author:  Ben Vining
 
 	This class implements the YIN pitch tracking algorithm, as described in "http://recherche.ircam.fr/equipes/pcm/cheveign/ps/2002_JASA_YIN_proof.pdf". This implementation uses an FFT to calculate the difference function in step 2 to increase performance and computational speed.
 
 	@see
 		: "YIN, a fundamental frequency estimator for speech and music", by Alain de Cheveigne ́ and Hideki Kawahara, 2002
 
 	@see
 		: YIN implementation in Java by Matthias Mauch, Joren Six and Paul Brossier : http://www.github.com/JorenSix/TarsosDSP/blob/master/src/core/be/tarsos/dsp/pitch/FastYin.java
 
 	@see
 		: YIN implementation in Python by Patrice Guyot : http://www.github.com/patriceguyot/Yin/blob/master/yin.py

  ==============================================================================
*/

#pragma once

#include "GlobalDefinitions.h"

#define THRESHOLD 0.15 // between 0.10 ~ 0.15

#define MIN_HZ 50 // simple max/min bounds on detected fundamental frequency
#define MAX_HZ 2000


class Yin
{
public:
	
	Yin(): yinBufferSize(round(MAX_BUFFERSIZE/2)), yinBuffer(1, yinBufferSize), isPitched(false) {
		powerTerms.ensureStorageAllocated(yinBufferSize);
		powerTerms.clearQuick();
		powerTerms.fill(0);
	};
	
	/*===============================================================================================================================================
	 	PITCH DETECTION RESULT
	 	@brief	this function is the main flow of the YIN algorithm.
	 	@param	inputBuffer		buffer to read audio from
	 	@param	inputChan		channel # of inputBuffer to read audio from
	 	@param	numSamples		inputBuffer length in samples
	 	@param	samplerate		current samplerate in Hz
	 	@return	current input pitch in Hz, or -1 if no pitch is detected
	 ==============================================================================================================================================*/
	float pitchDetectionResult(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, const double samplerate) {
		
		checkBufferSize(numSamples);
		
		int tauEstimate;
		float pitchInHz;
		
		difference(inputBuffer, inputChan, numSamples);
		
		cumulativeMeanNormalizedDifference();
		
		tauEstimate = absoluteThreshold();
		
		if(tauEstimate != -1) {
			pitchInHz = samplerate / parabolicInterpolation(tauEstimate);
			if(pitchInHz >= MIN_HZ && pitchInHz <= MAX_HZ) {
				isPitched = true;
			} else {
				pitchInHz = -1.0f;
				isPitched = false;
			}
		} else {
			pitchInHz = -1.0f;
			isPitched = false;
		}
		
		return pitchInHz;
	};
	
	
	// some helper functions ================//
	bool isitPitched() {
		return isPitched;
	};
	
	void checkBufferSize(const int newBlockSize) {
		if(yinBufferSize != round(newBlockSize / 2)) {
			yinBufferSize = round(newBlockSize / 2);
			yinBuffer.setSize(1, yinBufferSize, false, true, true);
		}
	};
	
	void clearBuffer() {
		yinBuffer.clear();
	};
	//======================================//
	
	
private:
	int yinBufferSize; 			  // stores current size of yinBuffer
	mutable AudioBuffer<float> yinBuffer; // stores the calculated values. half the size of the input audio buffer
	Array<float> powerTerms;
	
	bool isPitched;				  // stores whether the current audio vector is determined to be pitched or unpitched
	

	void difference(AudioBuffer<float>& inputBuffer, const int inputChan, const int inputBufferLength) {
		/*
		 THE DIFFERENCE FUNCTION
		 @brief implements the difference function described in step 2 of the YIN paper, using an FFT to increase computational efficiency
		 @param 	inputBuffer			reference to audio input buffer
		 @param		inputChan			channel # to read audio from in input buffer
		 @param		inputBufferLength	input buffer length, in samples
		 */
		
		const float* reading = inputBuffer.getReadPointer(inputChan);
		
		const int fftOrder = log2(inputBufferLength);
		
		dsp::FFT fft(fftOrder);
		
		dsp::Complex<float> fftBufferIn[inputBufferLength];
		dsp::Complex<float> fftBufferOut[inputBufferLength];
		dsp::Complex<float> kernelIn[inputBufferLength];
		dsp::Complex<float> kernelOut[inputBufferLength];
		dsp::Complex<float> yinStyleACFin[inputBufferLength];
		dsp::Complex<float> yinStyleACFout[inputBufferLength];
		
		// POWER TERM CALCULATION
		{
			powerTerms.clearQuick();
			for(int j = 0; j < yinBufferSize; ++j) { // first, calculate the first power term value...
				powerTerms.add(reading[j] * reading[j]);
			}
			// ... then iteratively calculate all others
			for(int i = 1; i < yinBufferSize; ++i) {
				powerTerms.add(powerTerms.getUnchecked(i - 1) - reading[i - 1] * reading[i - 1] + reading[i + yinBufferSize] * reading[i + yinBufferSize]);
			}
		}
		
		// YIN-STYLE AUTOCORRELATION VIA FFT
		{
			// 1. data
			for(int j = 0; j < inputBufferLength; ++j) {
				fftBufferIn[j] = { reading[j], 0 };
			}
			fft.perform(fftBufferIn, fftBufferOut, false);
			
			// 2. half of the data, disguised as a convolution kernel
			for(int j = 0; j < yinBufferSize; ++j) {
				kernelIn[j] = { reading[(yinBufferSize-1)-j], 0 };
			}
			fft.perform(kernelIn, kernelOut, false);
			
			// 3. convolution via complex multiplication
			for (int j = 0; j < inputBufferLength; ++j) {
				
				yinStyleACFin[j] = { fftBufferOut[j].real() * kernelOut[j].real() - fftBufferOut[j+1].real() * kernelOut[j+1].real(), fftBufferOut[j].imag() * kernelOut[j].imag() + fftBufferOut[j+1].imag() * kernelOut[j+1].imag() };
				
			}
			fft.perform(yinStyleACFin, yinStyleACFout, true);
		}
		
		// CALCULATION OF DIFFERENCE FUNCTION
		float* w = yinBuffer.getWritePointer(0);
		for(int j = 0; j < yinBufferSize; ++j) {
			w[j] = powerTerms.getUnchecked(0) + powerTerms.getUnchecked(j) - 2 * yinStyleACFout[yinBufferSize - 1 + j].real() / fft.getSize();
		}
		
	};
	
	
	void cumulativeMeanNormalizedDifference() const {
		/*
		 CUMULATIVE MEAN NORMALIZED DIFFERENCE FUNCTION
		 @brief implements the cumulative mean normalized difference function described in step 3 of the YIN paper
		 @code
		 yinBuffer[0] == yinBuffer[1] == 1;
		 @endcode
		 */
		int tau;
		float* w = yinBuffer.getWritePointer(0);
		const float* r = yinBuffer.getReadPointer(0);
		w[0] = 1;
		float runningSum = 0.0f;
		for(tau = 1; tau < yinBufferSize; ++tau) {
			runningSum += r[tau];
			w[tau] = r[tau] * tau / runningSum;
		}
	};
	
	
	int absoluteThreshold() {
		/*
		 ABSOLUTE THRESHOLD
		 @brief implements step 4 of the YIN paper, an absolute threshold
		 @return 	tau value
		 */
		int tau;
		float probabilityEstimate;
		const float* read = yinBuffer.getReadPointer(0);
		
		// first two positions in yinBuffer are always 1
		for(tau = 2; tau < yinBufferSize; ++tau) {
			if(read[tau] < THRESHOLD) {
				while (tau + 1 < yinBufferSize && read[tau + 1] < read[tau]) {
					++tau;
				}
				probabilityEstimate = 1 - read[tau];
				break;
			}
		}
		
		// if no pitch is detected, tau => -1
		if(tau == yinBufferSize || read[tau] >= THRESHOLD || probabilityEstimate > 1.0f) {
			tau = -1;
			isPitched = false;
		} else {
			isPitched = true;
		}
		
		return tau;
	};
	
	
	
	float parabolicInterpolation(int tauEstimate) const {
	/*
	 PARABOLIC INTERPOLATION
	 @brief implements step 5 of the YIN paper, parabolic interpolation. This step refines the estimated tau value. This is needed to detect higher frequencies more precisely.
	 @param 	tauEstimate		the estimated tau value returned from absoluteThreshold()
	 @return	a more precise tau value, parabolically interpolated with local minima of d'(tau)
	 */
		float betterTau;
		int x0;
		int x2;
		
		if(tauEstimate < 1) {
			x0 = tauEstimate;
		} else {
			x0 = tauEstimate - 1;
		}
		
		if (tauEstimate + 1 < yinBufferSize) {
			x2 = tauEstimate + 1;
		} else {
			x2 = tauEstimate;
		}
		
		if (x0 == tauEstimate) {
			const float* r = yinBuffer.getReadPointer(0);
			if(r[tauEstimate] <= r[x2]) {
				betterTau = tauEstimate;
			} else {
				betterTau = x2;
			}
		} else if (x2 == tauEstimate) {
			const float* r = yinBuffer.getReadPointer(0);
			if(r[tauEstimate] <= r[x0]) {
				betterTau = tauEstimate;
			} else {
				betterTau = x0;
			}
		} else {
			const float* r = yinBuffer.getReadPointer(0);
			float s0, s1, s2;
			s0 = r[x0];
			s1 = r[tauEstimate];
			s2 = r[x2];
			betterTau = tauEstimate + (s2 - s0) / (2 * (2 * s1 - s2 - s0));
		}
		
		return betterTau;
	};
	
};
