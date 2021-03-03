/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: Harmonizer
*/

#include "bv_Harmonizer/bv_Harmonizer.h"

#include "bv_Harmonizer/bv_HarmonizerVoice.cpp"
#include "bv_Harmonizer/bv_Harmonizer_VoiceAllocation.cpp"
#include "bv_Harmonizer/bv_Harmonizer_Midi.cpp"
#include "bv_Harmonizer/bv_Harmonizer_AutomatedMidiFeatures.cpp"
#include "bv_Harmonizer/PanningManager/PanningManager.cpp"
#include "bv_Harmonizer/GrainExtractor/GrainExtractor.cpp"



namespace bav

{


template<typename SampleType>
Harmonizer<SampleType>::Harmonizer():
    latchIsOn(false), intervalLatchIsOn(false), sampleRate(44100.0), lastNoteOnCounter(0), lastPitchWheelValue(64), shouldStealNotes(true), lowestPannedNote(0),
    velocityConverter(100), pitchConverter(440, 69, 12), bendTracker(2, 2),
    adsrIsOn(true), lastMidiTimeStamp(0), lastMidiChannel(1), playingButReleasedMultiplier(1.0f), sustainPedalDown(false), sostenutoPedalDown(false), softPedalDown(false), windowSize(0)
{
    adsrParams.attack  = 0.035f;
    adsrParams.decay   = 0.06f;
    adsrParams.sustain = 0.8f;
    adsrParams.release = 0.01f;
    
    quickReleaseParams.attack  = 0.01f;
    quickReleaseParams.decay   = 0.005f;
    quickReleaseParams.sustain = 1.0f;
    quickReleaseParams.release = 0.015f;
    
    quickAttackParams.attack  = 0.015f;
    quickAttackParams.decay   = 0.01f;
    quickAttackParams.sustain = 1.0f;
    quickAttackParams.release = 0.015f;
    
    setConcertPitchHz(440);
    setCurrentPlaybackSampleRate(44100.0);
    
    pedal.isOn = false;
    pedal.lastPitch = -1;
    pedal.upperThresh = 0;
    pedal.interval = 12;
    
    descant.isOn = false;
    descant.lastPitch = -1;
    descant.lowerThresh = 127;
    descant.interval = 12;
    
    currentInputFreq.store(0.0f);
    currentInputPeriod.store(0);
}


template<typename SampleType>
Harmonizer<SampleType>::~Harmonizer()
{
    voices.clear();
}

    
#undef bvh_VOID_TEMPLATE
#define bvh_VOID_TEMPLATE template<typename SampleType> void Harmonizer<SampleType>
    
    
bvh_VOID_TEMPLATE::initialize (const int initNumVoices, const double initSamplerate, const int initBlocksize)
{
    jassert (initNumVoices > 0 && initSamplerate > 0 && initBlocksize > 0);
    
    voices.clear();
    
    addNumVoices (initNumVoices);
    
    pitchDetector.initialize();
    
    setCurrentPlaybackSampleRate (initSamplerate);
    
    prepare (initBlocksize);
}

 
bvh_VOID_TEMPLATE::prepare (const int blocksize)  
{
    jassert (blocksize > 0);
    
    aggregateMidiBuffer.ensureSize (size_t(blocksize * 2));
    aggregateMidiBuffer.clear();
    
    for (auto* voice : voices)
        voice->prepare (blocksize);
    
    windowBuffer.setSize (1, blocksize, true, true, true);
    polarityReversalBuffer.setSize (1, blocksize);
    
    indicesOfGrainOnsets.ensureStorageAllocated (blocksize);
    
    grains.prepare (blocksize);
    panner.prepare (voices.size(), false);
    
    lastNoteOnCounter = 0;
}


bvh_VOID_TEMPLATE::setCurrentPlaybackSampleRate (const double newRate)
{
    jassert (newRate > 0);
    
    sampleRate = newRate;
    
    pitchDetector.setSamplerate (newRate, true);
    
    const float currentFreq = currentInputFreq.load();
    
    if (currentFreq > 0)
        setCurrentInputFreq (currentFreq);
    
    for (auto* voice : voices)
        voice->updateSampleRate (newRate);
}


bvh_VOID_TEMPLATE::releaseResources()
{
    aggregateMidiBuffer.clear();
    usableVoices.clear();
    polarityReversalBuffer.clear();
    windowBuffer.clear();
    intervalsLatchedTo.clear();
    indicesOfGrainOnsets.clear();
    currentNotes.clear();
    desiredNotes.clear();
    
    for (auto* voice : voices)
        voice->releaseResources();
    
    panner.releaseResources();
    grains.releaseResources();
    pitchDetector.releaseResources();
    
    voices.clear();
}
    

bvh_VOID_TEMPLATE::setCurrentInputFreq (const float newInputFreq)
{
    jassert (newInputFreq > 0);
    
    currentInputFreq.store(newInputFreq);
    
    currentInputPeriod.store(roundToInt (sampleRate / newInputFreq));
    
    if (intervalLatchIsOn && ! intervalsLatchedTo.isEmpty())
        playIntervalSet (intervalsLatchedTo, 1.0f, false, true);
}


/***********************************************************************************************************************************************
// audio rendering------------------------------------------------------------------------------------------------------------------------------
 **********************************************************************************************************************************************/

bvh_VOID_TEMPLATE::renderVoices (const AudioBuffer<SampleType>& inputAudio,
                                 AudioBuffer<SampleType>& outputBuffer,
                                 MidiBuffer& midiMessages)
{
    const ScopedLock sl (lock);
    
    outputBuffer.clear();
    
    processMidi (midiMessages);
    
    if (getNumActiveVoices() == 0)
        return;
    
    const float inputFrequency = pitchDetector.detectPitch (inputAudio);  // outputs -1 if frame is unpitched
    
    const bool frameIsPitched = (inputFrequency >= 0.0f);
    
    int periodThisFrame;
    bool polarityReversed = false;
    
    if (frameIsPitched)
    {
        if (currentInputFreq.load() != inputFrequency)
            setCurrentInputFreq (inputFrequency);
        
        periodThisFrame = currentInputPeriod.load();
    }
    else
    {
        // for unpitched frames, an arbitrary "period" is imposed on the signal for analysis grain extraction; this arbitrary period is randomized within a certain range
        
        Random& rand = Random::getSystemRandom();
        
        periodThisFrame = rand.nextInt (unpitchedArbitraryPeriodRange);
        
        if ((rand.nextInt (100) % 2) == 0)  // reverse the polarity approx 50% of the time
        {
            FloatVectorOperations::negate (polarityReversalBuffer.getWritePointer(0),
                                           inputAudio.getReadPointer(0),
                                           inputAudio.getNumSamples());
            
            polarityReversed = true;
        }
    }
    
    fillWindowBuffer (periodThisFrame * 2);
    
    AudioBuffer<SampleType> inverted (polarityReversalBuffer.getArrayOfWritePointers(), 1, 0, inputAudio.getNumSamples());

    const AudioBuffer<SampleType>& actualInput = polarityReversed ? inverted : inputAudio;
    
    grains.getGrainOnsetIndices (indicesOfGrainOnsets, actualInput, periodThisFrame);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->renderNextBlock (actualInput, outputBuffer, periodThisFrame, indicesOfGrainOnsets, windowBuffer);
}



// calculate Hanning window ------------------------------------------------------

bvh_VOID_TEMPLATE::fillWindowBuffer (const int numSamples)
{
    jassert (numSamples > 1);
    
    if (windowSize == numSamples)
        return;
    
    constexpr SampleType zero = SampleType(0.0);
    FloatVectorOperations::fill (windowBuffer.getWritePointer(0), zero, windowBuffer.getNumSamples());
    
    jassert (numSamples <= windowBuffer.getNumSamples());
    
#if BV_HARMONIZER_USE_VDSP
    if constexpr (std::is_same_v <SampleType, float>)
        vDSP_hann_window (windowBuffer.getWritePointer(0), vDSP_Length(numSamples), 2);
    else if constexpr (std::is_same_v <SampleType, double>)
        vDSP_hann_windowD (windowBuffer.getWritePointer(0), vDSP_Length(numSamples), 2);
#else
    const SampleType samplemultiplier = static_cast<SampleType>( (MathConstants<SampleType>::pi * 2.0) / (numSamples - 1) );
    constexpr SampleType one = SampleType(1.0);
    constexpr SampleType pointFive = SampleType(0.5);
    
    auto* writing = windowBuffer.getWritePointer(0);
    
    for (int i = 1; i < numSamples; ++i)
        writing[i] = static_cast<SampleType>( (one - (std::cos (i * samplemultiplier))) * pointFive );
#endif
    
    windowSize = numSamples;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS FOR UPDATING PARAMETERS -----------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// stereo width ---------------------------------------------------------------------------------------------------
bvh_VOID_TEMPLATE::updateStereoWidth (int newWidth)
{
    newWidth = jlimit (0, 100, newWidth);
    
    if (panner.getCurrentStereoWidth() == newWidth)
        return;
    
    panner.updateStereoWidth (newWidth);
    
    for (auto* voice : voices)
    {
        if (! voice->isVoiceActive())
            continue;
        
        if (voice->getCurrentlyPlayingNote() < lowestPannedNote)
        {
            const int currentPan = voice->getCurrentMidiPan();
            
            if (currentPan != 64)
            {
                panner.panValTurnedOff (currentPan);
                voice->setPan (64);
            }
        }
        else
        {
            voice->setPan (panner.getClosestNewPanValFromOld (voice->getCurrentMidiPan()));
        }
    }
}


bvh_VOID_TEMPLATE::updateLowestPannedNote (int newPitchThresh)
{
    newPitchThresh = jlimit (0, 127, newPitchThresh);
    
    const int prevLowestnote = lowestPannedNote.load();
    
    if (prevLowestnote == newPitchThresh)
        return;
    
    lowestPannedNote.store(newPitchThresh);
    
    for (auto* voice : voices)
    {
        if (! voice->isVoiceActive())
            continue;
        
        const int note = voice->getCurrentlyPlayingNote();
        const int currentPan = voice->getCurrentMidiPan();
    
        if (note < newPitchThresh)
        {
            if (currentPan != 64)
            {
                panner.panValTurnedOff (currentPan);
                voice->setPan (64);
            }
        }
        else if (note < prevLowestnote) // because we haven't updated the lowestPannedNote member variable yet, voices with pitches higher than newPitchThresh but lower than lowestPannedNote are the voices that now qualify for panning
        {
            if (currentPan == 64)
                voice->setPan (panner.getNextPanVal());
        }
    }
}

    
// concert pitch hz ------------------------------------------------------------------------------------------------
    
bvh_VOID_TEMPLATE::setConcertPitchHz (const int newConcertPitchhz)
{
    jassert (newConcertPitchhz > 0);
    
    if (pitchConverter.getCurrentConcertPitchHz() == newConcertPitchhz)
        return;
    
    pitchConverter.setConcertPitchHz (newConcertPitchhz);
    
    setCurrentInputFreq (currentInputFreq);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
}


// ADSR settings------------------------------------------------------------------------------------------------------------------------------
bvh_VOID_TEMPLATE::updateADSRsettings (const float attack, const float decay, const float sustain, const float release)
{
    // attack/decay/release time in SECONDS; sustain ratio 0.0 - 1.0
    
    const ScopedLock sl (lock);
    
    adsrParams.attack  = attack;
    adsrParams.decay   = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;
    
    for (auto* voice : voices)
        voice->setAdsrParameters (adsrParams);
}
    

bvh_VOID_TEMPLATE::updateQuickReleaseMs (const int newMs)
{
    if (newMs <= 0)
        return;
    
    const ScopedLock sl (lock);
    
    const float desiredR = newMs / 1000.0f;
    
    if (quickReleaseParams.release == desiredR)
        return;

    quickReleaseParams.release = desiredR;
    quickAttackParams .release = desiredR;
    
    for (auto* voice : voices)
    {
        voice->setQuickReleaseParameters(quickReleaseParams);
        voice->setQuickAttackParameters (quickAttackParams);
    }
}
    

bvh_VOID_TEMPLATE::updateQuickAttackMs (const int newMs)
{
    if (newMs <= 0)
        return;
    
    const ScopedLock sl (lock);
    
    const float desiredA = newMs / 1000.0f;
    
    if (quickAttackParams.attack == desiredA)
        return;
    
    quickAttackParams .attack = desiredA;
    quickReleaseParams.attack = desiredA;
    
    for (auto* voice : voices)
    {
        voice->setQuickAttackParameters (quickAttackParams);
        voice->setQuickReleaseParameters(quickReleaseParams);
    }
}


#undef bvh_VOID_TEMPLATE

template class Harmonizer<float>;
template class Harmonizer<double>;


} // namespace
