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
    harmonizer.setPitchDetectionRange (40.0, 2000.0);
    harmonizer.setPitchDetectionTolerance (0.15);
    
    prevOutputGain = outputGain;
    prevInputGain  = inputGain;
    
    prevDryPanningMults[0] = dryPanningMults[0];
    prevDryPanningMults[1] = dryPanningMults[1];
    
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    dryWetMixer.setMixingRule(dsp::DryWetMixingRule::linear);
    
    prepare (initSamplerate, std::max (initSamplesPerBlock, MAX_BUFFERSIZE));
    
    numStoredInputSamples  = 0;
    numStoredOutputSamples = 0;
    
    const int doubleBlocksize = internalBlocksize * 2;
    
    inputCollectionBuffer .setSize (1, doubleBlocksize, true, true, true);
    inputTransferBuffer   .setSize (1, doubleBlocksize, true, true, true);
    outputCollectionBuffer.setSize (2, doubleBlocksize, true, true, true);
    outputInterimBuffer   .setSize (2, doubleBlocksize, true, true, true);
    
    inBuffer .setSize (1, internalBlocksize, true, true, true);
    dryBuffer.setSize (2, internalBlocksize, true, true, true);
    wetBuffer.setSize (2, internalBlocksize, true, true, true);
    
    initialized = true;
};


template<typename SampleType>
void ImogenEngine<SampleType>::prepare (double sampleRate, int samplesPerBlock)
{
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    if(harmonizer.getSamplerate() != sampleRate)
        harmonizer.setCurrentPlaybackSampleRate(sampleRate);
    
    const int newblocksize = std::max (MAX_BUFFERSIZE, samplesPerBlock); // so we don't deallocate below a sensible size
    
    midiChoppingBuffer.ensureSize (roundToInt(newblocksize * 1.5f));
    
    dspSpec.sampleRate = sampleRate;
    dspSpec.maximumBlockSize = internalBlocksize;
    dspSpec.numChannels = 2;
    
    limiter.prepare(dspSpec);
    
    dryWetMixer.prepare(dspSpec);
    dryWetMixer.setWetLatency(2); // latency in samples of the ESOLA algorithm
    
    bypassDelay.prepare(dspSpec);
    bypassDelay.setDelay(2); // latency in samples of the ESOLA algorithm
    
    harmonizer.resetNoteOnCounter(); // ??
    harmonizer.prepare(newblocksize);
    
    clearBuffers();
    
    resourcesReleased = false;
};


template<typename SampleType>
void ImogenEngine<SampleType>::process (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output,
                                        MidiBuffer& midiMessages,
                                        const bool applyFadeIn, const bool applyFadeOut)
{
    // at this layer, we check to ensure that the buffer sent to us does not exceed the internal blocksize we want to process. If it does, it is broken into smaller chunks, and processWrapped() called on each of these chunks in sequence.
    // at this level, our only garuntee is that will not recieve an empty buffer (ie, 0 samples long)
    
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
    
    // first, deal with midi for this chunk. Because we're working with such small blocksizes, we can safely iterate through all midi messages for this time period before doing audio rendering, instead of trying to chop the audio rendering between midi messages.
    
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
    
    // now, we need to code defensively against small blocksizes to ensure that renderBlock() is only ever fed a chunk of internalBlocksize of the most recently recieved samples
    
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
    
    const int totalNumInputSamplesAvailable = numStoredInputSamples + numNewSamples;
    
    if (totalNumInputSamplesAvailable < internalBlocksize) // not calling renderBlock() this time, not enough samples to process!
    {
        inputCollectionBuffer.copyFrom (0, numStoredInputSamples, input, 0, 0, numNewSamples);
        numStoredInputSamples = totalNumInputSamplesAvailable;
        
        if (numStoredOutputSamples < numNewSamples)
        {
            output.clear();
            return;
        }
        
        for (int chan = 0; chan < 2; ++chan)
            output.copyFrom (chan, 0, outputCollectionBuffer, chan, 0, numNewSamples);
        
        usedOutputSamples (numNewSamples);
        
        return;
    }
    
    AudioBuffer<SampleType> thisChunksOutput (outputCollectionBuffer.getArrayOfWritePointers(), 2, numStoredOutputSamples, internalBlocksize);
    
    if (numStoredInputSamples == 0)
    {
        renderBlock (input, thisChunksOutput, applyFadeIn, applyFadeOut);
    }
    else
    {
        inputCollectionBuffer.copyFrom (0, numStoredInputSamples, input, 0, 0, numNewSamples);
        
        AudioBuffer<SampleType> inputCollectionBufferProxy (inputCollectionBuffer.getArrayOfWritePointers(), 1, 0, internalBlocksize);
        
        renderBlock (inputCollectionBufferProxy, thisChunksOutput, applyFadeIn, applyFadeOut);
    }
    
    for (int chan = 0; chan < 2; ++chan)
        output.copyFrom (chan, 0, outputCollectionBuffer, chan, 0, numNewSamples);
    
    usedOutputSamples (numNewSamples);
    
    numStoredInputSamples = totalNumInputSamplesAvailable - internalBlocksize;
    
    if (numStoredInputSamples <= 0)
    {
        numStoredInputSamples = 0;
        return;
    }

    inputTransferBuffer  .copyFrom (0, 0, inputCollectionBuffer, 0, internalBlocksize, numStoredInputSamples);
    inputCollectionBuffer.copyFrom (0, 0, inputTransferBuffer, 0, 0, numStoredInputSamples);
};



