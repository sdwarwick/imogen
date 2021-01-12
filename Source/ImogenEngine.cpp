/*
  ==============================================================================

    ImogenEngine.cpp
    Created: 6 Jan 2021 12:30:40am
    Author:  Ben Vining

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "GlobalDefinitions.h"

#include "DelayBuffer.h"


template<typename SampleType>
ImogenEngine<SampleType>::ImogenEngine(ImogenAudioProcessor& p):
    processor(p),
    resourcesReleased(true), initialized(false),
    lastRecievedFadeIn(false), lastRecievedFadeOut(false)
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
    
    const int aggregateBufferSizes = internalBlocksize * 2;
    
    inputCollectionBuffer .setSize (1, aggregateBufferSizes, true, true, true);
    outputCollectionBuffer.setSize (2, aggregateBufferSizes, true, true, true);
    copyingInterimBuffer  .setSize (2, aggregateBufferSizes, true, true, true);
    
    const int midiBufferSizes = std::max (aggregateBufferSizes * 2, samplesPerBlock * 2);
    
    midiChoppingBuffer  .ensureSize (midiBufferSizes * 2);
    midiInputCollection .ensureSize (midiBufferSizes);
    midiOutputCollection.ensureSize (midiBufferSizes);
    
    chunkMidiBuffer.ensureSize(aggregateBufferSizes);
    
    dspSpec.sampleRate = sampleRate;
    dspSpec.maximumBlockSize = internalBlocksize;
    dspSpec.numChannels = 2;
    
    limiter.prepare (dspSpec);
    
    dryWetMixer.prepare (dspSpec);
    dryWetMixer.setWetLatency(0); // latency in samples of the ESOLA algorithm
    
    harmonizer.resetNoteOnCounter(); // ??
    harmonizer.prepare (aggregateBufferSizes);
    
    clearBuffers();
    
    firstLatencyPeriod = true;
    
    resourcesReleased = false;
};


template<typename SampleType>
void ImogenEngine<SampleType>::clearBuffers()
{
    harmonizer.clearBuffers();
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
    
    firstLatencyPeriod = true;
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
    
    resourcesReleased  = true;
    initialized        = false;
    firstLatencyPeriod = true;
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    AUDIO RENDERING
 
    The internal algorithm always processes samples in blocks of internalBlocksize samples, regardless of the buffer sizes recieved from the host. This regulated-blocksize processing happens in the renderBlock() function.
    In order to achieve this blocksize regulation, the processing is wrapped in several layers of buffer slicing and what essentially amounts to an audio & MIDI FIFO.
 */

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
        
        // put just the midi messages for this time segment into midiChoppingBuffer
        // the harmonizer's midi output will be returned by being copied to this same region of the midiChoppingBuffer.
        midiChoppingBuffer.clear();
        copyRangeOfMidiBuffer (midiMessages, midiChoppingBuffer, startSample, 0, chunkNumSamples);
        
        processWrapped (inBusProxy, outputProxy, midiChoppingBuffer, actuallyFadingIn, actuallyFadingOut);
        
        // copy the harmonizer's midi output (the beginning chunk of midiChoppingBuffer) back to midiMessages, at the original startSample
        copyRangeOfMidiBuffer (midiChoppingBuffer, midiMessages, 0, startSample, chunkNumSamples);
        
        startSample += chunkNumSamples;
        samplesLeft -= chunkNumSamples;
    
        actuallyFadingIn  = false;
        actuallyFadingOut = false;
    }
};


