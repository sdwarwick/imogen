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
    
    
    void getSamples (SampleType* outputSamples, const int numSamples, const int newPeriod, const int origStartSample)
    {
        for (int i = 0; i < numSamples; ++i)
            outputSamples[i] = getNextSample (newPeriod, origStartSample);
    }
    
    
    SampleType getNextSample (const int newPeriod, const int bufferPos)
    {
        jassert (synthesisGrains.size() == bvh_NUM_SYNTHESIS_GRAINS);
        jassert (newPeriod > 0);
        
        if (! anyGrainsAreActive())
        {
//            nextSynthesisIndex = 0;
            startNewGrain (newPeriod, 0);
        }
        
        auto sample = SampleType(0);
        
        for (auto* grain : synthesisGrains)
        {
            if (! grain->isActive())
                continue;
            
            sample += grain->getNextSample();
            
            if (grain->samplesLeft() == grain->halfwayIndex())
                startNewGrain (newPeriod, bufferPos);
        }
        
        if (nextSynthesisIndex > 0)
            --nextSynthesisIndex;
        
        return sample;
    }
    
    
    void skipSamples (const int numSamples)
    {
        for (auto* grain : synthesisGrains)
            grain->stop();
        
        nextSynthesisIndex = std::max(0, nextSynthesisIndex - numSamples);
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
    
    inline void startNewGrain (const int newPeriod, const int idealBufferPos)
    {
        if (auto* newGrain = getAvailableGrain())
        {
            auto* analysisGrain = analyzer->findClosestGrain (idealBufferPos - nextSynthesisIndex);
            
            const auto scaledStart = anyGrainsAreActive()
                                   ? nextSynthesisIndex - ( analysisGrain->percentOfExpectedSize() * analysisGrain->getStartSample() )
                                   : 0;
            
            const auto samplesInFuture = juce::jlimit (0, nextSynthesisIndex, juce::roundToInt (scaledStart));
            
            newGrain->startNewGrain (analysisGrain, samplesInFuture);
            
            nextSynthesisIndex += newPeriod;
        }
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
