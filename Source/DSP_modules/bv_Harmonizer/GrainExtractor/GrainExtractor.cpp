/*
    Part of module: bv_Harmonizer
    Direct parent file: GrainExtractor.h
    Classes: GrainExtractor
 */


#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{
    
    
template<typename SampleType>
GrainExtractor<SampleType>::GrainExtractor()
{ };


template<typename SampleType>
GrainExtractor<SampleType>::~GrainExtractor()
{ };


template<typename SampleType>
void GrainExtractor<SampleType>::prepare (const int maxBlocksize)
{
    // maxBlocksize = max period of input audio
    
    peakIndices.ensureStorageAllocated (maxBlocksize);
    
    lastBlocksize = maxBlocksize;
};


template<typename SampleType>
void GrainExtractor<SampleType>::releaseResources()
{
    peakIndices.clear();
};


template<typename SampleType>
void GrainExtractor<SampleType>::getGrainOnsetIndices (Array<int>& targetArray,
                                                       const AudioBuffer<SampleType>& inputAudio,
                                                       const int period)
{
    targetArray.clearQuick();
    
    const int totalNumSamples = inputAudio.getNumSamples();
    const SampleType* reading = inputAudio.getReadPointer(0);
    
    // identify  peak indices for each pitch period & places them in the peakIndices array
    
    findPsolaPeaks (peakIndices, reading, totalNumSamples, period);
    
    jassert (! peakIndices.isEmpty());
    
    // create array of grain start indices, such that grains are 2 pitch periods long, CENTERED on points of synchronicity previously identified
    
    for (int peakIndex : peakIndices)
    {
        int grainStart = peakIndex - period; // offset the peak index by the period so that the peak index will be in the center of the grain (if grain is 2 periods long)
        
        if (grainStart >= 0)
            targetArray.add (grainStart);
        else
        {
            if (peakIndices.indexOf(peakIndex) < peakIndices.size() - 2) // we have more peaks coming, so we can disregard this one
                continue;
            
            grainStart += roundToInt (period / 2.0f);
            
            if (grainStart >= 0)
                targetArray.add (grainStart);
            else
                targetArray.add (peakIndex);
        }
    }
    
    jassert (! targetArray.isEmpty());
};



template class GrainExtractor<float>;
template class GrainExtractor<double>;


}; // namespace
