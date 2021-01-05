/*
 ==============================================================================
 
 InputAnalysis.cpp
 Created: 14 Dec 2020 6:32:56pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#include "InputAnalysis.h"


#define MIN_SAMPLES_NEEDED 32 // the minimum number of samples needed to calculate the pitch of a chunk


PitchTracker::PitchTracker(): prevDetectedPitch(0.0f), tolerence(0.15f), minHz(50.0f), maxHz(2000.0f)
{
    yinBuffer.setSize(1, MAX_BUFFERSIZE);
    //	powerTerms.ensureStorageAllocated(MAX_BUFFERSIZE);
    //	powerTerms.clearQuick();
    //	powerTerms.fill(0);
};

PitchTracker::~PitchTracker()
{ };


float PitchTracker::getPitch(AudioBuffer<float>& inputAudio, const double samplerate)
{
    if (inputAudio.getNumSamples() < MIN_SAMPLES_NEEDED)
        return prevDetectedPitch;
    
    float period = simpleYin(inputAudio);
    
    if(! (period > 0))
        return prevDetectedPitch;
    
    const float detectedHz = samplerate / period;

    if(detectedHz >= minHz && detectedHz <= maxHz)
    {
        prevDetectedPitch = detectedHz;
        return detectedHz;
    }
    
    return prevDetectedPitch;
};


float PitchTracker::simpleYin(AudioBuffer<float>& inputAudio) noexcept
{
    const int yinBufferSize = roundToInt(inputAudio.getNumSamples()/2);
    
    const float* in      = inputAudio.getReadPointer (0);
          float* yinData = yinBuffer .getWritePointer(0);
    
    float delta      = 0.0f;
    float runningSum = 0.0f;
    
    yinData[0] = 1.0f;
    for (int tau = 1; tau < yinBufferSize; ++tau)
    {
        yinData[tau] = 0.0;
        for (int j = 0; j < yinBufferSize; ++j)
        {
            delta = in[j] - in[j + tau];
            yinData[tau] += (delta * delta);
        }
        runningSum += yinData[tau];
        if (runningSum != 0)
            yinData[tau] *= tau / runningSum;
        else
            yinData[tau] = 1.0;
        
        int period = tau - 3;
        
        if (tau > 4 && (yinData[period] < tolerence) && (yinData[period] < yinData[period + 1]))
            return quadraticPeakPosition (yinBuffer.getReadPointer(0), period, yinBufferSize);
    }
    return quadraticPeakPosition (yinBuffer.getReadPointer(0), minElement(yinBuffer.getReadPointer(0), yinBufferSize), yinBufferSize);
};

unsigned int PitchTracker::minElement(const float* data, const int dataSize) noexcept
{
    unsigned int j, pos = 0;
    float tmp = data[0];
    for (j = 0; j < dataSize; j++)
    {
        pos = (tmp < data[j]) ? pos : j;
        tmp = (tmp < data[j]) ? tmp : data[j];
    }
    return pos;
};


float PitchTracker::quadraticPeakPosition (const float *data, unsigned int pos, const int dataSize) noexcept
{
    unsigned int x0, x2;
    
    if (pos == 0 || pos == dataSize - 1)
        return pos;
    
    x0 = (pos < 1) ? pos : pos - 1;
    x2 = (pos + 1 < dataSize) ? pos + 1 : pos;
    
    if (x0 == pos)
        return (data[pos] <= data[x2]) ? pos : x2;
    
    if (x2 == pos)
        return (data[pos] <= data[x0]) ? pos : x0;
    
    float s0 = data[x0];
    float s2 = data[x2];
    return pos + 0.5 * (s0 - s2 ) / (s0 - 2.* data[pos] + s2);
}

//
//float PitchTracker::findPitch(AudioBuffer<float>& inputAudio, const int inputChan, const double samplerate)
//{
///*	Implements the YIN pitch tracking algorithm, as described in "http://recherche.ircam.fr/equipes/pcm/cheveign/ps/2002_JASA_YIN_proof.pdf". This implementation uses an FFT to calculate the difference function in step 2 to increase performance and computational speed.
//
//	@see
//	: "YIN, a fundamental frequency estimator for speech and music", by Alain de Cheveigne ́ and Hideki Kawahara, 2002
//
//	@see
//	: YIN implementation in Java by Matthias Mauch, Joren Six and Paul Brossier : http://www.github.com/JorenSix/TarsosDSP/blob/master/src/core/be/tarsos/dsp/pitch/FastYin.java
//
//	@see
//	: YIN implementation in Python by Patrice Guyot : http://www.github.com/patriceguyot/Yin/blob/master/yin.py
// */
//
//	const int numSamples = inputAudio.getNumSamples();
//
//	if(numSamples < MIN_SAMPLES_NEEDED)
//		return prevDetectedPitch;
//
//	if(const int newbuffersize = round(numSamples/2); yinBufferSize != newbuffersize)
//	{
//		yinBufferSize = newbuffersize;
//		yinBuffer.setSize(1, newbuffersize, false, true, true);
//		if(powerTerms.size() != newbuffersize)
//			powerTerms.resize(newbuffersize);
//	}
//
//	int tauEstimate;
//	float pitchInHz;
//
//	difference(inputAudio, inputChan, numSamples);
//
//	cumulativeMeanNormalizedDifference();
//
//	tauEstimate = absoluteThreshold();
//
//	if(tauEstimate != -1) {
//		pitchInHz = samplerate / parabolicInterpolation(tauEstimate);
//		if(pitchInHz >= MIN_HZ && pitchInHz <= MAX_HZ) {
//		} else {
//			pitchInHz = -1.0f;
//		}
//	} else {
//		pitchInHz = -1.0f;
//	}
//
//	prevDetectedPitch = pitchInHz;
//	return pitchInHz;
//
//};
//
//void PitchTracker::difference(AudioBuffer<float>& inputBuffer, const int inputChan, const int numSamples) {
//	/*
//	 THE DIFFERENCE FUNCTION
//	 @brief implements the difference function described in step 2 of the YIN paper, using an FFT to increase computational efficiency
//	 @param 	inputBuffer			reference to audio input buffer
//	 @param		inputChan			channel # to read audio from in input buffer
//	 @param		inputBufferLength	input buffer length, in samples
//	 */
//
//	const float* in = inputBuffer.getReadPointer(inputChan);
//
//	//const int fftOrder = log2(numSamples);
//
//	dsp::FFT fft(log2(numSamples));
//
//	dsp::Complex<float> fftBufferIn[numSamples];
//	dsp::Complex<float> fftBufferOut[numSamples];
//	dsp::Complex<float> kernelIn[numSamples];
//	dsp::Complex<float> kernelOut[numSamples];
//	dsp::Complex<float> yinStyleACFin[numSamples];
//	dsp::Complex<float> yinStyleACFout[numSamples];
//
//
//	// POWER TERM CALCULATION
//	{
//		powerTerms.clearQuick();
//
//		powerTerms.add(0.0f);
//		for (int j = 0; j < yinBufferSize; ++j)
//			powerTerms.setUnchecked(0, powerTerms.getUnchecked(0) + in[j] * in[j]);
//
//		for(int tau = 1; tau < yinBufferSize; ++tau)
//			powerTerms.add(powerTerms.getUnchecked(tau-1) - in[tau-1] * in[tau-1] + in[tau + yinBufferSize] * in[tau+yinBufferSize]);
//	}
//
//	// YIN-STYLE AUTOCORRELATION VIA FFT
//	{
//		// 1. data
//		for(int j = 0; j < numSamples; ++j) {
//			fftBufferIn[j] = { in[j], 0 };
//		}
//		fft.perform(fftBufferIn, fftBufferOut, false);
//
//		// 2. half of the data, disguised as a convolution kernel
//		for(int j = 0; j < yinBufferSize; ++j) {
//			kernelIn[j] = { in[yinBufferSize - j], 0 };
//		}
//		fft.perform(kernelIn, kernelOut, false);
//
//		// 3. convolution via complex multiplication
//		for (int j = 0; j < numSamples; ++j)
//			yinStyleACFin[j] = { fftBufferOut[j].real() * kernelOut[j].real() - fftBufferOut[j+1].real() * kernelOut[j+1].real(), fftBufferOut[j].imag() * kernelOut[j].imag() + fftBufferOut[j+1].imag() * kernelOut[j+1].imag() };
//
//		fft.perform(yinStyleACFin, yinStyleACFout, true);
//	}
//
//	// CALCULATION OF DIFFERENCE FUNCTION
//	float* w = yinBuffer.getWritePointer(0);
//	for(int j = 0; j < yinBufferSize; ++j)
//		w[j] = powerTerms.getUnchecked(0) + powerTerms.getUnchecked(j) - 2 * yinStyleACFout[yinBufferSize - 1 + j].real() / fft.getSize();
//
//};
//
//
//
//void PitchTracker::fastDifference(AudioBuffer<float>& inputAudio, const int inputChan)
//{
//
////	float *audioTransformedComplex = new float[frameSize + 2];
////	float *audioOutReal = new float[frameSize];
////	float *kernel = new float[frameSize];
////	float *kernelTransformedComplex = new float[frameSize + 2];
////	float *yinStyleACFComplex = new float[frameSize + 2];
////	float *powerTerms = new float[yinBufferSize];
////
////	// YIN-STYLE AUTOCORRELATION via FFT
////	// 1. data
////	std::vector<float> real1 (audiofft::AudioFFT::ComplexSize(frameSize));
////	std::vector<float> imag1 (audiofft::AudioFFT::ComplexSize(frameSize));
////	m_fft.fft(in, real1.data(), imag1.data());
////	for (int j = 0; j < yinBufferSize + 2; ++j) {
////		audioTransformedComplex[j*2] = real1[j];
////		audioTransformedComplex[j*2+1] = imag1[j];
////	}
////
////	// 2. half of the data, disguised as a convolution kernel
////	for (int j = 0; j < yinBufferSize; ++j) {
////		kernel[j] = in[yinBufferSize-1-j];
////	}
////	for (int j = yinBufferSize; j < frameSize; ++j) {
////		kernel[j] = 0.;
////	}
////	//m_fft.forward(kernel, kernelTransformedComplex);
////	std::vector<float> real2 (audiofft::AudioFFT::ComplexSize(frameSize));
////	std::vector<float> imag2 (audiofft::AudioFFT::ComplexSize(frameSize));
////	m_fft.fft(kernel, real2.data(), imag2.data());
////	for (int j = 0; j < yinBufferSize + 2; ++j) {
////		kernelTransformedComplex[j*2] = real2[j];
////		kernelTransformedComplex[j*2+1] = imag2[j];
////	}
////
////	// 3. convolution via complex multiplication -- written into
////	for (int j = 0; j <= yinBufferSize; ++j) {
////		yinStyleACFComplex[j*2] = // real
////		audioTransformedComplex[j*2] * kernelTransformedComplex[j*2] -
////		audioTransformedComplex[j*2+1] * kernelTransformedComplex[j*2+1];
////		yinStyleACFComplex[j*2+1] = // imaginary
////		audioTransformedComplex[j*2] * kernelTransformedComplex[j*2+1] +
////		audioTransformedComplex[j*2+1] * kernelTransformedComplex[j*2];
////	}
////
////	//m_fft.inverse(yinStyleACFComplex, audioOutReal);
////	std::vector<float> real3 (audiofft::AudioFFT::ComplexSize(frameSize));
////	std::vector<float> imag3 (audiofft::AudioFFT::ComplexSize(frameSize));
////	for (int j = 0; j <= yinBufferSize; ++j) {
////		real3[j] = yinStyleACFComplex[j*2];
////		imag3[j] = yinStyleACFComplex[j*2+1];
////	}
////	m_fft.ifft(audioOutReal, real3.data(), imag3.data());
////
////	// CALCULATION OF difference function
////	// ... according to (7) in the Yin paper.
////	for (int j = 0; j < yinBufferSize; ++j) {
////		yinBuffer[j] = powerTerms[0] + powerTerms[j] - 2 *
////		audioOutReal[j+m_yinBufferSize-1];
////	}
////	delete [] audioTransformedComplex;
////	delete [] audioOutReal;
////	delete [] kernel;
////	delete [] kernelTransformedComplex;
////	delete [] yinStyleACFComplex;
////	delete [] powerTerms;
//
//};
//
//
//
//
//void PitchTracker::cumulativeMeanNormalizedDifference() const {
//	/*
//	 CUMULATIVE MEAN NORMALIZED DIFFERENCE FUNCTION
//	 @brief implements the cumulative mean normalized difference function described in step 3 of the YIN paper
//	 @code
//	 yinBuffer[0] == yinBuffer[1] == 1;
//	 @endcode
//	 */
//	int tau;
//	float* w = yinBuffer.getWritePointer(0);
//	const float* r = yinBuffer.getReadPointer(0);
//	w[0] = 1;
//	float runningSum = 0.0f;
//	for(tau = 1; tau < yinBufferSize; ++tau) {
//		runningSum += r[tau];
//		w[tau] = r[tau] * tau / runningSum;
//	}
//};
//
//
//int PitchTracker::absoluteThreshold() {
//	/*
//	 ABSOLUTE THRESHOLD
//	 @brief implements step 4 of the YIN paper, an absolute threshold
//	 @return 	tau value
//	 */
//	int tau;
//	float probabilityEstimate;
//	const float* read = yinBuffer.getReadPointer(0);
//
//	// first two positions in yinBuffer are always 1
//	for(tau = 2; tau < yinBufferSize; ++tau) { // limit range to yinBufferSize - 1 ?
//		if(read[tau] < THRESHOLD) {
//			while (tau + 1 < yinBufferSize && read[tau + 1] < read[tau]) {
//				++tau;
//			}
//			probabilityEstimate = 1 - read[tau];
//			break;
//		}
//	}
//
//	// if no pitch is detected, tau = -1
//	if(tau == yinBufferSize || read[tau] >= THRESHOLD || probabilityEstimate > 1.0f) {
//		tau = -1;
//		//isPitched = false;
//	} else {
//		//isPitched = true;
//	}
//
//	return tau;
//};
//
//
//float PitchTracker::parabolicInterpolation(int tauEstimate) const {
//	/*
//	 PARABOLIC INTERPOLATION
//	 @brief implements step 5 of the YIN paper, parabolic interpolation. This step refines the estimated tau value. This is needed to detect higher frequencies more precisely.
//	 @param 	tauEstimate		the estimated tau value returned from absoluteThreshold()
//	 @return	a more precise tau value, parabolically interpolated with local minima of d'(tau)
//	 */
//	float betterTau;
//	int x0;
//	int x2;
//
//	if(tauEstimate < 1) {
//		x0 = tauEstimate;
//	} else {
//		x0 = tauEstimate - 1;
//	}
//
//	if (tauEstimate + 1 < yinBufferSize) {
//		x2 = tauEstimate + 1;
//	} else {
//		x2 = tauEstimate;
//	}
//
//	if (x0 == tauEstimate) {
//		const float* r = yinBuffer.getReadPointer(0);
//		if(r[tauEstimate] <= r[x2]) {
//			betterTau = tauEstimate;
//		} else {
//			betterTau = x2;
//		}
//	} else if (x2 == tauEstimate) {
//		const float* r = yinBuffer.getReadPointer(0);
//		if(r[tauEstimate] <= r[x0]) {
//			betterTau = tauEstimate;
//		} else {
//			betterTau = x0;
//		}
//	} else {
//		const float* r = yinBuffer.getReadPointer(0);
//		float s0, s1, s2;
//		s0 = r[x0];
//		s1 = r[tauEstimate];
//		s2 = r[x2];
//		betterTau = tauEstimate + (s2 - s0) / (2 * (2 * s1 - s2 - s0));
//	}
//
//	return betterTau;
//};
//
//

