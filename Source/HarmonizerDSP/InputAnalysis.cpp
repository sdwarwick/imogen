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
};

PitchTracker::~PitchTracker()
{ };


float PitchTracker::getPitch(const AudioBuffer<float>& inputAudio, const double samplerate)
{
    if (inputAudio.getNumSamples() < MIN_SAMPLES_NEEDED)
        return prevDetectedPitch;
    
    const auto period = simpleYin(inputAudio);
    
    if(! (period > 0))
        return prevDetectedPitch;
    
    const auto detectedHz = samplerate / period;

    if(detectedHz >= minHz && detectedHz <= maxHz)
    {
        prevDetectedPitch = detectedHz;
        return detectedHz;
    }
    
    return prevDetectedPitch;
};


float PitchTracker::simpleYin(const AudioBuffer<float>& inputAudio) noexcept
{
    const int yinBufferSize = roundToInt(inputAudio.getNumSamples()/2);
    
    const auto* in      = inputAudio.getReadPointer (0);
          auto* yinData = yinBuffer .getWritePointer(0);
    
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
    auto tmp = data[0];
    for (j = 0; j < dataSize; j++)
    {
        pos = (tmp < data[j]) ? pos : j;
        tmp = (tmp < data[j]) ? tmp : data[j];
    }
    return pos;
};


float PitchTracker::quadraticPeakPosition (const float* data, unsigned int pos, const int dataSize) noexcept
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
    
    auto s0 = data[x0];
    auto s2 = data[x2];
    return pos + 0.5 * (s0 - s2 ) / (s0 - 2.* data[pos] + s2);
}

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

    const auto* data = inputAudio.getReadPointer(0);
    
    auto y1_0 = data[0];
    auto y1_1 = data[1] - y1_0 + (2.0f * y1_0);
    
    y2.add(y1_0);
    y2.add(y1_1 + (2.0f * y1_0));
    
    for(int i = 2; i < numSamples; ++i)
    {
        auto x_i = data[i] - data[i - 1];
        auto y1_i = x_i + (2.0f * y1_1) - y1_0;
        const auto y2b1 = y2.getUnchecked(i - 1);
        const auto y2b2 = y2.getUnchecked(i - 2);
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
            const auto rs1 = ((i - windowLength - 1) >= 0)  ? y2.getUnchecked(i - windowLength - 1) : 0;
            const auto rs2 = (i + windowLength < y2.size()) ? y2.getUnchecked(i + windowLength)     : 0;
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
            const auto rs1 = ((i - windowLength - 1) >= 0)  ? y3.getUnchecked(i - windowLength - 1) : 0;
            const auto rs2 = (i + windowLength < y3.size()) ? y3.getUnchecked(i + windowLength)     : 0;
            runningSum -= (rs1 - rs2);
            meanVal = runningSum / (2.0f * windowLength + 1.0f);
        }
        y.add(y3.getUnchecked(i) - meanVal);
    }
    
    // last stage
    auto last = y.getUnchecked(0);
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