template<typename SampleType>
void ImogenEngine<SampleType>::processWrapped (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output,
                                               MidiBuffer& midiMessages,
                                               const bool applyFadeIn, const bool applyFadeOut)
{
    // at this level, the buffer block sizes sent to us are garunteed to NEVER exceed the declared internalBlocksize, but they may still be SMALLER than this blocksize -- the individual buffers this function recieves may be as short as 1 sample long.
    // this is where the secret sauce happens that ensures the consistent block sizes fed to renderBlock()
    
    const int numNewSamples = inBus.getNumSamples();
    
    jassert (numNewSamples <= internalBlocksize);
    
    AudioBuffer<SampleType> input; // input needs to be a MONO buffer!
    
    switch (processor.getModulatorSource()) // isolate a mono input buffer from the input bus, mixing to mono if necessary
    {
        case (ImogenAudioProcessor::ModulatorInputSource::left):
        {
            input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers(), 1, numNewSamples);
            break;
        }
            
        case (ImogenAudioProcessor::ModulatorInputSource::right):
        {
            input = AudioBuffer<SampleType> ( (inBus.getArrayOfWritePointers() + (inBus.getNumChannels() > 1)), 1, numNewSamples);
            break;
        }
            
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
    
    // add new midi events recieved into the back of the midiInputCollection buffer queue
    addToEndOfMidiBuffer (midiMessages, midiInputCollection, numNewSamples);
    
    if (numStoredInputSamples < internalBlocksize) // not calling renderBlock() this time, not enough samples to process!
    {
        if (numStoredOutputSamples == 0) // this should only trigger during the first latency period of all time!
        {
            jassert (firstLatencyPeriod);
            output.clear();
            return;
        }
    }
    else // render the new chunk of internalBlocksize samples
    {
        firstLatencyPeriod = false;
        
        // alias buffer referring to just the chunk of the inputCollectionBuffer that we'll be reading from
        // our start sample here is always 0 in the inputCollectionBuffer, with block size internalBlocksize
        AudioBuffer<SampleType> thisChunksInput  (inputCollectionBuffer.getArrayOfWritePointers(), 1, 0, internalBlocksize);
        
        // alias buffer referring to just the chunk of the outputCollectionBuffer that we'll be writing to
        // our start sample here is numStoredOutputSamples - meaning we're appending these new output samples to the END of the outputCollectionBuffer
        AudioBuffer<SampleType> thisChunksOutput (outputCollectionBuffer.getArrayOfWritePointers(), 2, numStoredOutputSamples, internalBlocksize);
        
        // copy just the next internalBlocksize worth's of midi events into the chunkMidiBuffer
        chunkMidiBuffer.clear();
        copyRangeOfMidiBuffer (midiInputCollection, chunkMidiBuffer, 0, 0, internalBlocksize);
        deleteMidiEventsAndPushUpRest (midiInputCollection, internalBlocksize);
        
        // appends the next rendered block of samples to the end of the outputCollectionBuffer
        renderBlock (thisChunksInput, thisChunksOutput, chunkMidiBuffer);
        
        // appends the returned midi buffer to the end of the midiOutputCollection buffer
        addToEndOfMidiBuffer (chunkMidiBuffer, midiOutputCollection, internalBlocksize);
        
        numStoredOutputSamples += internalBlocksize;
        numStoredInputSamples  -= internalBlocksize;
        
        if (numStoredInputSamples > 0)
            pushUpLeftoverSamples (inputCollectionBuffer, internalBlocksize, numStoredInputSamples);
    }
    
    jassert (numNewSamples <= numStoredOutputSamples);
    
    // write the next numNewSamples worth of output samples to the output, always starting from sample 0 of outputCollectionBuffer
    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, outputCollectionBuffer, chan, 0, numNewSamples);
    
    numStoredOutputSamples -= numNewSamples;
    
    if (numStoredOutputSamples > 0)
        pushUpLeftoverSamples (outputCollectionBuffer, numNewSamples, numStoredOutputSamples);
    
    // copy the next numNewSamples's worth of midi events from the midiOutputCollection buffer to the output midi buffer
    copyRangeOfMidiBuffer (midiOutputCollection, midiMessages, 0, 0, numNewSamples);
    deleteMidiEventsAndPushUpRest (midiOutputCollection, numNewSamples);
    
    if (applyFadeIn)
        output.applyGainRamp(0, numNewSamples, 0.0f, 1.0f);
    
    if (applyFadeOut)
        output.applyGainRamp(0, numNewSamples, 1.0f, 0.0f);
};



