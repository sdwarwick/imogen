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
 
 psola_resynthesis.h:   This file defines AnalysisGrain and SynthesisGrain classes, which are used in the HarmonizerVoice's pitch shifting.
 
======================================================================================================================================================*/



namespace bav
{

/*------------------------------------------------------------------------------------------------------------------------------------------------------
 AnalysisGrain :    This class stores the actual audio samples that comprise a single audio grain, with a Hann window applied. The parent Harmonizer object owns a collection of these grains.
------------------------------------------------------------------------------------------------------------------------------------------------------*/

template<typename SampleType>
class AnalysisGrain
{
public:
    AnalysisGrain(): numActive(0), size(0), empty(true) { }
    
    void reserveSize (int numSamples) { samples.setSize(1, numSamples); }
    
    void incNumActive() noexcept { ++numActive; }
    
    void decNumActive() {
        --numActive;
        if (numActive == 0) clear();
    }
    
    void storeNewGrain (const SampleType* inputSamples, int startSample, int endSample)
    {
        empty = false;
        samples.clear();
        origStart = startSample;
        origEnd = endSample;
        size = endSample - startSample;
        jassert (size > 0);
        auto* writing = samples.getWritePointer(0);
        vecops::copy (inputSamples + startSample, writing, size);
        
        //  apply Hann window to input samples
        for (int s = 0; s < size; ++s)
            writing[s] *= getWindowValue (size, s);
    }
    
    SampleType getSample (int index) const
    {
        jassert (index < size);
        return samples.getSample (0, index);
    }
    
    int getSize() const noexcept { return size; }
    
    int getStartSample() const noexcept { return origStart; }
    
    int getEndSample() const noexcept { return origEnd; }
    
    bool isEmpty() const noexcept { return empty; }
    
    
private:
    inline SampleType getWindowValue (int windowSize, int index)
    {
        const auto cos2 = std::cos (static_cast<SampleType> (2 * index)
                                    * juce::MathConstants<SampleType>::pi / static_cast<SampleType> (windowSize - 1));
        
        return static_cast<SampleType> (0.5 - 0.5 * cos2);
    }
    
    void clear()
    {
        samples.clear();
        size = 0;
        empty = true;
        origStart = 0;
        origEnd = 0;
    }
    
    int numActive; // this counts the number of SynthesisGrains that are referring to this AnalysisGrain
    
    int origStart, origEnd;  // the original start & end sample indices of this grain
    
    int size;
    
    bool empty;
    
    juce::AudioBuffer<SampleType> samples;
};

    

/*------------------------------------------------------------------------------------------------------------------------------------------------------
SynthesisGrain :   This class holds a pointer to a specific AnalysisGrain, and its respacing information. Each HarmonizerVoice owns a colllection of these grains, and calls getNextSample() on these directly to generate its audio stream.
------------------------------------------------------------------------------------------------------------------------------------------------------*/

template<typename SampleType>
class SynthesisGrain
{
    using AnalysisGrain = AnalysisGrain<SampleType>;
    
public:
    SynthesisGrain(): active(false), readingIndex(0), grain(nullptr), zeroesLeft(0) { }
    
    bool isActive() const noexcept { return active; }
    
    void startNewGrain (AnalysisGrain* newGrain, int synthesisMarker)
    {
        jassert (newGrain != nullptr);
        active = true;
        grain = newGrain;
        newGrain->incNumActive();
        readingIndex = 0;
        zeroesLeft = synthesisMarker;
    }
    
    SampleType getNextSample()
    {
        if (zeroesLeft > 0)
        {
            jassert (readingIndex == 0);
            --zeroesLeft;
            return 0;
        }
        
        const auto sample = grain->getSample (readingIndex++);
        
        if (readingIndex >= grain->getSize())
            stop();
        
        return sample;
    }
    
    void skipSamples (int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            if (zeroesLeft > 0)
            {
                --zeroesLeft;
                continue;
            }
            
            ++readingIndex;
            
            if (readingIndex >= grain->getSize())
            {
                stop();
                return;
            }
        }
    }
    
    int samplesLeft() const
    {
        if (active)
            return grain->getSize() - readingIndex + std::max(0, zeroesLeft);
        
        return 0;
    }
    
    
private:
    bool active;
    int readingIndex;
    AnalysisGrain* grain;
    int zeroesLeft;
    
    void stop()
    {
        active = false;
        readingIndex = 0;
        zeroesLeft = 0;
        grain->decNumActive();
        grain = nullptr;
    }
};


}  // namespace