template<typename SampleType>
void ImogenEngine<SampleType>::renderBlock (const AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output,
                                            const bool applyFadeIn, const bool applyFadeOut)
{
    // at this stage, the blocksize is garunteed to ALWAYS be the declared internalBlocksize.
    
    jassert (input.getNumSamples() == internalBlocksize);
    
    harmonizer.analyzeInput (input);
    
    AudioBuffer<SampleType> inputBufferProxy (inBuffer.getArrayOfWritePointers(), 1, 0, internalBlocksize); // internal inBuffer alias
    
    // master input gain
    // N.B. it's possible that the inputProxy already refers to the same memory address as the inputBufferProxy if the modulator source is "sum to mono", because the internal inBuffer is used to store the mono-summed input signal in that process.
    if (input.getReadPointer(0) == inputBufferProxy.getReadPointer(0))
        inputBufferProxy.applyGainRamp (0, internalBlocksize, prevInputGain, inputGain);
    else
        inputBufferProxy.copyFromWithRamp (0, 0, input.getReadPointer(0), internalBlocksize, prevInputGain, inputGain);
    
    prevInputGain = inputGain;
    
    AudioBuffer<SampleType> dryProxy (dryBuffer.getArrayOfWritePointers(), 2, 0, internalBlocksize); // proxy for internal dry buffer
    AudioBuffer<SampleType> wetProxy (wetBuffer.getArrayOfWritePointers(), 2, 0, internalBlocksize); // proxy for internal wet buffer
    
    // write to dry buffer & apply panning (w/ panning multipliers ramped)
    for (int chan = 0; chan < 2; ++chan)
    {
        dryProxy.copyFromWithRamp (chan, 0, inputBufferProxy.getReadPointer(0), internalBlocksize, prevDryPanningMults[chan], dryPanningMults[chan]);
        prevDryPanningMults[chan] = dryPanningMults[chan];
    }
    
    // dry gain
    dryProxy.applyGainRamp (0, internalBlocksize, prevDryGain, dryGain);
    prevDryGain = dryGain;
    
    dryWetMixer.pushDrySamples ( dsp::AudioBlock<SampleType>(dryProxy) );
    
    harmonizer.renderVoices (inputBufferProxy, wetProxy, 0); // puts the harmonizer's rendered stereo output into wetProxy (= "wetBuffer")
    
    // wet gain
    wetProxy.applyGainRamp (0, internalBlocksize, prevWetGain, wetGain);
    prevWetGain = wetGain;
    
    dryWetMixer.mixWetSamples ( dsp::AudioBlock<SampleType>(wetProxy) ); // puts the mixed dry & wet samples into "wetProxy" (= "wetBuffer")
    
    output.makeCopyOf (wetProxy, true); // transfer from wetBuffer to output buffer
    
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
    
    numStoredOutputSamples += internalBlocksize;
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
        outputCollectionBuffer.copyFrom (chan, 0, outputInterimBuffer,    chan, 0,          numStoredOutputSamples);
    }
};



template<typename SampleType>
void ImogenEngine<SampleType>::processBypassed (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output)
{
    const int totalNumSamples = inBus.getNumSamples();
    
    if (totalNumSamples <= wetBuffer.getNumSamples())
    {
        processBypassedWrapped (inBus, output);
        return;
    }
    
    if (processor.isNonRealtime())
    {
        prepare (processor.getSampleRate(), totalNumSamples);
        processBypassedWrapped (inBus, output);
        return;
    }
    
    // simple check to make sure a buggy host doesn't send chunks bigger than the size allotted in prepareToPlay...
    int samplesLeft = totalNumSamples;
    int startSample = 0;
    
    while (samplesLeft > 0)
    {
        const int chunkNumSamples = std::min (wetBuffer.getNumSamples(), samplesLeft);
        
        AudioBuffer<SampleType> inBusProxy (inBus.getArrayOfWritePointers(), inBus.getNumChannels(), startSample, chunkNumSamples);
        AudioBuffer<SampleType> outputProxy (output.getArrayOfWritePointers(), 2, startSample, chunkNumSamples);
        
        processBypassedWrapped (inBusProxy, outputProxy);
        
        startSample += chunkNumSamples;
        samplesLeft -= chunkNumSamples;
    }
};


template<typename SampleType>
void ImogenEngine<SampleType>::processBypassedWrapped (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output)
{
    if (output.getNumChannels() > inBus.getNumChannels())
        for (int chan = inBus.getNumChannels(); chan < output.getNumChannels(); ++chan)
            output.clear(chan, 0, output.getNumSamples());
    
    dsp::AudioBlock<SampleType> inBlock  (inBus);
    dsp::AudioBlock<SampleType> outBlock (output);
    
    // delay line for latency compensation, so that DAW track's total latency will not change whether or not plugin bypass is active
    if (inBlock == outBlock)
        bypassDelay.process (dsp::ProcessContextReplacing   <SampleType> (inBlock) );
    else
        bypassDelay.process (dsp::ProcessContextNonReplacing<SampleType> (inBlock, outBlock) );
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
    bypassDelay.reset();
    
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

template<typename SampleType>
void ImogenEngine<SampleType>::updatePitchDetectionSettings(const float newMinHz, const float newMaxHz, const float newTolerance)
{
    harmonizer.setPitchDetectionRange(newMinHz, newMaxHz);
    harmonizer.setPitchDetectionTolerance(newTolerance);
};



template class ImogenEngine<float>;
template class ImogenEngine<double>;

