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
    
    inputGainMultiplier = 1.0f;
    prevInputGainMultiplier = 1.0f;
    
    outputGainMultiplier = 1.0f;
    prevOutputGainMultiplier = 1.0f;
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
    
    prevOutputGainMultiplier = outputGainMultiplier;
    prevInputGainMultiplier  = inputGainMultiplier;
    
    prevDryPanningMults[0] = dryPanningMults[0];
    prevDryPanningMults[1] = dryPanningMults[1];
    
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    dryWetMixer.setMixingRule(dsp::DryWetMixingRule::linear);
    
    prepare (initSamplerate, std::max (initSamplesPerBlock, MAX_BUFFERSIZE));
    
    initialized = true;
};


template<typename SampleType>
void ImogenEngine<SampleType>::prepare (double sampleRate, int samplesPerBlock)
{
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    if(harmonizer.getSamplerate() != sampleRate)
        harmonizer.setCurrentPlaybackSampleRate(sampleRate);
    
    const int newblocksize = std::max (MAX_BUFFERSIZE, samplesPerBlock); // so we don't deallocate below a sensible size
    
    wetBuffer.setSize (2, newblocksize, true, true, true);
    dryBuffer.setSize (2, newblocksize, true, true, true);
    inBuffer .setSize (1, newblocksize, true, true, true);
    
    dspSpec.maximumBlockSize = newblocksize;
    dspSpec.sampleRate  = sampleRate;
    dspSpec.numChannels = 2;
    
    limiter.prepare(dspSpec);
    
    dryWetMixer.prepare(dspSpec);
    dryWetMixer.setWetLatency(2); // latency in samples of the ESOLA algorithm
    
    bypassDelay.prepare(dspSpec);
    bypassDelay.setDelay(2); // latency in samples of the ESOLA algorithm
    
    harmonizer.resetNoteOnCounter(); // ??
    harmonizer.prepare(newblocksize);
    
    choppingMidibuffer.ensureSize (roundToInt (samplesPerBlock * 1.5));
    
    clearBuffers();
    
    resourcesReleased = false;
};


template<typename SampleType>
void ImogenEngine<SampleType>::process (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output,
                                        MidiBuffer& midiMessages,
                                        const bool applyFadeIn, const bool applyFadeOut)
{
    const int totalNumSamples = inBus.getNumSamples();
    
    if (totalNumSamples <= wetBuffer.getNumSamples())
    {
        processWrapped (inBus, output, midiMessages, applyFadeIn, applyFadeOut);
        return;
    }
    
    if (processor.isNonRealtime())
    {
        prepare (processor.getSampleRate(), totalNumSamples);
        processWrapped (inBus, output, midiMessages, applyFadeIn, applyFadeOut);
        return;
    }
    
    // simple check to make sure a buggy host doesn't send chunks bigger than the size allotted in prepareToPlay...
    
    int samplesLeft = totalNumSamples;
    int startSample = 0;
    
    bool actuallyFadingIn  = applyFadeIn;
    bool actuallyFadingOut = applyFadeOut;
    
    while (samplesLeft > 0)
    {
        const int chunkNumSamples = std::min (wetBuffer.getNumSamples(), samplesLeft);
        
        AudioBuffer<SampleType> inBusProxy (inBus.getArrayOfWritePointers(), inBus.getNumChannels(), startSample, chunkNumSamples);
        AudioBuffer<SampleType> outputProxy (output.getArrayOfWritePointers(), 2, startSample, chunkNumSamples);
        
        // construct a new midiBuffer containing only events with timestamps contained within this chunk, with sample offset accounted for
        
        choppingMidibuffer.clear();
        
        auto midiIterator  = midiMessages.findNextSamplePosition (startSample);
        const auto midiEnd = midiMessages.findNextSamplePosition (startSample + chunkNumSamples - 1);
        
        std::for_each (midiIterator, midiEnd,
                       [&] (const MidiMessageMetadata& meta)
                           { choppingMidibuffer.addEvent (meta.getMessage(), meta.samplePosition - startSample); } );
        
        processWrapped (inBusProxy, outputProxy, choppingMidibuffer, actuallyFadingIn, actuallyFadingOut);
        
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
    const int totalNumSamples = inBus.getNumSamples();
    
    AudioBuffer<SampleType> input; // input needs to be a MONO buffer!
    
    switch (processor.getModulatorSource()) // isolate a mono input buffer from the input bus, mixing to mono if necessary
    {
        case (ImogenAudioProcessor::ModulatorInputSource::left):
            input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers(), 1, totalNumSamples);
            break;
            
        case (ImogenAudioProcessor::ModulatorInputSource::right):
            input = AudioBuffer<SampleType> ( (inBus.getArrayOfWritePointers() + (inBus.getNumChannels() > 1)), 1, totalNumSamples);
            break;
            
        case (ImogenAudioProcessor::ModulatorInputSource::mixToMono):
        {
            const int totalNumChannels = inBus.getNumChannels();
            
            if (totalNumChannels == 1)
            {
                input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers(), 1, totalNumSamples);
                break;
            }
            
            inBuffer.copyFrom (0, 0, inBus, 0, 0, totalNumSamples);
            
            for (int channel = 1; channel < totalNumChannels; ++channel)
                inBuffer.addFrom (0, 0, inBus, channel, 0, totalNumSamples);
            
            inBuffer.applyGain (1.0f / totalNumChannels);
            
            input = AudioBuffer<SampleType> (inBuffer.getArrayOfWritePointers(), 1, totalNumSamples);
            break;
        }
    }
    
    analyzeInput (input);
    
    if (processor.shouldChopInput())
        renderWithChopping (input, output, midiMessages);
    else
        renderNoChopping (input, output, midiMessages);
    
    if (applyFadeIn)
        output.applyGainRamp (0, totalNumSamples, 0.0f, 1.0f);
    
    if (applyFadeOut)
        output.applyGainRamp (0, totalNumSamples, 1.0f, 0.0f);
};



