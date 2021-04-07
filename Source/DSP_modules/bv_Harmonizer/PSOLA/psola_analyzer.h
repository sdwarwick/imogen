/*======================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 psola_analyzer.h :     This file defines a class that performs the analysis stage of the PSOLA process.
 
======================================================================================================================================================*/


#include "GrainExtractor/GrainExtractor.h"
#include <climits>  // for INT_MAX


#define bvh_NUM_ANALYSIS_GRAINS 48


namespace bav
{
    
template<typename SampleType>
class PsolaAnalyzer
{
    using Analysis_Grain = AnalysisGrain<SampleType>;
    
    
public:
    PsolaAnalyzer() { }
    
    
    void prepare (int blocksize)
    {
        while (analysisGrains.size() < bvh_NUM_ANALYSIS_GRAINS)
            analysisGrains.add (new Analysis_Grain());
        
        for (auto* grain : analysisGrains)
            grain->reserveSize (blocksize);
        
        indicesOfGrainOnsets.ensureStorageAllocated (blocksize);
        grainExtractor.prepare (blocksize);
    }
    
    
    void reset()
    {
        for (auto* grain : analysisGrains)
            grain->clear();
    }
    
    
    void analyzeInput (const SampleType* inputSamples, const int numSamples, const int periodThisFrame)
    {
        jassert (analysisGrains.size() == bvh_NUM_ANALYSIS_GRAINS);
        jassert (periodThisFrame > 0 && numSamples >= periodThisFrame * 2);
        
        grainExtractor.getGrainOnsetIndices (indicesOfGrainOnsets, inputSamples, numSamples, periodThisFrame);
        
        const auto grainSize = periodThisFrame * 2;
        
        jassert (! indicesOfGrainOnsets.isEmpty());
        jassert (indicesOfGrainOnsets.getLast() + grainSize <= numSamples);
        
        jassert (getEmptyGrain() != nullptr);  // there should be at least 1 grain slot available each analysis frame
        
        for (int index : indicesOfGrainOnsets)  //  write to analysis grains...
        {
            auto* grain = getEmptyGrain();
            jassert (grain != nullptr);
            grain->storeNewGrain (inputSamples, index, index + grainSize);
        }
        
        if (indicesOfGrainOnsets.getUnchecked(0) > 0)  // bit @ beginning
            if (auto* grain = getEmptyGrain())
                grain->storeNewGrain (inputSamples, 0, indicesOfGrainOnsets.getUnchecked(0));
        
        if (indicesOfGrainOnsets.getLast() + grainSize < numSamples)  // bit @ end
            if (auto* grain = getEmptyGrain())
                grain->storeNewGrain (inputSamples, indicesOfGrainOnsets.getLast(), numSamples);
    }
    
    
    Analysis_Grain* findClosestGrain (int synthesisMarker) const
    {
        Analysis_Grain* closestGrain = nullptr;
        int distance = INT_MAX;
        
        for (auto* grain : analysisGrains)
        {
            if (grain->isEmpty())
                continue;
            
            const auto newDist = abs(synthesisMarker - grain->getStartSample());
            
            if (closestGrain == nullptr || newDist < distance)
            {
                closestGrain = grain;
                distance = newDist;
            }
        }
        
        jassert (closestGrain != nullptr);
        
        return closestGrain;
    }
    
    
    void clearUnusedGrains()
    {
        for (auto* grain : analysisGrains)
            if (grain->numReferences() <= 0)
                grain->clear();
    }
    
    
    void releaseResources()
    {
        indicesOfGrainOnsets.clear();
        grainExtractor.releaseResources();
        analysisGrains.clear();
    }
    
    
    
private:
    
    inline Analysis_Grain* getEmptyGrain() const
    {
        for (auto* grain : analysisGrains)
            if (grain->isEmpty() || grain->numReferences() <= 0)
                return grain;
        
        return nullptr;
    }
    
    
    GrainExtractor<SampleType> grainExtractor;
    juce::Array<int> indicesOfGrainOnsets;
    
    juce::OwnedArray<Analysis_Grain> analysisGrains;
};


template class PsolaAnalyzer<float>;
template class PsolaAnalyzer<double>;
    

#undef bvh_NUM_ANALYSIS_GRAINS
    
}  // namespace
