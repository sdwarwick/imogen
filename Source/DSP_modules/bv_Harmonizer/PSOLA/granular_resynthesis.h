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
 
 granular_resynthesis.h:   This file defines AnalysisGrain and SynthesisGrain classes, which are used in the PSOLA process.
 
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
    
    void decNumActive() noexcept { --numActive; }
    
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
    
    void clear()
    {
        samples.clear();
        size = 0;
        empty = true;
        origStart = 0;
        origEnd = 0;
        numActive = 0;
    }
    
    int numReferences() const noexcept { return numActive; }
    
    SampleType getSample (int index) const
    {
        jassert (index < size);
        return samples.getSample (0, index);
    }
    
    int getSize() const noexcept { return size; }
    
    int getStartSample() const noexcept { return origStart; }
    
    int getEndSample() const noexcept { return origEnd; }
    
    bool isEmpty() const noexcept { return empty || size == 0; }
    
    
private:
    inline SampleType getWindowValue (int windowSize, int index)
    {
        const auto cos2 = std::cos (static_cast<SampleType> (2 * index)
                                    * juce::MathConstants<SampleType>::pi / static_cast<SampleType> (windowSize - 1));
        
        return static_cast<SampleType> (0.5 - 0.5 * cos2);
    }
    
    int numActive; // this counts the number of SynthesisGrains that are referring to this AnalysisGrain
    
    int origStart, origEnd;  // the original start & end sample indices of this grain
    
    int size;
    
    bool empty;
    
    juce::AudioBuffer<SampleType> samples;
};

template class AnalysisGrain<float>;
template class AnalysisGrain<double>;
    

/*------------------------------------------------------------------------------------------------------------------------------------------------------
SynthesisGrain :   This class holds a pointer to a specific AnalysisGrain, and its respacing information. Each HarmonizerVoice owns a colllection of these grains, and calls getNextSample() on these directly to generate its audio stream.
------------------------------------------------------------------------------------------------------------------------------------------------------*/

template<typename SampleType>
class SynthesisGrain
{
    using Grain = AnalysisGrain<SampleType>;
    
public:
    SynthesisGrain(): active(false), readingIndex(0), grain(nullptr), zeroesLeft(0), halfIndex(0) { }
    
    bool isActive() const noexcept { return active; }
    
    int size() const noexcept { return grain->getSize(); }
    
    int halfwayIndex() const noexcept { return halfIndex; }
    
    void startNewGrain (Grain* newGrain, int synthesisMarker)
    {
        jassert (newGrain != nullptr);
        newGrain->incNumActive();
        grain = newGrain;
        
        active = true;
        readingIndex = 0;
        zeroesLeft = synthesisMarker;
        halfIndex = juce::roundToInt (grain->getSize() * 0.5f);
    }
    
    SampleType getNextSample()
    {
        jassert (active);
        
        if (zeroesLeft > 0)
        {
            jassert (readingIndex == 0);
            --zeroesLeft;
            return SampleType(0);
        }
        
        const auto sample = grain->getSample (readingIndex++);
        
        if (readingIndex >= grain->getSize())
            stop();
        
        return sample;
    }
    
    int samplesLeft() const
    {
        if (active)
            return grain->getSize() - readingIndex + std::max(0, zeroesLeft);
        
        return 0;
    }
    
    void stop()
    {
        jassert (active);
        active = false;
        readingIndex = 0;
        zeroesLeft = 0;
        grain->decNumActive();
        grain = nullptr;
        halfIndex = 0;
    }
    
    
private:
    bool active;
    int readingIndex;
    Grain* grain;
    int zeroesLeft;
    int halfIndex;
};
    
template class SynthesisGrain<float>;
template class SynthesisGrain<double>;


}  // namespace
