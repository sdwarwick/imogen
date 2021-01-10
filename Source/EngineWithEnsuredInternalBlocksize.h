/*
  ==============================================================================

    EngineWithEnsuredInternalBlocksize.h
    Created: 10 Jan 2021 1:40:19am
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


template<typename SampleType>
class EngineWithEnsuredInternalBlocksize
{
    
public:
    
    void process (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages,
                  const bool applyFadeIn, const bool applyFadeOut) final;
    
    
private:
    
    void processWrapped (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output,
                         MidiBuffer& midiMessages,
                         const bool applyFadeIn, const bool applyFadeOut) final;
    
    
    virtual void renderBlock (const AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output) = 0;
    
};