template<typename SampleType>
void ImogenEngine<SampleType>::renderNoChopping (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output,
                                                  MidiBuffer& midiMessages)
{
    auto midiIterator = midiMessages.findNextSamplePosition(0);
    
    if (midiIterator != midiMessages.cend())
    {
        harmonizer.clearMidiBuffer();
        
        std::for_each (midiIterator,
                       midiMessages.cend(),
                       [&] (const MidiMessageMetadata& meta) { harmonizer.handleMidiEvent (meta.getMessage(), meta.samplePosition); } );
        
        midiMessages.swapWith (harmonizer.returnMidiBuffer());
    }
    
    renderBlock (input, output, 0, input.getNumSamples());
};


template<typename SampleType>
void ImogenEngine<SampleType>::renderWithChopping (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output,
                                                    MidiBuffer& midiMessages)
{
    harmonizer.clearMidiBuffer();
    
    auto midiIterator = midiMessages.findNextSamplePosition(0);
    
    int  numSamples  = input.getNumSamples();
    int  startSample = 0;
    bool firstEvent  = true;
    
    for (; numSamples > 0; ++midiIterator)
    {
        if (midiIterator == midiMessages.cend())
        {
            renderBlock (input, output, startSample, numSamples);
            break;
        }
        
        const auto metadata = *midiIterator;
        const int  samplePosition = metadata.samplePosition;
        const int  samplesToNextMidiMessage = samplePosition - startSample;
        
        if (samplesToNextMidiMessage >= numSamples)
        {
            renderBlock (input, output, startSample, numSamples);
            harmonizer.handleMidiEvent(metadata.getMessage(), samplePosition);
            break;
        }
        
        if (firstEvent && samplesToNextMidiMessage == 0)
        {
            harmonizer.handleMidiEvent(metadata.getMessage(), samplePosition);
            continue;
        }
        
        firstEvent = false;
        
        renderBlock (input, output, startSample, samplesToNextMidiMessage);
        harmonizer.handleMidiEvent(metadata.getMessage(), samplePosition);
        
        startSample += samplesToNextMidiMessage;
        numSamples  -= samplesToNextMidiMessage;
    }
    
    std::for_each (midiIterator,
                   midiMessages.cend(),
                   [&] (const MidiMessageMetadata& meta)
                       { harmonizer.handleMidiEvent (meta.getMessage(), meta.samplePosition); } );
    
    midiMessages.swapWith (harmonizer.returnMidiBuffer());
};


