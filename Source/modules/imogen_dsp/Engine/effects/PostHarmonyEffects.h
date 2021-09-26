#pragma once

#include <lemons_audio_effects/lemons_audio_effects.h>

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
class PostHarmonyEffects
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;

    PostHarmonyEffects (State& stateToUse);

    void prepare (double samplerate, int blocksize);

    void process (AudioBuffer& harmonySignal, AudioBuffer& drySignal, AudioBuffer& output);

    void updateStereoWidth (int width);

private:
    State&      state;
    Parameters& parameters {state.parameters};

    EQ< SampleType >         eq {parameters.eqState};
    Compressor< SampleType > compressor {state};
    DeEsser< SampleType >    deEsser {state};

    DryWetMixer< SampleType > dryWetMixer {parameters};
    Delay< SampleType >       delay {state};
    Reverb< SampleType >      reverb {state};
    OutputGain< SampleType >  outputGain {parameters};
    Limiter< SampleType >     limiter {state};
};

}  // namespace Imogen
