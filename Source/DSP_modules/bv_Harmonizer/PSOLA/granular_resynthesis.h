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
 AnalysisGrain :    This class stores the actual audio samples that comprise a single audio grain, with a Hann window applied.
------------------------------------------------------------------------------------------------------------------------------------------------------*/

template<typename SampleType>
class AnalysisGrain
{
public:
    AnalysisGrain(): numActive(0), origStart(0), origEnd(0), size(0), percentOfExpectedGrainSize(0) { }
    
    void reserveSize (int numSamples) { samples.setSize(1, numSamples); }
    
    void incNumActive() noexcept { ++numActive; }
    
    void decNumActive() noexcept
    {
        --numActive;
        
        if (numActive <= 0)
            clear();
    }
    
    float percentOfExpectedSize() const noexcept { return percentOfExpectedGrainSize; }
    
    void storeNewGrain (const SampleType* inputSamples, int startSample, int endSample, int expectedGrainSize)
    {
        samples.clear();
        origStart = startSample;
        origEnd = endSample;
        size = endSample - startSample + 1;
        jassert (size > 0);
        jassert (samples.getNumSamples() >= size);
        jassert (expectedGrainSize > 0);
        percentOfExpectedGrainSize = float(size) / float(expectedGrainSize);
        
        auto* writing = samples.getWritePointer(0);
        vecops::copy (inputSamples + startSample, writing, size);
        
        //  apply Hann window to input samples
        for (int s = 0; s < size; ++s)
            writing[s] *= getWindowValue (size, s);
    }
    
    void clear() noexcept
    {
        samples.clear();
        size = 0;
        origStart = 0;
        origEnd = 0;
        numActive = 0;
        percentOfExpectedGrainSize = 0.0f;
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
    
    bool isEmpty() const noexcept { return size < 1; }
    
    
private:
    inline SampleType getWindowValue (int windowSize, int index) const
    {
        const auto cos2 = std::cos (static_cast<SampleType> (2 * index)
                                    * juce::MathConstants<SampleType>::pi / static_cast<SampleType> (windowSize - 1));
        
        return static_cast<SampleType> (0.5 - 0.5 * cos2);
    }
    
    int numActive; // this counts the number of SynthesisGrains that are referring to this AnalysisGrain
    
    int origStart, origEnd;  // the original start & end sample indices of this grain
    
    int size;
    
    float percentOfExpectedGrainSize;
    
    juce::AudioBuffer<SampleType> samples;
};

template class AnalysisGrain<float>;
template class AnalysisGrain<double>;
    
    

/*------------------------------------------------------------------------------------------------------------------------------------------------------
SynthesisGrain :   This class holds a pointer to a specific AnalysisGrain, and its respacing information so it can be used to create a stream of repitched audio.
------------------------------------------------------------------------------------------------------------------------------------------------------*/

template<typename SampleType>
class SynthesisGrain
{
    using Grain = AnalysisGrain<SampleType>;
    
public:
    SynthesisGrain(): readingIndex(0), grain(nullptr), zeroesLeft(0) { }
    
    bool isActive() const noexcept
    {
        if (grain == nullptr)
            return false;
        
        return grain->getSize() > 0;
    }
    
    bool isHalfwayThrough() const noexcept { return samplesLeft() == juce::roundToInt(getSize() * 0.5f); }
    
    void startNewGrain (Grain* newGrain, int samplesInFuture)
    {
        jassert (newGrain != nullptr && ! newGrain->isEmpty());
        newGrain->incNumActive();
        grain = newGrain;
        
        readingIndex = 0;
        
        zeroesLeft = samplesInFuture;
    }
    
    SampleType getNextSample()
    {
        jassert (isActive() && ! grain->isEmpty());
        
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
    
    int samplesLeft() const noexcept
    {
        if (isActive())
            return grain->getSize() - readingIndex + std::max(0, zeroesLeft);
        
        return 0;
    }
    
    int getSize() const noexcept
    {
        if (grain != nullptr)
            return grain->getSize();
        
        return 0;
    }
    
    void stop() noexcept
    {
        readingIndex = 0;
        zeroesLeft = 0;
        halfIndex = 0;
        
        if (grain != nullptr)
        {
            grain->decNumActive();
            grain = nullptr;
        }
    }
    
    
private:
    int readingIndex;  // the next index to be read from the AnalysisGrain's buffer
    Grain* grain;
    int zeroesLeft;  // the number of zeroes this grain will output before actually outputting its samples. This allows grains to be respaced into the future.
    int halfIndex;  // marks the halfway point for this grain
};
    
template class SynthesisGrain<float>;
template class SynthesisGrain<double>;


}  // namespace
