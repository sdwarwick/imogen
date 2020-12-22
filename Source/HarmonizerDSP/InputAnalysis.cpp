/*
  ==============================================================================

    InputAnalysis.cpp
    Created: 14 Dec 2020 6:32:56pm
    Author:  Ben Vining

  ==============================================================================
*/

#include "InputAnalysis.h"

#define THRESHOLD 0.15 // between 0.10 ~ 0.15
#define MIN_HZ 50 // simple max/min bounds on detected fundamental frequency
#define MAX_HZ 2000



PitchTracker::PitchTracker(): yinBufferSize(round(MAX_BUFFERSIZE/2))
{
	yinBuffer.setSize(1, round(MAX_BUFFERSIZE/2));
	powerTerms.ensureStorageAllocated(round(MAX_BUFFERSIZE/2));
	powerTerms.clearQuick();
	powerTerms.fill(0);
};

PitchTracker::~PitchTracker()
{ };

float PitchTracker::findPitch(AudioBuffer<float>& inputAudio, const int inputChan, const int startSample, const int numSamples, const double samplerate)
{
/*	Implements the YIN pitch tracking algorithm, as described in "http://recherche.ircam.fr/equipes/pcm/cheveign/ps/2002_JASA_YIN_proof.pdf". This implementation uses an FFT to calculate the difference function in step 2 to increase performance and computational speed.

	@see
	: "YIN, a fundamental frequency estimator for speech and music", by Alain de Cheveigne ́ and Hideki Kawahara, 2002

	@see
	: YIN implementation in Java by Matthias Mauch, Joren Six and Paul Brossier : http://www.github.com/JorenSix/TarsosDSP/blob/master/src/core/be/tarsos/dsp/pitch/FastYin.java

	@see
	: YIN implementation in Python by Patrice Guyot : http://www.github.com/patriceguyot/Yin/blob/master/yin.py
 */
	if(yinBufferSize != numSamples)
	{
		yinBufferSize = round(numSamples/2);
		yinBuffer.setSize(1, round(numSamples/2), false, true, true);
	}
	
	int tauEstimate;
	float pitchInHz;
	
	difference(inputAudio, inputChan, numSamples, startSample);
	
	cumulativeMeanNormalizedDifference();
	
	tauEstimate = absoluteThreshold();
	
	if(tauEstimate != -1) {
		pitchInHz = samplerate / parabolicInterpolation(tauEstimate);
		if(pitchInHz >= MIN_HZ && pitchInHz <= MAX_HZ) {
		} else {
			pitchInHz = -1.0f;
		}
	} else {
		pitchInHz = -1.0f;
	}
	
	return pitchInHz;
	
};

