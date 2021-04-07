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
 
 psola_shifter.h :     This file defines a class that performs the resynthesis stage of the PSOLA process.
 
======================================================================================================================================================*/


namespace bav
{
    
#define bvh_NUM_SYNTHESIS_GRAINS 48  // these are cheap, no reason not to have a lot
    
    
template<typename SampleType>
class PsolaShifter
{
    using Analyzer = PsolaAnalyzer<SampleType>;
    using Synthesis_Grain = SynthesisGrain<SampleType>;
    
public:
    PsolaShifter (Analyzer* parentAnalyzer): analyzer(parentAnalyzer)
    {
        jassert (analyzer != nullptr);
    }
    
    
    void getSamples (SampleType* outputSamples, const int numSamples, const int newPeriod)
    {
        for (int i = 0; i < numSamples; ++i)
            outputSamples[i] = getNextSample (newPeriod);
    }
    
    
    SampleType getNextSample (const int newPeriod)
    {
        jassert (synthesisGrains.size() == bvh_NUM_SYNTHESIS_GRAINS);
        jassert (newPeriod > 0);
        
        if (! anyGrainsAreActive())
            startNewGrain (newPeriod);
        
        int grainsToStart = 0;
        
        auto sample = SampleType(0);
        
        for (auto* grain : synthesisGrains)
        {
            if (! grain->isActive())
            {
                grain->stop();
                continue;
            }
        
            sample += grain->getNextSample();
            
            if (grain->samplesLeft() == grain->halfwayIndex())
                ++grainsToStart;
        }
        
        for (int i = 0; i < grainsToStart; ++i)
            startNewGrain (newPeriod);
        
        if (nextSynthesisIndex > 0)
            --nextSynthesisIndex;
        
        return sample;
    }
    
    
    void prepare()
    {
        while (synthesisGrains.size() < bvh_NUM_SYNTHESIS_GRAINS)
            synthesisGrains.add (new Synthesis_Grain());
    }
    
    
    void reset()
    {
        nextSynthesisIndex = 0;
        
        for (auto* grain : synthesisGrains)
            grain->stop();
    }
    
    void releaseResources()
    {
        nextSynthesisIndex = 0;
        synthesisGrains.clear();
    }
    
    
private:
    
    inline void startNewGrain (const int newPeriod)
    {
        if (! anyGrainsAreActive())
            nextSynthesisIndex = 0;
        
        if (auto* newGrain = getAvailableGrain())
        {
            newGrain->startNewGrain (analyzer->findClosestGrain (nextSynthesisIndex), nextSynthesisIndex);
            nextSynthesisIndex += newPeriod;
        }
        
        jassert (anyGrainsAreActive());
    }
    
    inline bool anyGrainsAreActive() const
    {
        for (auto* grain : synthesisGrains)
            if (grain->isActive())
                return true;
        
        return false;
    }
    
    inline Synthesis_Grain* getAvailableGrain()
    {
        for (auto* grain : synthesisGrains)
            if (! grain->isActive())
                return grain;
        
        return nullptr;
    }
    
    
    
    Analyzer* analyzer;
    
    juce::OwnedArray<Synthesis_Grain> synthesisGrains;
    
    int nextSynthesisIndex = 0;
};


template class PsolaShifter<float>;
template class PsolaShifter<double>;
    
#undef bvh_NUM_SYNTHESIS_GRAINS
    
}  // namespace
