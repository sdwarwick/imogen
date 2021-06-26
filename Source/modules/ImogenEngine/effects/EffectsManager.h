#pragma once

#include "StereoReducer.h"
#include "InputGain.h"
#include "NoiseGate.h"
#include "DryPanner.h"
#include "EQ.h"
#include "Compressor.h"
#include "DeEsser.h"
#include "DryWetMixer.h"
#include "Delay.h"
#include "Reverb.h"
#include "OutputGain.h"
#include "Limiter.h"

namespace Imogen
{
template < typename SampleType >
class EffectsManager
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    EffectsManager (State& stateToUse);

    void prepare (double samplerate, int blocksize);

    void processPreHarmony (const AudioBuffer& input, bool leadIsBypassed);

    void processPostHarmony (AudioBuffer& harmonySignal, AudioBuffer& output);

    const AudioBuffer& getProcessedInputSignal() const;

    void updateStereoWidth (int width);

private:
    State&      state;
    Parameters& parameters {state.parameters};

    AudioBuffer processedMonoBuffer, pannedLeadBuffer;

    StereoReducer< SampleType >   stereoReducer {parameters};
    dsp::FX::Filter< SampleType > initialLoCut {dsp::FX::FilterType::HighPass, 65.f};
    InputGain< SampleType >       inputGain {state};
    NoiseGate< SampleType >       gate {state};
    DryPanner< SampleType >       dryPanner {parameters};

    EQ< SampleType >         EQ {parameters};
    Compressor< SampleType > compressor {state};
    DeEsser< SampleType >    deEsser {state};

    DryWetMixer< SampleType > dryWetMixer {parameters};
    Delay< SampleType >       delay {state};
    Reverb< SampleType >      reverb {state};
    OutputGain< SampleType >  outputGain {parameters};
    Limiter< SampleType >     limiter {state};
};

}  // namespace Imogen