template<typename SampleType>
void ImogenEngine<SampleType>::renderBlock (const AudioBuffer<SampleType>& input,
                                                  AudioBuffer<SampleType>& output,
                                                  MidiBuffer& midiMessages)
{
    // at this stage, the blocksize is garunteed to ALWAYS be the declared internalBlocksize.
    
    harmonizer.processMidi (midiMessages);
    
    // master input gain
    inBuffer.copyFromWithRamp (0, 0, input.getReadPointer(0), internalBlocksize, prevInputGain, inputGain);
    prevInputGain = inputGain;
    
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
    
    harmonizer.renderVoices (inBuffer, wetBuffer); // puts the harmonizer's rendered stereo output into wetBuffer
    
    // wet gain
    wetBuffer.applyGainRamp (0, internalBlocksize, prevWetGain, wetGain);
    prevWetGain = wetGain;
    
    dryWetMixer.mixWetSamples ( dsp::AudioBlock<SampleType>(wetBuffer) ); // puts the mixed dry & wet samples into "wetProxy" (= "wetBuffer")
    
    output.makeCopyOf (wetBuffer, true); // transfer from wetBuffer to output buffer
    
    // master output gain
    output.applyGainRamp (0, internalBlocksize, prevOutputGain, outputGain);
    prevOutputGain = outputGain;
    
    if (! processor.limiterToggle->get())
        return;

    dsp::AudioBlock<SampleType> limiterBlock (output);
    limiter.process (dsp::ProcessContextReplacing<SampleType>(limiterBlock));
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    AUDIO RENDERING WHILE THE PLUGIN IS IN BYPASS MODE
 
    With the current setup, when the plugin is in bypass mode, it continues the same FIFO wrapping process around internal blocks of internalBlocksize, in order to preserve the overall latency of the plugin.
    In order to be able to use the same buffer system, while in bypass mode, the input will be summed to mono and copied to every output channel.
 
    I am considering implementing the switch for the input selection process in processBypassedWrapped... ¯\_(ツ)_/¯
 */

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
    
    // with current setup, input will be mixed to mono & copied to each output channel.
    if (inBus.getNumChannels() == 1)
        input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers(), 1, numNewSamples);
    else
    {
        const int totalNumChannels = inBus.getNumChannels();
        
        inBuffer.copyFrom (0, 0, inBus, 0, 0, numNewSamples);
        
        for (int channel = 1; channel < totalNumChannels; ++channel)
            inBuffer.addFrom (0, 0, inBus, channel, 0, numNewSamples);
        
        inBuffer.applyGain (1.0f / totalNumChannels);
        
        input = AudioBuffer<SampleType> (inBuffer.getArrayOfWritePointers(), 1, numNewSamples);
    }
    
    inputCollectionBuffer.copyFrom (0, numStoredInputSamples, input, 0, 0, numNewSamples);
    numStoredInputSamples += numNewSamples;
    
    if (numStoredInputSamples < internalBlocksize)
    {
        if (numStoredOutputSamples == 0)
        {
            jassert (firstLatencyPeriod);
            output.clear();
            return;
        }
    }
    else
    {
        firstLatencyPeriod = false;
        
        for (int chan = 0; chan < 2; ++chan)
            outputCollectionBuffer.copyFrom (chan, numStoredOutputSamples, inputCollectionBuffer, 0, 0, internalBlocksize);
        
        numStoredOutputSamples += internalBlocksize;
        numStoredInputSamples  -= internalBlocksize;
        
        if (numStoredInputSamples > 0)
            pushUpLeftoverSamples (inputCollectionBuffer, internalBlocksize, numStoredInputSamples);
    }
    
    jassert (numNewSamples <= numStoredOutputSamples);
    
    // write to output
    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, outputCollectionBuffer, chan, 0, numNewSamples);
    
    numStoredOutputSamples -= numNewSamples;
    
    if (numStoredOutputSamples > 0)
        pushUpLeftoverSamples (outputCollectionBuffer, numNewSamples, numStoredOutputSamples);
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
    VARIOUS UTILITY FUNCTIONS
 
    ... for helping with management of the audio & MIDI FIFO
 */

