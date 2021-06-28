#pragma once

#include "PreHarmony/StereoReducer.h"
#include "PreHarmony/InputGain.h"
#include "PreHarmony/NoiseGate.h"

#include "PostHarmony/EQ.h"
#include "PostHarmony/Compressor.h"
#include "PostHarmony/DeEsser.h"
#include "PostHarmony/DryWetMixer.h"
#include "PostHarmony/Delay.h"
#include "PostHarmony/Reverb.h"
#include "PostHarmony/OutputGain.h"
#include "PostHarmony/Limiter.h"

namespace Imogen
{
template < typename SampleType >
class EffectsManager
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    EffectsManager (State& stateToUse);

    void prepare (double samplerate, int blocksize);

    void processPreHarmony (const AudioBuffer& input);

    void processPostHarmony (AudioBuffer& harmonySignal, AudioBuffer& drySignal, AudioBuffer& output);

    const AudioBuffer& getProcessedInputSignal() const;

    void updateStereoWidth (int width);

private:
    State&      state;
    Parameters& parameters {state.parameters};

    AudioBuffer processedMonoBuffer;

    StereoReducer< SampleType >   stereoReducer {parameters};
    dsp::FX::Filter< SampleType > initialLoCut {dsp::FX::FilterType::HighPass, 65.f};
    InputGain< SampleType >       inputGain {state};
    NoiseGate< SampleType >       gate {state};

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
