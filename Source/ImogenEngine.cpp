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
    inBuffer(1, MAX_BUFFERSIZE), wetBuffer(2, MAX_BUFFERSIZE), dryBuffer(2, MAX_BUFFERSIZE), monoSummingBuffer(1, MAX_BUFFERSIZE * 2),
    resourcesReleased(true), initialized(false)
{ };


template<typename SampleType>
ImogenEngine<SampleType>::~ImogenEngine()
{ };


template<typename SampleType>
void ImogenEngine<SampleType>::initialize (const double initSamplerate, const int initSamplesPerBlock, const int initNumVoices)
{
    for (int i = 0; i < initNumVoices; ++i)
        harmonizer.addVoice(new HarmonizerVoice<SampleType>(&harmonizer));
    
    harmonizer.newMaxNumVoices(std::max(initNumVoices, MAX_POSSIBLE_NUMBER_OF_VOICES));
    harmonizer.setPitchDetectionRange(40.0, 2000.0);
    harmonizer.setPitchDetectionTolerance(0.15);
    
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    dryWetMixer.setMixingRule(dsp::DryWetMixingRule::linear);
    
    prepare (initSamplerate, std::max(initSamplesPerBlock, MAX_BUFFERSIZE));
    
    initialized = true;
};


template<typename SampleType>
void ImogenEngine<SampleType>::prepare (double sampleRate, int samplesPerBlock)
{
    // setLatencySamples(newLatency); // TOTAL plugin latency!
    
    if(harmonizer.getSamplerate() != sampleRate)
        harmonizer.setCurrentPlaybackSampleRate(sampleRate);
    
    const int newblocksize = std::max(MAX_BUFFERSIZE, samplesPerBlock);
    
    wetBuffer.setSize(2, newblocksize, true, true, true);
    dryBuffer.setSize(2, newblocksize, true, true, true);
    inBuffer .setSize(1, newblocksize, true, true, true);
    monoSummingBuffer.setSize(1, newblocksize, true, true, true);
    
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
    
    clearBuffers();
    
    resourcesReleased = false;
};


template<typename SampleType>
void ImogenEngine<SampleType>::process (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output,
                                        MidiBuffer& midiMessages,
                                        const bool applyFadeIn, const bool applyFadeOut,
                                        const bool chopInput)
{
    AudioBuffer<SampleType> input; // input needs to be a MONO buffer!
    
    const int totalNumSamples = inBus.getNumSamples();
    
    switch (processor.getModulatorSource()) // isolate a mono input buffer from the input Bus
    {
        case ImogenAudioProcessor::ModulatorInputSource::left:
            input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers(), 1, totalNumSamples);
            break;
            
        case ImogenAudioProcessor::ModulatorInputSource::right:
            input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers() + (inBus.getNumChannels() > 1), 1, totalNumSamples);
            break;
            
        case ImogenAudioProcessor::ModulatorInputSource::mixToMono:
        {
            if (inBus.getNumChannels() == 1)
            {
                input = AudioBuffer<SampleType> (inBus.getArrayOfWritePointers(), 1, totalNumSamples);
                break;
            }
            
            if(processor.isNonRealtime() && monoSummingBuffer.getNumSamples() < totalNumSamples)
                monoSummingBuffer.setSize(1, totalNumSamples);
            
            monoSummingBuffer.copyFrom(0, 0, inBus, 0, 0, totalNumSamples);
            
            const int totalNumChannels = inBus.getNumChannels();
            
            for (int channel = 1; channel < totalNumChannels; ++channel)
                monoSummingBuffer.addFrom(0, 0, inBus, channel, 0, totalNumSamples);
            
            monoSummingBuffer.applyGain(0, totalNumSamples, 1.0f / totalNumChannels);
            
            input = AudioBuffer<SampleType> (monoSummingBuffer.getArrayOfWritePointers(), 1, totalNumSamples);
            break;
        }
    }
    
    if (chopInput)
        processWithChopping (input, output, midiMessages);
    else
        processNoChopping (input, output, midiMessages);
        
    if (applyFadeIn)
        output.applyGainRamp (0, totalNumSamples, 0.0f, 1.0f);
    
    if (applyFadeOut)
        output.applyGainRamp (0, totalNumSamples, 1.0f, 0.0f);
};


template<typename SampleType>
void ImogenEngine<SampleType>::processBypassed (AudioBuffer<SampleType>& inBus, AudioBuffer<SampleType>& output)
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
void ImogenEngine<SampleType>::processNoChopping (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages)
{
    harmonizer.clearMidiBuffer();
    
    auto midiIterator = midiMessages.findNextSamplePosition(0);
    
    std::for_each (midiIterator,
                   midiMessages.cend(),
                   [&] (const MidiMessageMetadata& meta) { harmonizer.handleMidiEvent (meta.getMessage(), meta.samplePosition); } );
    
    midiMessages.swapWith (harmonizer.returnMidiBuffer());
    
    renderBlock (input, output, 0, input.getNumSamples());
};


