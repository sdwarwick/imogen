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
 
 	@dependencies
 		"FloatFFT.h", a custom implementation of an FFT process with complexForward and complexInverse operations.
 		In this FFT implementation, data is grouped in pairs of samples, such that fft[i] is the real component and fft[i+1] is the imaginary.

  ==============================================================================
*/

#pragma once

#include "FloatFFT.h"
#include "GlobalDefinitions.h"

#define THRESHOLD 0.15 // between 0.10 ~ 0.15

#define MIN_HZ 50 // simple max/min bounds on detected fundamental frequency
#define MAX_HZ 2000


class Yin
{
public:
	
	Yin(): yinBufferSize(MAX_BUFFERSIZE/2), isPitched(false) {
		yinBuffer.setSize(1, MAX_BUFFERSIZE/2); // yin buffer size is half the size of the input buffer in samples
		yinBuffer.clear();
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
	
	mutable AudioBuffer<float> yinBuffer; // stores the calculated values. half the size of the input audio buffer
	int yinBufferSize; 			  // stores current size of yinBuffer
	
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
		
		FloatFFT* fft; // an FFT object to quickly calculate the difference function
		fft = new FloatFFT(inputBufferLength);
		
		const int doubleInputLength = inputBufferLength * 2;
		float fftBuffer[doubleInputLength];
		float kernel[doubleInputLength];
		float yinStyleACF[doubleInputLength];
		
		// POWER TERM CALCULATION
		float powerTerms[yinBufferSize];
		for(int j = 0; j < yinBufferSize; ++j) { // first, calculate the first power term value...
			powerTerms[0] += reading[j] * reading[j];
		}
		// ... then iteratively calculate all others
		for(int i = 1; i < yinBufferSize; ++i) {
			powerTerms[i] = powerTerms[i - 1] - reading[i - 1] * reading[i - 1] + reading[i + yinBufferSize] * reading[i + yinBufferSize];
		}
		
		// YIN-STYLE AUTOCORRELATION VIA FFT
		// 1. data
		for(int j = 0; j < inputBuffer.getNumSamples(); ++j) {
			fftBuffer[2*j] = reading[j]; // real
			fftBuffer[2*j+1] = 0; // imaginary
		}
		fft->complexForward(fftBuffer);
		
		// 2. half of the data, disguised as a convolution kernel
		for(int j = 0; j < yinBufferSize; ++j) {
			kernel[2*j] = reading[(yinBufferSize-1)-j]; // real
			kernel[2*j+1] = 0; // imaginary
			kernel[2*j+inputBufferLength] = 0; // real
			kernel[2*j+inputBufferLength+1] = 0; // imaginary
		}
		fft->complexForward(kernel);
		
		// 3. convolution via complex multiplication
		for (int j = 0; j < inputBufferLength; ++j) {
			yinStyleACF[2*j] = fftBuffer[2*j] * kernel[2*j] - fftBuffer[2*j+1] * kernel[2*j+1]; // real
			yinStyleACF[2*j+1] = fftBuffer[2*j+1] * kernel[2*j] + fftBuffer[2*j] * kernel[2*j+1]; // imaginary
		}
		fft->complexInverse(yinStyleACF);
		
		// CALCULATION OF DIFFERENCE FUNCTION
		float* w = yinBuffer.getWritePointer(0);
		for(int j = 0; j < yinBufferSize; ++j) {
			w[j] = powerTerms[0] + powerTerms[j] - 2 * yinStyleACF[2 * (yinBufferSize - 1 + j)];
		}
		
		delete fft;
	};
	
	
	/*
	 CUMULATIVE MEAN NORMALIZED DIFFERENCE FUNCTION
	 @brief implements the cumulative mean normalized difference function described in step 3 of the YIN paper
	 @code
	 	yinBuffer[0] == yinBuffer[1] == 1;
	 @endcode
	 */
	void cumulativeMeanNormalizedDifference() const {
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
	
	
	/*
	 ABSOLUTE THRESHOLD
	 @brief implements step 4 of the YIN paper, an absolute threshold
	 @return 	tau value
	 */
	int absoluteThreshold() {
		// implements step 4 of the YIN paper
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
	
	
	/*
	 PARABOLIC INTERPOLATION
	 @brief implements step 5 of the YIN paper, parabolic interpolation. This step refines the estimated tau value. This is needed to detect higher frequencies more precisely.
	 @param 	tauEstimate		the estimated tau value returned from absoluteThreshold()
	 @return	a more precise tau value, parabolically interpolated with local minima of d'(tau)
	 */
	float parabolicInterpolation(int tauEstimate) const {
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
