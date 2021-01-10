/*
  ==============================================================================

    ImogenEngine.cpp
    Created: 6 Jan 2021 12:30:40am
    Author:  Ben Vining

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "GlobalDefinitions.h"

template<typename SampleType>
ImogenEngine<SampleType>::ImogenEngine(ImogenAudioProcessor& p):
    processor(p),
    resourcesReleased(true), initialized(false)
{
    dryPanningMults[0] = 0.5f;
    dryPanningMults[1] = 0.5f;
    prevDryPanningMults[0] = 0.5f;
    prevDryPanningMults[1] = 0.5f;
    
    inputGain = 1.0f;
    prevInputGain = 1.0f;
    
    outputGain = 1.0f;
    prevOutputGain = 1.0f;
    
    dryGain = 1.0f;
    prevDryGain = 1.0f;
    
    wetGain = 1.0f;
    prevWetGain = 1.0f;
};


template<typename SampleType>
ImogenEngine<SampleType>::~ImogenEngine()
{ };


template<typename SampleType>
void ImogenEngine<SampleType>::initialize (const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices)
{
    for (int i = 0; i < initNumVoices; ++i)
        harmonizer.addVoice (new HarmonizerVoice<SampleType>(&harmonizer));
    
    harmonizer.newMaxNumVoices (std::max (initNumVoices, MAX_POSSIBLE_NUMBER_OF_VOICES));
    
    prevOutputGain = outputGain;
    prevInputGain  = inputGain;
    
    prevDryPanningMults[0] = dryPanningMults[0];
    prevDryPanningMults[1] = dryPanningMults[1];

    processor.setLatencySamples (internalBlocksize);
    
    prepare (initSamplerate, std::max (initSamplesPerBlock, MAX_BUFFERSIZE));
    
    numStoredInputSamples  = 0;
    numStoredOutputSamples = 0;
    
    inBuffer .setSize (1, internalBlocksize, true, true, true);
    dryBuffer.setSize (2, internalBlocksize, true, true, true);
    wetBuffer.setSize (2, internalBlocksize, true, true, true);
    
    initialized = true;
};


template<typename SampleType>
void ImogenEngine<SampleType>::prepare (double sampleRate, int samplesPerBlock)
{
    if(harmonizer.getSamplerate() != sampleRate)
        harmonizer.setCurrentPlaybackSampleRate(sampleRate);
    
    const int aggregateBufferSizes = std::max (internalBlocksize * 2, samplesPerBlock);
    
    inputCollectionBuffer .setSize (1, aggregateBufferSizes, true, true, true);
    inputInterimBuffer    .setSize (1, aggregateBufferSizes, true, true, true);
    outputCollectionBuffer.setSize (2, aggregateBufferSizes, true, true, true);
    outputInterimBuffer   .setSize (2, aggregateBufferSizes, true, true, true);
    
    midiChoppingBuffer.ensureSize (aggregateBufferSizes);
    
    dspSpec.sampleRate = sampleRate;
    dspSpec.maximumBlockSize = internalBlocksize;
    dspSpec.numChannels = 2;
    
    limiter.prepare (dspSpec);
    
    dryWetMixer.prepare (dspSpec);
    dryWetMixer.setWetLatency(0); // latency in samples of the ESOLA algorithm
    
    harmonizer.resetNoteOnCounter(); // ??
    harmonizer.prepare (aggregateBufferSizes);
    
    clearBuffers();
    
    resourcesReleased = false;
};


template<typename SampleType>
void ImogenEngine<SampleType>::process (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output,
                                        MidiBuffer& midiMessages,
                                        const bool applyFadeIn, const bool applyFadeOut)
{
    // at this layer, we check to ensure that the buffer sent to us does not exceed the internal blocksize we want to process. If it does, it is broken into smaller chunks, and processWrapped() called on each of these chunks in sequence.
    // at this level, our only garuntee is that we will not recieve an empty buffer (ie, 0 samples long)
    
    const int totalNumSamples = inBus.getNumSamples();
    
    jassert (totalNumSamples > 0);
    
    if (totalNumSamples <= internalBlocksize)
    {
        processWrapped (inBus, output, midiMessages, applyFadeIn, applyFadeOut);
        return;
    }
    
    int samplesLeft = totalNumSamples;
    int startSample = 0;
    
    bool actuallyFadingIn  = applyFadeIn;
    bool actuallyFadingOut = applyFadeOut;
    
    while (samplesLeft > 0)
    {
        const int chunkNumSamples = std::min (internalBlocksize, samplesLeft);
        
        AudioBuffer<SampleType> inBusProxy  (inBus.getArrayOfWritePointers(),  inBus.getNumChannels(), startSample, chunkNumSamples);
        AudioBuffer<SampleType> outputProxy (output.getArrayOfWritePointers(), 2,                      startSample, chunkNumSamples);
        
        copyRangeOfMidiBuffer (midiMessages, midiChoppingBuffer, startSample, 0, chunkNumSamples);
        
        processWrapped (inBusProxy, outputProxy, midiChoppingBuffer, actuallyFadingIn, actuallyFadingOut);
        
        copyRangeOfMidiBuffer (midiChoppingBuffer, midiMessages, 0, startSample, chunkNumSamples);
        
        startSample += chunkNumSamples;
        samplesLeft -= chunkNumSamples;
        
        if ( (numStoredInputSamples + chunkNumSamples) >= internalBlocksize )
        {
            actuallyFadingIn  = false;
            actuallyFadingOut = false;
        }
    }
};


template<typename SampleType>
void ImogenEngine<SampleType>::processWrapped (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output,
                                               MidiBuffer& midiMessages,
                                               const bool applyFadeIn, const bool applyFadeOut)
{
    // at this level, the buffer block sizes sent to us are garunteed to NEVER exceed the declared internalBlocksize, but they may still be SMALLER than this blocksize -- the individual buffers this function recieves may be as short as 1 sample long.
    
    const int numNewSamples = inBus.getNumSamples();
    
    jassert (numNewSamples <= internalBlocksize);
    
    AudioBuffer<SampleType> input; // input needs to be a MONO buffer!
    
    switch (processor.getModulatorSource()) // isolate a mono input buffer from the input bus, mixing to mono if necessary
    {
        case (ImogenAudioProcessor::ModulatorInputSource::left):
            input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers(), 1, numNewSamples);
            break;
            
        case (ImogenAudioProcessor::ModulatorInputSource::right):
            input = AudioBuffer<SampleType> ( (inBus.getArrayOfWritePointers() + (inBus.getNumChannels() > 1)), 1, numNewSamples);
            break;
            
        case (ImogenAudioProcessor::ModulatorInputSource::mixToMono):
        {
            const int totalNumChannels = inBus.getNumChannels();
            
            if (totalNumChannels == 1)
            {
                input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers(), 1, numNewSamples);
                break;
            }
            
            inBuffer.copyFrom (0, 0, inBus, 0, 0, numNewSamples);
            
            for (int channel = 1; channel < totalNumChannels; ++channel)
                inBuffer.addFrom (0, 0, inBus, channel, 0, numNewSamples);
            
            inBuffer.applyGain (1.0f / totalNumChannels);
            
            input = AudioBuffer<SampleType> (inBuffer.getArrayOfWritePointers(), 1, numNewSamples);
            break;
        }
    }
    
    // copy new input samples recieved this frame into the back of the inputCollectionBuffer queue
    inputCollectionBuffer.copyFrom (0, numStoredInputSamples, input, 0, 0, numNewSamples);
    numStoredInputSamples += numNewSamples;
    
    if (numStoredInputSamples < internalBlocksize) // not calling renderBlock() this time, not enough samples to process!
    {
        // midi for the too-small block...
        if (auto midiIterator = midiMessages.findNextSamplePosition(0);
            midiIterator != midiMessages.cend())
        {
            harmonizer.clearMidiBuffer();
            
            std::for_each (midiIterator,
                           midiMessages.cend(),
                           [&] (const MidiMessageMetadata& meta)
                               { harmonizer.handleMidiEvent (meta.getMessage(), meta.samplePosition); } );
            
            midiMessages.swapWith (harmonizer.returnMidiBuffer());
        }
        
        if (numStoredOutputSamples == 0)
        {
            output.clear();
            return;
        }
        
        const int outputSamplesUsing = std::min (numStoredOutputSamples, numNewSamples);
        
        for (int chan = 0; chan < 2; ++chan)
            output.copyFrom (chan, 0, outputCollectionBuffer, chan, 0, outputSamplesUsing);
        
        usedOutputSamples (outputSamplesUsing);
    
        return;
    }
    
    // alias buffer referring to just the chunk of the inputCollectionBuffer that we'll be reading from
    AudioBuffer<SampleType> thisChunksInput  (inputCollectionBuffer.getArrayOfWritePointers(), 1, 0, internalBlocksize);
    
    // alias buffer referring to just the chunk of the outputCollectionBuffer that we'll be writing to
    AudioBuffer<SampleType> thisChunksOutput (outputCollectionBuffer.getArrayOfWritePointers(), 2, numStoredOutputSamples, internalBlocksize);
    
    // appends the next rendered block of samples to the end of the outputCollectionBuffer
    renderBlock (thisChunksInput, thisChunksOutput, midiMessages, applyFadeIn, applyFadeOut);
    
    numStoredOutputSamples += internalBlocksize;
    usedInputSamples (internalBlocksize);
    
    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, outputCollectionBuffer, chan, 0, numNewSamples);
    
    usedOutputSamples (numNewSamples);
};



template<typename SampleType>
void ImogenEngine<SampleType>::renderBlock (const AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output,
                                            MidiBuffer& midiMessages,
                                            const bool applyFadeIn, const bool applyFadeOut)
{
    // at this stage, the blocksize is garunteed to ALWAYS be the declared internalBlocksize.
    
    jassert (input .getNumSamples() == internalBlocksize);
    jassert (output.getNumSamples() == internalBlocksize);
    jassert (input .getNumChannels() == 1);
    jassert (output.getNumChannels() == 2);
    
    // first, deal with MIDI for this block.
    if (auto midiIterator = midiMessages.findNextSamplePosition(0);
        midiIterator != midiMessages.cend())
    {
        harmonizer.clearMidiBuffer();
        
        std::for_each (midiIterator,
                       midiMessages.cend(),
                       [&] (const MidiMessageMetadata& meta)
                           { harmonizer.handleMidiEvent (meta.getMessage(), meta.samplePosition); } );
        
        midiMessages.swapWith (harmonizer.returnMidiBuffer());
    }
    
    // master input gain
    inBuffer.copyFromWithRamp (0, 0, input.getReadPointer(0), internalBlocksize, prevInputGain, inputGain);
    prevInputGain = inputGain;
    
    harmonizer.analyzeInput (inBuffer);
    
    // write to dry buffer & apply panning (w/ panning multipliers ramped)
    for (int chan = 0; chan < 2; ++chan)
    {
        dryBuffer.copyFromWithRamp (chan, 0, inBuffer.getReadPointer(0), internalBlocksize, prevDryPanningMults[chan], dryPanningMults[chan]);
        prevDryPanningMults[chan] = dryPanningMults[chan];
    }
    
    // dry gain
    dryBuffer.applyGainRamp (0, internalBlocksize, prevDryGain, dryGain);
    prevDryGain = dryGain;
    
    dryWetMixer.pushDrySamples ( dsp::AudioBlock<SampleType>(dryBuffer) );
    
    harmonizer.renderVoices (inBuffer, wetBuffer); // puts the harmonizer's rendered stereo output into wetProxy (= "wetBuffer")
    
    // wet gain
    wetBuffer.applyGainRamp (0, internalBlocksize, prevWetGain, wetGain);
    prevWetGain = wetGain;
    
    dryWetMixer.mixWetSamples ( dsp::AudioBlock<SampleType>(wetBuffer) ); // puts the mixed dry & wet samples into "wetProxy" (= "wetBuffer")
    
    output.makeCopyOf (wetBuffer, true); // transfer from wetBuffer to output buffer
    
    // master output gain
    output.applyGainRamp (0, internalBlocksize, prevOutputGain, outputGain);
    prevOutputGain = outputGain;
    
    if (processor.isLimiterOn())
    {
        dsp::AudioBlock<SampleType> limiterBlock (output);
        limiter.process (dsp::ProcessContextReplacing<SampleType>(limiterBlock));
    }
    
    if (applyFadeIn)
        output.applyGainRamp (0, internalBlocksize, 0.0f, 1.0f);
    
    if (applyFadeOut)
        output.applyGainRamp (0, internalBlocksize, 1.0f, 0.0f);
};



template<typename SampleType>
void ImogenEngine<SampleType>::processBypassed (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output)
{
    const int totalNumSamples = inBus.getNumSamples();
    
    jassert (totalNumSamples > 0);
    
    if (totalNumSamples <= internalBlocksize)
    {
        processBypassedWrapped (inBus, output);
        return;
    }
    
    int samplesLeft = totalNumSamples;
    int startSample = 0;
    
    while (samplesLeft > 0)
    {
        const int chunkNumSamples = std::min (internalBlocksize, samplesLeft);
        
        AudioBuffer<SampleType> inBusProxy  (inBus.getArrayOfWritePointers(),  inBus.getNumChannels(), startSample, chunkNumSamples);
        AudioBuffer<SampleType> outputProxy (output.getArrayOfWritePointers(), 2,                      startSample, chunkNumSamples);
        
        processBypassedWrapped (inBusProxy, outputProxy);
        
        startSample += chunkNumSamples;
        samplesLeft -= chunkNumSamples;
    }
};


template<typename SampleType>
void ImogenEngine<SampleType>::processBypassedWrapped (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output)
{
    const int numNewSamples = inBus.getNumSamples();
    
    jassert (numNewSamples <= internalBlocksize);
    
    AudioBuffer<SampleType> input; // input needs to be a MONO buffer!
    
    const int totalNumChannels = inBus.getNumChannels();
    
    if (totalNumChannels == 1)
        input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers(), 1, numNewSamples);
    else
    {
        inBuffer.copyFrom (0, 0, inBus, 0, 0, numNewSamples);
        
        for (int channel = 1; channel < totalNumChannels; ++channel)
            inBuffer.addFrom (0, 0, inBus, channel, 0, numNewSamples);
        
        inBuffer.applyGain (1.0f / totalNumChannels);
        
        input = AudioBuffer<SampleType> (inBuffer.getArrayOfWritePointers(), 1, numNewSamples);
    }
    
    // copy new input samples recieved this frame into the back of the inputCollectionBuffer queue
    inputCollectionBuffer.copyFrom (0, numStoredInputSamples, input, 0, 0, numNewSamples);
    numStoredInputSamples += numNewSamples;
    
    if (numStoredInputSamples < internalBlocksize) // not calling renderBlock() this time, not enough samples to process!
    {
        if (numStoredOutputSamples == 0)
        {
            output.clear();
            return;
        }
        
        const int outputSamplesUsing = std::min (numStoredOutputSamples, numNewSamples);
        
        for (int chan = 0; chan < 2; ++chan)
            output.copyFrom (chan, 0, outputCollectionBuffer, chan, 0, outputSamplesUsing);
        
        usedOutputSamples (outputSamplesUsing);
        
        return;
    }
    
    // alias buffer referring to just the chunk of the inputCollectionBuffer that we'll be reading from
    AudioBuffer<SampleType> thisChunksInput  (inputCollectionBuffer.getArrayOfWritePointers(), 1, 0, internalBlocksize);
    
    // alias buffer referring to just the chunk of the outputCollectionBuffer that we'll be writing to
    AudioBuffer<SampleType> thisChunksOutput (outputCollectionBuffer.getArrayOfWritePointers(), 2, numStoredOutputSamples, internalBlocksize);
    
    // appends the next rendered block of samples to the end of the outputCollectionBuffer
    for (int chan = 0; chan < 2; ++chan)
        thisChunksOutput.copyFrom(chan, 0, thisChunksInput, 0, 0, internalBlocksize);
    
    numStoredOutputSamples += internalBlocksize;
    usedInputSamples (internalBlocksize);
    
    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, outputCollectionBuffer, chan, 0, numNewSamples);
    
    usedOutputSamples (numNewSamples);
};



template<typename SampleType>
void ImogenEngine<SampleType>::usedOutputSamples (const int numSamplesUsed)
{
    jassert (numStoredOutputSamples >= numSamplesUsed);
    
    numStoredOutputSamples -= numSamplesUsed;
    
    if (numStoredOutputSamples == 0)
        return;
    
    for (int chan = 0; chan < 2; ++chan)
    {
        outputInterimBuffer   .copyFrom (chan, 0, outputCollectionBuffer, chan, numSamplesUsed, numStoredOutputSamples);
        outputCollectionBuffer.copyFrom (chan, 0, outputInterimBuffer,    chan, 0,              numStoredOutputSamples);
    }
};


template<typename SampleType>
void ImogenEngine<SampleType>::usedInputSamples (const int numSamplesUsed)
{
    jassert (numStoredInputSamples >= numSamplesUsed);
    
    numStoredInputSamples -= numSamplesUsed;
    
    if (numStoredInputSamples == 0)
        return;
    
    for (int chan = 0; chan < 2; ++chan)
    {
        inputInterimBuffer   .copyFrom (chan, 0, inputCollectionBuffer, chan, numSamplesUsed, numStoredInputSamples);
        inputCollectionBuffer.copyFrom (chan, 0, inputInterimBuffer,    chan, 0,              numStoredInputSamples);
    }
};



template<typename SampleType>
void ImogenEngine<SampleType>::copyRangeOfMidiBuffer (const MidiBuffer& inputBuffer, MidiBuffer& outputBuffer,
                                                      const int startSampleOfInput,
                                                      const int startSampleOfOutput,
                                                      const int numSamples)
{
    outputBuffer.clear (startSampleOfOutput, numSamples);
    
    auto midiIterator = inputBuffer.findNextSamplePosition(startSampleOfInput);
    
    if (midiIterator == inputBuffer.cend())
        return;
    
    auto midiEnd = ++(inputBuffer.findNextSamplePosition(startSampleOfInput + numSamples - 1));
    
    if (midiIterator == midiEnd)
        return;
    
    if (midiEnd == ++inputBuffer.cend())
        midiEnd = inputBuffer.cend();
    
    const int sampleOffset = startSampleOfOutput - startSampleOfInput;
    
    std::for_each (midiIterator, midiEnd,
                   [&] (const MidiMessageMetadata& meta)
                       { outputBuffer.addEvent (meta.getMessage(), meta.samplePosition + sampleOffset); } );
};



template<typename SampleType>
void ImogenEngine<SampleType>::clearBuffers()
{
    harmonizer.clearBuffers();
    harmonizer.clearMidiBuffer();
    wetBuffer.clear();
    dryBuffer.clear();
    inBuffer .clear();
    midiChoppingBuffer.clear();
};


template<typename SampleType>
void ImogenEngine<SampleType>::reset()
{
    harmonizer.allNotesOff(false);
    
    releaseResources();
    
    prevDryPanningMults[0] = 0.5f;
    prevDryPanningMults[1] = 0.5f;
    dryPanningMults[0] = 0.5f;
    dryPanningMults[1] = 0.5f;
    
    prevInputGain  = inputGain;
    prevOutputGain = outputGain;
    
    prevDryGain = dryGain;
    prevWetGain = wetGain;
    
    numStoredInputSamples  = 0;
    numStoredOutputSamples = 0;
};


template<typename SampleType>
void ImogenEngine<SampleType>::releaseResources()
{
    harmonizer.releaseResources();
    harmonizer.resetNoteOnCounter();
    
    wetBuffer.setSize(0, 0, false, false, false);
    dryBuffer.setSize(0, 0, false, false, false);
    inBuffer .setSize(0, 0, false, false, false);
    
    clearBuffers();
    
    dryWetMixer.reset();
    limiter.reset();
    
    resourcesReleased = true;
    initialized       = false;
};


// functions for updating parameters ----------------------------------------------------------------------------------------------------------------

template<typename SampleType>
void ImogenEngine<SampleType>::updateNumVoices(const int newNumVoices)
{
    const int currentVoices = harmonizer.getNumVoices();
    
    if (currentVoices == newNumVoices)
        return;
    
    if(newNumVoices > currentVoices)
    {
        processor.suspendProcessing (true);
        
        for(int i = 0; i < newNumVoices - currentVoices; ++i)
            harmonizer.addVoice(new HarmonizerVoice<SampleType>(&harmonizer));
        
        harmonizer.newMaxNumVoices(newNumVoices); // increases storage overheads for internal harmonizer functions dealing with arrays of notes
        
        processor.suspendProcessing (false);
    }
    else
        harmonizer.removeNumVoices(currentVoices - newNumVoices);
};


template<typename SampleType>
void ImogenEngine<SampleType>::updateDryVoxPan  (const int newMidiPan)
{
    prevDryPanningMults[0] = dryPanningMults[0];
    prevDryPanningMults[1] = dryPanningMults[1];
    const float Rpan = newMidiPan / 127.0f;
    dryPanningMults[1] = Rpan;
    dryPanningMults[0] = 1.0f - Rpan;
};


template<typename SampleType>
void ImogenEngine<SampleType>::updateInputGain (const float newInGain)
{
    prevInputGain = inputGain;
    inputGain = newInGain;
};


template<typename SampleType>
void ImogenEngine<SampleType>::updateOutputGain (const float newOutGain)
{
    prevOutputGain = outputGain;
    outputGain = newOutGain;
};


template<typename SampleType>
void ImogenEngine<SampleType>::updateDryGain (const float newDryGain)
{
    prevDryGain = dryGain;
    dryGain = newDryGain;
};


template<typename SampleType>
void ImogenEngine<SampleType>::updateWetGain (const float newWetGain)
{
    prevWetGain = wetGain;
    wetGain = newWetGain;
};



template<typename SampleType>
void ImogenEngine<SampleType>::updateDryWet(const float newWetMixProportion)
{
    dryWetMixer.setWetMixProportion(newWetMixProportion / 100.0f);
    
    // need to set latency!!!
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateAdsr(const float attack, const float decay, const float sustain, const float release, const bool isOn)
{
    harmonizer.updateADSRsettings(attack, decay, sustain, release);
    harmonizer.setADSRonOff(isOn);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateQuickKill(const int newMs)
{
    harmonizer.updateQuickReleaseMs(newMs);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateQuickAttack(const int newMs)
{
    harmonizer.updateQuickAttackMs(newMs);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateStereoWidth(const int newStereoWidth, const int lowestPannedNote)
{
    harmonizer.updateLowestPannedNote(lowestPannedNote);
    harmonizer.updateStereoWidth     (newStereoWidth);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateMidiVelocitySensitivity(const int newSensitivity)
{
    harmonizer.updateMidiVelocitySensitivity(newSensitivity);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updatePitchbendSettings(const int rangeUp, const int rangeDown)
{
    harmonizer.updatePitchbendSettings(rangeUp, rangeDown);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updatePedalPitch(const bool isOn, const int upperThresh, const int interval)
{
    harmonizer.setPedalPitch           (isOn);
    harmonizer.setPedalPitchUpperThresh(upperThresh);
    harmonizer.setPedalPitchInterval   (interval);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateDescant(const bool isOn, const int lowerThresh, const int interval)
{
    harmonizer.setDescant           (isOn);
    harmonizer.setDescantLowerThresh(lowerThresh);
    harmonizer.setDescantInterval   (interval);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateConcertPitch(const int newConcertPitchHz)
{
    harmonizer.setConcertPitchHz(newConcertPitchHz);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateNoteStealing(const bool shouldSteal)
{
    harmonizer.setNoteStealingEnabled(shouldSteal);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateMidiLatch(const bool isLatched)
{
    harmonizer.setMidiLatch(isLatched, true);
};

template<typename SampleType>
void ImogenEngine<SampleType>::updateLimiter(const float thresh, const float release)
{
    limiter.setThreshold(thresh);
    limiter.setRelease(release);
};



template class ImogenEngine<float>;
template class ImogenEngine<double>;

