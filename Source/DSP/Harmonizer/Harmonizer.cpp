
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
 
 bv_Harmonizer.cpp: This file defines implementation details for the Harmonizer class. The Harmonizer is essentially a synthesizer that pitch shifts an input instead of using oscillators to make sound.
 
======================================================================================================================================================*/


#include "HarmonizerVoice.cpp"
#include "PSOLA/GrainExtractor/GrainExtractor.cpp"


namespace bav
{
template < typename SampleType >
Harmonizer< SampleType >::Harmonizer() //: autoPitch(&analyzer)
{
    Base::setConcertPitchHz (440);

    pitchDetector.setConfidenceThresh (pitchDetectionConfidenceThresh);

    Base::updateQuickReleaseMs (adsrQuickReleaseMs);

    Base::playingButReleasedMultiplier = playingButReleasedGainMultiplier;
    Base::softPedalMultiplier          = softPedalGainMultiplier;
}


template < typename SampleType >
void Harmonizer< SampleType >::initialized (const double initSamplerate, const int initBlocksize)
{
    pitchDetector.initialize();
    juce::ignoreUnused (initSamplerate, initBlocksize);
    //    autoPitch.initialize();
}


template < typename SampleType >
void Harmonizer< SampleType >::prepared (int blocksize)
{
    inputStorage.setSize (1, blocksize);
    analyzer.prepare (blocksize);
}


template < typename SampleType >
void Harmonizer< SampleType >::resetTriggered()
{
    analyzer.reset();
    //    autoPitch.reset();
}


template < typename SampleType >
void Harmonizer< SampleType >::samplerateChanged (double newSamplerate)
{
    pitchDetector.setSamplerate (newSamplerate);
}


template < typename SampleType >
void Harmonizer< SampleType >::updatePitchDetectionHzRange (const int minHz, const int maxHz)
{
    pitchDetector.setHzRange (minHz, maxHz);

    if (Base::sampleRate > 0) pitchDetector.setSamplerate (Base::sampleRate);
}


template < typename SampleType >
void Harmonizer< SampleType >::release()
{
    inputStorage.clear();
    pitchDetector.releaseResources();
    analyzer.releaseResources();
    //    autoPitch.releaseResources();
}


template < typename SampleType >
void Harmonizer< SampleType >::render (const AudioBuffer& input, AudioBuffer& output, juce::MidiBuffer& midiMessages)
{
    jassert (input.getNumSamples() == output.getNumSamples());
    jassert (output.getNumChannels() == 2);

    if (Base::getNumActiveVoices() == 0)
        analyzer.reset();
    else
        analyzer.clearUnusedGrains();

    analyzeInput (input);

    Base::renderVoices (midiMessages, output);
}


template < typename SampleType >
void Harmonizer< SampleType >::analyzeInput (const AudioBuffer& inputAudio)
{
    jassert (Base::sampleRate > 0);

    const auto inputFrequency = pitchDetector.detectPitch (inputAudio); // outputs 0.0 if frame is unpitched
    const bool frameIsPitched = inputFrequency > 0;

    const auto numSamples = inputAudio.getNumSamples();

    vecops::copy (inputAudio.getReadPointer (0), inputStorage.getWritePointer (0), numSamples);

    if (!frameIsPitched && bav::math::probability (50)) // for unpitched frames, reverse the polarity approx 50% of the time
        vecops::multiplyC (inputStorage.getWritePointer (0), SampleType (-1), numSamples);

    period = frameIsPitched ? juce::roundToInt (Base::sampleRate / inputFrequency)
                            : juce::Random::getSystemRandom().nextInt (pitchDetector.getCurrentLegalPeriodRange());

    jassert (period > 0);

    analyzer.analyzeInput (inputStorage.getReadPointer (0), numSamples, period);
}


template < typename SampleType >
void Harmonizer< SampleType >::addNumVoices (const int voicesToAdd)
{
    if (voicesToAdd == 0) return;

    for (int i = 0; i < voicesToAdd; ++i)
        Base::voices.add (new Voice (this));

    jassert (Base::voices.size() >= voicesToAdd);

    Base::numVoicesChanged();
}


template class Harmonizer< float >;
template class Harmonizer< double >;


} // namespace bav
