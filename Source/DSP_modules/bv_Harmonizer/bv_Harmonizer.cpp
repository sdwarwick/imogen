/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: Harmonizer
*/

#include "bv_HarmonizerVoice.cpp"
#include "GrainExtractor/GrainExtractor.cpp"


#define bvh_ADSR_QUICK_ATTACK_MS 5
#define bvh_ADSR_QUICK_RELEASE_MS 5

#define bvh_PLAYING_BUT_RELEASED_GAIN_MULTIPLIER 0.4
#define bvh_SOFT_PEDAL_GAIN_MULTIPLIER 0.65

#define bvh_PITCH_DETECTION_CONFIDENCE_THRESH 0.15


namespace bav
{


template<typename SampleType>
Harmonizer<SampleType>::Harmonizer():
    windowSize(0),
    currentInputFreq(0.0f)
{
    Base::setConcertPitchHz(440);
    
    pitchDetector.setConfidenceThresh (SampleType(bvh_PITCH_DETECTION_CONFIDENCE_THRESH));
    
    Base::updateQuickAttackMs (bvh_ADSR_QUICK_ATTACK_MS);
    Base::updateQuickReleaseMs (bvh_ADSR_QUICK_RELEASE_MS);
    
    Base::playingButReleasedMultiplier = float(bvh_PLAYING_BUT_RELEASED_GAIN_MULTIPLIER);
    Base::softPedalMultiplier = float(bvh_SOFT_PEDAL_GAIN_MULTIPLIER);
}
    
#undef bvh_ADSR_QUICK_ATTACK_MS
#undef bvh_ADSR_QUICK_RELEASE_MS
#undef bvh_PLAYING_BUT_RELEASED_GAIN_MULTIPLIER
#undef bvh_SOFT_PEDAL_GAIN_MULTIPLIER

    
template<typename SampleType>
void Harmonizer<SampleType>::initialized (const double initSamplerate, const int initBlocksize)
{
    pitchDetector.initialize();
    juce::ignoreUnused (initSamplerate, initBlocksize);
}


template<typename SampleType>
void Harmonizer<SampleType>::prepared (int blocksize)
{
    windowBuffer.setSize (1, blocksize, true, true, true);
    inputStorage.setSize (1, blocksize);

    indicesOfGrainOnsets.ensureStorageAllocated (blocksize);

    grains.prepare (blocksize);
}


template<typename SampleType>
void Harmonizer<SampleType>::samplerateChanged (double newSamplerate)
{
    pitchDetector.setSamplerate (newSamplerate);
}
    

template<typename SampleType>
void Harmonizer<SampleType>::updatePitchDetectionHzRange (const int minHz, const int maxHz)
{
    pitchDetector.setHzRange (minHz, maxHz);
    
    if (Base::sampleRate > 0)
        pitchDetector.setSamplerate (Base::sampleRate);
}

    
template<typename SampleType>
void Harmonizer<SampleType>::release()
{
    inputStorage.clear();
    windowBuffer.clear();
    indicesOfGrainOnsets.clear();
    grains.releaseResources();
    pitchDetector.releaseResources();
}

    
template<typename SampleType>
void Harmonizer<SampleType>::analyzeInput (const AudioBuffer& inputAudio)
{
    jassert (Base::sampleRate > 0);
    
    const float inputFrequency = pitchDetector.detectPitch (inputAudio);  // outputs 0.0 if frame is unpitched
    const bool frameIsPitched  = inputFrequency > 0.0f;
    
    const int numSamples = inputAudio.getNumSamples();
    
    int periodThisFrame;
    bool polarityReversed = false;
    
    if (frameIsPitched)
    {
        currentInputFreq = inputFrequency;
        periodThisFrame = juce::roundToInt (Base::sampleRate / inputFrequency);
    }
    else
    {
        // for unpitched frames, an arbitrary "period" is imposed on the signal for analysis grain extraction; this arbitrary period is randomized within a certain range
        periodThisFrame = juce::Random::getSystemRandom().nextInt (unpitchedArbitraryPeriodRange);
        
        if (bav::math::probability (50))  // for unpitched frames, also reverse the polarity approx 50% of the time
        {
            FVO::negate (inputStorage.getWritePointer(0), inputAudio.getReadPointer(0), numSamples);
            polarityReversed = true;
        }
    }
    
    if (! polarityReversed)
        FVO::copy (inputStorage.getWritePointer(0), inputAudio.getReadPointer(0), numSamples);
    
    fillWindowBuffer (periodThisFrame * 2);
    
    thisFramesInput = polarityReversed ? inputStorage : inputAudio;
    nextFramesPeriod = periodThisFrame;
    
    grains.getGrainOnsetIndices (indicesOfGrainOnsets, thisFramesInput, periodThisFrame);
}


// calculate Hanning window ------------------------------------------------------

template<typename SampleType>
inline void Harmonizer<SampleType>::fillWindowBuffer (const int numSamples)
{
    jassert (numSamples > 1);
    
    if (windowSize == numSamples)
        return;
    
    constexpr SampleType zero = SampleType(0.0);
    juce::FloatVectorOperations::fill (windowBuffer.getWritePointer(0), zero, windowBuffer.getNumSamples());
    
    jassert (numSamples <= windowBuffer.getNumSamples());
    
    juce::dsp::WindowingFunction<SampleType>::fillWindowingTables (windowBuffer.getWritePointer(0),
                                                                   size_t(numSamples),
                                                                   juce::dsp::WindowingFunction<SampleType>::hann,
                                                                   true);
    windowSize = numSamples;
}
    
    
// adds a specified # of voices
template<typename SampleType>
void Harmonizer<SampleType>::addNumVoices (const int voicesToAdd)
{
    if (voicesToAdd == 0)
        return;
    
    for (int i = 0; i < voicesToAdd; ++i)
        Base::voices.add (new Voice(this));
    
    jassert (Base::voices.size() >= voicesToAdd);
    
    Base::numVoicesChanged();
}


template class Harmonizer<float>;
template class Harmonizer<double>;


} // namespace