template<typename SampleType>
void ImogenEngine<SampleType>::renderBlock (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output,
                                            const int startSample, const int numSamples)
{
    AudioBuffer<SampleType> inputProxy  (input .getArrayOfWritePointers(), 1, startSample, numSamples); // main input
    AudioBuffer<SampleType> outputProxy (output.getArrayOfWritePointers(), 2, startSample, numSamples); // main output
    
    AudioBuffer<SampleType> inBufferProxy (inBuffer.getArrayOfWritePointers(), 1, 0, numSamples); // internal inBuffer alias
    
    if (inputProxy.getReadPointer(0) == inBufferProxy.getReadPointer(0))
        inBufferProxy.applyGainRamp (0, numSamples, prevInputGainMultiplier, inputGainMultiplier);
    else
        inBufferProxy.copyFromWithRamp (0, 0, inputProxy.getReadPointer(0), numSamples, prevInputGainMultiplier, inputGainMultiplier);
    
    prevInputGainMultiplier = inputGainMultiplier; // check here to make sure that numSamples > 2 ... ??
    
    AudioBuffer<SampleType> dryProxy (dryBuffer.getArrayOfWritePointers(), 2, 0, numSamples); // proxy for internal dry buffer
    AudioBuffer<SampleType> wetProxy (wetBuffer.getArrayOfWritePointers(), 2, 0, numSamples); // proxy for internal wet buffer
    
    // write to dry buffer & apply panning (w/ panning multipliers ramped)
    for (int chan = 0; chan < 2; ++chan)
    {
        dryProxy.copyFromWithRamp (chan, 0, inBufferProxy.getReadPointer(0), numSamples, prevDryPanningMults[chan], dryPanningMults[chan]);
        prevDryPanningMults[chan] = dryPanningMults[chan];
    }
    
    dryWetMixer.pushDrySamples ( dsp::AudioBlock<SampleType>(dryProxy) );
    
    harmonizer.renderVoices (inBufferProxy, wetProxy, startSample); // puts the harmonizer's rendered stereo output into "wetProxy" (= "wetBuffer")
    
    dryWetMixer.mixWetSamples ( dsp::AudioBlock<SampleType>(wetProxy) ); // puts the mixed dry & wet samples into "wetProxy" (= "wetBuffer")
    
    outputProxy.makeCopyOf (wetProxy, true); // transfer from wetBuffer to output buffer
    
    outputProxy.applyGainRamp (0, numSamples, prevOutputGainMultiplier, outputGainMultiplier);
    
    prevOutputGainMultiplier = outputGainMultiplier;
    
    if (! processor.isLimiterOn())
        return;
    
    dsp::AudioBlock<SampleType> limiterBlock (outputProxy);
    limiter.process (dsp::ProcessContextReplacing<SampleType>(limiterBlock));
};


template<typename SampleType>
void ImogenEngine<SampleType>::analyzeInput (const AudioBuffer<SampleType>& input)
{
    harmonizer.analyzeInput (input);
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
    
    // should I include the mono input selection / summing to mono process here...???
    
    // delay line for latency compensation, so that DAW track's total latency will not change whether or not plugin bypass is active
    if (inBlock == outBlock)
        bypassDelay.process (dsp::ProcessContextReplacing   <SampleType> (inBlock) );
    else
        bypassDelay.process (dsp::ProcessContextNonReplacing<SampleType> (inBlock, outBlock) );
};





template<typename SampleType>
void ImogenEngine<SampleType>::clearBuffers()
{
    harmonizer.clearBuffers();
    harmonizer.clearMidiBuffer();
    wetBuffer.clear();
    dryBuffer.clear();
    inBuffer .clear();
    choppingMidibuffer.clear();
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
    
    prevInputGainMultiplier  = inputGainMultiplier;
    prevOutputGainMultiplier = outputGainMultiplier;
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
    prevInputGainMultiplier = inputGainMultiplier;
    inputGainMultiplier = newInGain;
};


template<typename SampleType>
void ImogenEngine<SampleType>::updateOutputGain (const float newOutGain)
{
    
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