// copies samples from a later range of an AudioBuffer to the front of that AudioBuffer.
// ex. take samples 45 - 50 of inputCollectionBuffer and copy them to sample indices 0 - 4 of inputCollectionBuffer
template<typename SampleType>
void ImogenEngine<SampleType>::pushUpLeftoverSamples (AudioBuffer<SampleType>& targetBuffer,
                                                      const int numSamplesUsed, const int numSamplesLeft)
{
    for (int chan = 0; chan < 2; ++chan)
    {
        copyingInterimBuffer.copyFrom (chan, 0, targetBuffer,         chan, numSamplesUsed, numSamplesLeft);
        targetBuffer        .copyFrom (chan, 0, copyingInterimBuffer, chan, 0,              numSamplesLeft);
    }
};

// appends midi events from one MidiBuffer to the end of an aggregateBuffer. The number of events copied will correspond to the numSamples argument.
template<typename SampleType>
void ImogenEngine<SampleType>::addToEndOfMidiBuffer (const MidiBuffer& sourceBuffer, MidiBuffer& aggregateBuffer,
                                                     const int numSamples)
{
    auto sourceStart = sourceBuffer.findNextSamplePosition(0);
    
    if (sourceStart == sourceBuffer.cend())
        return;
    
    const auto sourceEnd = sourceBuffer.findNextSamplePosition(numSamples - 1);
    
    if (sourceStart == sourceEnd)
        return;
    
    int aggregateStartSample = aggregateBuffer.getLastEventTime() + 1;
    
    std::for_each (sourceStart, sourceEnd,
                   [&] (const MidiMessageMetadata& meta)
                       {
                           aggregateBuffer.addEvent (meta.getMessage(),
                                                     meta.samplePosition + aggregateStartSample);
                       } );
};

// deletes midi events in the targetBuffer from timestamps 0 to numSamplesUsed, and copies any events left from that timestamp to the end of the targetBuffer to now be at the start of the targetBuffer
template<typename SampleType>
void ImogenEngine<SampleType>::deleteMidiEventsAndPushUpRest (MidiBuffer& targetBuffer,
                                                              const int numSamplesUsed)
{
    MidiBuffer temp (targetBuffer);
    
    targetBuffer.clear();
    
    auto copyingStart = temp.findNextSamplePosition(numSamplesUsed - 1);
    
    if (copyingStart == temp.cend())
        return;
    
    std::for_each (copyingStart, temp.cend(),
                   [&] (const MidiMessageMetadata& meta)
                       {
                           targetBuffer.addEvent (meta.getMessage(),
                                                  std::max (0,
                                                           (meta.samplePosition - numSamplesUsed + 1)) );
                       } );
};

// copies a range of events from one MidiBuffer to another MidiBuffer, applying a timestamp offset. The number of events copied will correspond to the numSamples argument.
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
    
    const auto midiEnd = inputBuffer.findNextSamplePosition(startSampleOfInput + numSamples);
    
    if (midiIterator == midiEnd)
        return;
    
    const int sampleOffset = startSampleOfOutput - startSampleOfInput;
    
    std::for_each (midiIterator, midiEnd,
                   [&] (const MidiMessageMetadata& meta)
                       {
                           outputBuffer.addEvent (meta.getMessage(),
                                                  std::max (0,
                                                            meta.samplePosition + sampleOffset));
                       } );
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