template<typename SampleType>
void ImogenEngine<SampleType>::processWithChopping (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output, MidiBuffer& midiMessages)
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
                   [&] (const MidiMessageMetadata& meta) { harmonizer.handleMidiEvent (meta.getMessage(), meta.samplePosition); } );
    
    midiMessages.swapWith(harmonizer.returnMidiBuffer());
};


template<typename SampleType>
void ImogenEngine<SampleType>::renderBlock (AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output,
                                            const int startSample, const int numSamples)
{
    if (processor.isNonRealtime())
    {
        AudioBuffer<SampleType> inProxy  (input .getArrayOfWritePointers(), 1, startSample, numSamples);
        AudioBuffer<SampleType> outProxy (output.getArrayOfWritePointers(), 2, startSample, numSamples);
        
        if (wetBuffer.getNumSamples() < numSamples)
            prepare (processor.getSampleRate(), numSamples);
        
        renderChunk (inProxy, outProxy);
    }
    else
    {
        int chunkStartSample = startSample;
        int samplesLeft      = numSamples;
        
        while(samplesLeft > 0)
        {
            const int chunkNumSamples = std::min(samplesLeft, wetBuffer.getNumSamples());
            
            AudioBuffer<SampleType> inProxy  (input .getArrayOfWritePointers(), 1, chunkStartSample, chunkNumSamples);
            AudioBuffer<SampleType> outProxy (output.getArrayOfWritePointers(), 2, chunkStartSample, chunkNumSamples);
            
            renderChunk (inProxy, outProxy);
            
            chunkStartSample += chunkNumSamples;
            samplesLeft      -= chunkNumSamples;
        }
    }
};


template<typename SampleType>
void ImogenEngine<SampleType>::renderChunk (const AudioBuffer<SampleType>& input, AudioBuffer<SampleType>& output)
{
    const int numSamples = input.getNumSamples();
    
    AudioBuffer<SampleType> inBufferProxy (inBuffer.getArrayOfWritePointers(), 1, 0, numSamples);
    
    inBufferProxy.copyFrom (0, 0, input, 0, 0, numSamples); // copy to input storage buffer so that input gain can be applied
    
    inBufferProxy.applyGain (processor.getInputGainMult()); // apply input gain
    
    AudioBuffer<SampleType> dryProxy (dryBuffer.getArrayOfWritePointers(), 2, 0, numSamples);
    AudioBuffer<SampleType> wetProxy (wetBuffer.getArrayOfWritePointers(), 2, 0, numSamples);
    
    // write to dry buffer & apply panning...
    dryProxy.copyFrom (0, 0, inBufferProxy, 0, 0, numSamples);
    dryProxy.copyFrom (1, 0, inBufferProxy, 0, 0, numSamples);
    dryProxy.applyGain (0, 0, numSamples, processor.getDryPanningMult(0));
    dryProxy.applyGain (1, 0, numSamples, processor.getDryPanningMult(1));
    
    dryWetMixer.pushDrySamples( dsp::AudioBlock<SampleType>(dryProxy) );
    
    harmonizer.renderVoices (inBufferProxy, wetProxy); // puts the harmonizer's rendered stereo output into "wetProxy" (= "wetBuffer")
    
    dryWetMixer.mixWetSamples ( dsp::AudioBlock<SampleType>(wetProxy) ); // puts the mixed dry & wet samples into "wetProxy" (= "wetBuffer")
    
    output.makeCopyOf (wetProxy, true); // transfer from wetBuffer to output buffer
    
    output.applyGain (processor.getOutputGainMult()); // apply master output gain
    
    // output limiter
    if (processor.isLimiterOn())
    {
        dsp::AudioBlock<SampleType> limiterBlock (output);
        limiter.process (dsp::ProcessContextReplacing<SampleType>(limiterBlock));
    }
};


template<typename SampleType>
void ImogenEngine<SampleType>::clearBuffers()
{
    harmonizer.clearBuffers();
    harmonizer.clearMidiBuffer();
    wetBuffer.clear();
    dryBuffer.clear();
    inBuffer.clear();
    monoSummingBuffer.clear();
};


template<typename SampleType>
void ImogenEngine<SampleType>::reset()
{
    harmonizer.allNotesOff(false);
    harmonizer.resetNoteOnCounter();
    clearBuffers();
    dryWetMixer.reset();
    limiter.reset();
    bypassDelay.reset();
};


template<typename SampleType>
void ImogenEngine<SampleType>::releaseResources()
{
    harmonizer.releaseResources();
    
    monoSummingBuffer.setSize(0, 0, false, false, false);
    wetBuffer        .setSize(0, 0, false, false, false);
    dryBuffer        .setSize(0, 0, false, false, false);
    inBuffer         .setSize(0, 0, false, false, false);
    
    clearBuffers();
    
    dryWetMixer.reset();
    limiter.reset();
    bypassDelay.reset();
    
    resourcesReleased = true;
};


// functions for updating parameters ----------------------------------------------------------------------------------------------------------------

template<typename SampleType>
void ImogenEngine<SampleType>::updateNumVoices(const int newNumVoices)
{
    const int currentVoices = harmonizer.getNumVoices();
    
    if(currentVoices != newNumVoices)
    {
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
        
        // update GUI numVoices ComboBox
    }
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