#undef MIN_SAMPLES_NEEDED



EpochFinder::EpochFinder()
{
    y .ensureStorageAllocated(MAX_BUFFERSIZE);
    y2.ensureStorageAllocated(MAX_BUFFERSIZE);
    y3.ensureStorageAllocated(MAX_BUFFERSIZE);
    
    y .clearQuick();
    y2.clearQuick();
    y3.clearQuick();
};

EpochFinder::~EpochFinder()
{ };

void EpochFinder::increaseBufferSizes(const int newMaxBlocksize)
{
    y .ensureStorageAllocated(newMaxBlocksize);
    y2.ensureStorageAllocated(newMaxBlocksize);
    y3.ensureStorageAllocated(newMaxBlocksize);
};


void EpochFinder::extractEpochSampleIndices(const AudioBuffer<float>& inputAudio, const double samplerate, Array<int>& outputArray)
{
    /*
     EXTRACT EPOCH INDICES
     
     uses a ZFR approach to find sample index #s of epoch locations to an integer array
     
     @see : "Epoch Extraction From Speech Signals", by K. Sri Rama Murty and B. Yegnanarayana, 2008 : http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=6D94C490DA889017DE4362D322E1A23C?doi=10.1.1.586.7214&rep=rep1&type=pdf
     
     @see : example of ESOLA in C++ by Arjun Variar : http://www.github.com/viig99/esolafast/blob/master/src/esola.cpp
     */
    
    const int numSamples = inputAudio.getNumSamples();
    
    int windowLength = ceil( floor((0.0015 * samplerate) / numSamples) * (1 / numSamples) );
    
    if(windowLength > numSamples)
        windowLength = numSamples;
    
    outputArray.clearQuick();
    y .clearQuick();
    y2.clearQuick();
    y3.clearQuick();

    const float* data = inputAudio.getReadPointer(0);
    
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
    float runningSum = 0.0f;
    for(int i = 0; i < 2 * windowLength + 2; ++i)
        runningSum += ( (i < y2.size()) ? y2.getUnchecked(i) : 0 );
    
    float meanVal = 0.0f;
    for(int i = 0; i < numSamples; ++i)
    {
        if((i - windowLength < 0) || (i + windowLength >= numSamples))
            meanVal = y2.getUnchecked(i);
        else if (i - windowLength == 0)
            meanVal = runningSum / (2.0f * windowLength + 1.0f);
        else
        {
            float rs1 = ((i - windowLength - 1) >= 0)  ? y2.getUnchecked(i - windowLength - 1) : 0;
            float rs2 = (i + windowLength < y2.size()) ? y2.getUnchecked(i + windowLength)     : 0;
            runningSum -= (rs1 - rs2);
            meanVal = runningSum / (2.0f * windowLength + 1.0f);
        }
        y3.add(y2.getUnchecked(i) - meanVal);
    }
    
    // fourth stage
    runningSum = 0.0f;
    for(int i = 0; i < 2 * windowLength + 2; ++i)
        runningSum += ( (i < y3.size()) ? y3.getUnchecked(i) : 0 );
    
    meanVal = 0.0f;
    for(int i = 0; i < numSamples; ++i)
    {
        if((i - windowLength < 0) || (i + windowLength >= numSamples))
            meanVal = y3.getUnchecked(i);
        else if (i - windowLength == 0)
            meanVal = runningSum / (2.0f * windowLength + 1.0f);
        else
        {
            float rs1 = ((i - windowLength - 1) >= 0)  ? y3.getUnchecked(i - windowLength - 1) : 0;
            float rs2 = (i + windowLength < y3.size()) ? y3.getUnchecked(i + windowLength)     : 0;
            runningSum -= (rs1 - rs2);
            meanVal = runningSum / (2.0f * windowLength + 1.0f);
        }
        y.add(y3.getUnchecked(i) - meanVal);
    }
    
    // last stage
    float last = y.getUnchecked(0);
    float act;
    outputArray.add(0);
    for(int i = 0; i < numSamples; ++i)
    {
        act = y[i];
        if(last < 0 and act > 0)
            outputArray.add(i);
        last = act;
    }
    outputArray.add(numSamples - 1);
};


int EpochFinder::averageDistanceBetweenEpochs(const Array<int>& epochIndices)
{
    int averageDistance = 0;
    
    for (int i = 0; i < epochIndices.size() - 1; ++i)
    {
        const int distance = epochIndices.getUnchecked(i + 1) - epochIndices.getUnchecked(i);
        averageDistance = ceil((averageDistance + distance) / 2);
    }
    
    if (! (averageDistance > 0))
        return 1;
    
    return averageDistance;
};