void PitchTracker::difference(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples, const int startSample) {
	/*
	 THE DIFFERENCE FUNCTION
	 @brief implements the difference function described in step 2 of the YIN paper, using an FFT to increase computational efficiency
	 @param 	inputBuffer			reference to audio input buffer
	 @param		inputChan			channel # to read audio from in input buffer
	 @param		inputBufferLength	input buffer length, in samples
	 */
	
	const float* reading = inputBuffer.getReadPointer(inputChan);
	
	const int fftOrder = log2(numSamples);
	
	dsp::FFT fft(fftOrder);
	
	dsp::Complex<float> fftBufferIn[numSamples];
	dsp::Complex<float> fftBufferOut[numSamples];
	dsp::Complex<float> kernelIn[numSamples];
	dsp::Complex<float> kernelOut[numSamples];
	dsp::Complex<float> yinStyleACFin[numSamples];
	dsp::Complex<float> yinStyleACFout[numSamples];
	
	// POWER TERM CALCULATION
	{
		powerTerms.clearQuick();
		for(int j = 0; j < yinBufferSize; ++j) { // first, calculate the first power term value...
			powerTerms.add(reading[startSample + j] * reading[startSample + j]);
		}
		// ... then iteratively calculate all others
		for(int i = 1; i < yinBufferSize; ++i) {
			powerTerms.add(powerTerms.getUnchecked(i - 1) - reading[startSample + i - 1] * reading[startSample + i - 1] + reading[startSample + i + yinBufferSize] * reading[startSample + i + yinBufferSize]);
		}
	}
	
	// YIN-STYLE AUTOCORRELATION VIA FFT
	{
		// 1. data
		for(int j = 0; j < numSamples; ++j) {
			fftBufferIn[j] = { reading[startSample + j], 0 };
		}
		fft.perform(fftBufferIn, fftBufferOut, false);
		
		// 2. half of the data, disguised as a convolution kernel
		for(int j = 0; j < yinBufferSize; ++j) {
			kernelIn[j] = { reading[startSample + yinBufferSize - j], 0 };
		}
		fft.perform(kernelIn, kernelOut, false);
		
		// 3. convolution via complex multiplication
		for (int j = 0; j < numSamples; ++j) {
			
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


void PitchTracker::cumulativeMeanNormalizedDifference() const {
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


int PitchTracker::absoluteThreshold() {
	/*
	 ABSOLUTE THRESHOLD
	 @brief implements step 4 of the YIN paper, an absolute threshold
	 @return 	tau value
	 */
	int tau;
	float probabilityEstimate;
	const float* read = yinBuffer.getReadPointer(0);
	
	// first two positions in yinBuffer are always 1
	for(tau = 2; tau < yinBufferSize; ++tau) { // limit range to yinBufferSize - 1 ?
		if(read[tau] < THRESHOLD) {
			while (tau + 1 < yinBufferSize && read[tau + 1] < read[tau]) {
				++tau;
			}
			probabilityEstimate = 1 - read[tau];
			break;
		}
	}
	
	// if no pitch is detected, tau = -1
	if(tau == yinBufferSize || read[tau] >= THRESHOLD || probabilityEstimate > 1.0f) {
		tau = -1;
		//isPitched = false;
	} else {
		//isPitched = true;
	}
	
	return tau;
};


float PitchTracker::parabolicInterpolation(int tauEstimate) const {
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





EpochFinder::EpochFinder()
{
	y.ensureStorageAllocated(MAX_BUFFERSIZE);
	y.clearQuick();
	y2.ensureStorageAllocated(MAX_BUFFERSIZE);
	y2.clearQuick();
	y3.ensureStorageAllocated(MAX_BUFFERSIZE);
	y3.clearQuick();
	epochs.ensureStorageAllocated(MAX_BUFFERSIZE);
	epochs.clearQuick();
};

EpochFinder::~EpochFinder()
{ };

Array<int> EpochFinder::extractEpochSampleIndices(AudioBuffer<float>& inputAudio, const int inputChan, const double samplerate)
{
	/*
	 EXTRACT EPOCH INDICES
	 
	 uses a ZFR approach to find sample index #s of epoch locations to an integer array
	 
	 @see : "Epoch Extraction From Speech Signals", by K. Sri Rama Murty and B. Yegnanarayana, 2008 : http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=6D94C490DA889017DE4362D322E1A23C?doi=10.1.1.586.7214&rep=rep1&type=pdf
	 
	 @see : example of ESOLA in C++ by Arjun Variar : http://www.github.com/viig99/esolafast/blob/master/src/esola.cpp
	 */
	
	const int numSamples = inputAudio.getNumSamples();
	
	
	const int window_length = round(numSamples * 0.1);  // ?? was originally based on samplerate... 
	
	epochs.clearQuick();
	y.clearQuick();
	y2.clearQuick();
	y3.clearQuick();
	
	float mean_val;
	float running_sum = 0.0f;
	
	const float* data = inputAudio.getReadPointer(inputChan);
	
	const float x0 = data[0];
	float y1_0 = x0;
	float y1_1 = data[1] - x0 + (2.0f * y1_0);
	float x_i;
	float y1_i;
	y2.add(y1_0);
	y2.add(y1_1 + (2.0f * y1_0));
	for(int i = 2; i < numSamples; ++i)
	{
		x_i = data[i] - data[i - 1];
		y1_i = x_i + (2.0f * y1_1) - y1_0;
		const float y2b1 = y2.getUnchecked(i - 1);
		const float y2b2 = y2.getUnchecked(i - 2);
		y2.add(y1_i + (2.0f * y2b1 - y2b2));
		y1_0 = y1_1;
		y1_1 = y1_i;
	}
	
	// third stage
	for(int i = 0; i < 2 * window_length + 2; ++i)
		running_sum += y2.getUnchecked(i);
	
	mean_val = 0.0f;
	for(int i = 0; i < numSamples; ++i) {
		if((i - window_length < 0) || (i + window_length >= numSamples)) {
			mean_val = y2.getUnchecked(i);
		} else if (i - window_length == 0) {
			mean_val = running_sum / (2.0f * window_length + 1.0f);
		} else {
			running_sum -= y2.getUnchecked(i - window_length - 1) - y2.getUnchecked(i + window_length);
			mean_val = running_sum / (2.0f * window_length + 1.0f);
		}
		y3.add(y2.getUnchecked(i) - mean_val);
	}
	
	// fourth stage
	running_sum = 0.0f;
	for(int i = 0; i < 2 * window_length + 2; ++i)
		running_sum += y3.getUnchecked(i);
	
	mean_val = 0.0f;
	for(int i = 0; i < numSamples; ++i) {
		if((i - window_length < 0) || (i + window_length >= numSamples)) {
			mean_val = y3.getUnchecked(i);
		} else if (i - window_length == 0) {
			mean_val = running_sum / (2.0f * window_length + 1.0f);
		} else {
			running_sum -= y3.getUnchecked(i - window_length - 1) - y3.getUnchecked(i + window_length);
			mean_val = running_sum / (2.0f * window_length + 1.0f);
		}
		y.add(y3.getUnchecked(i) - mean_val);
	}
	
	// last stage
	float last = y.getUnchecked(0);
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
