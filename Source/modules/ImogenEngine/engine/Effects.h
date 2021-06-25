#pragma once

#include <ImogenEngine/effects/StereoReducer.h>
#include <ImogenEngine/effects/InputGain.h>
#include <ImogenEngine/effects/NoiseGate.h>
#include <ImogenEngine/effects/DryPanner.h>
#include <ImogenEngine/effects/EQ.h>
#include <ImogenEngine/effects/Compressor.h>
#include <ImogenEngine/effects/DeEsser.h>
#include <ImogenEngine/effects/DryWetMixer.h>
#include <ImogenEngine/effects/Delay.h>
#include <ImogenEngine/effects/Reverb.h>
#include <ImogenEngine/effects/OutputGain.h>
#include <ImogenEngine/effects/Limiter.h>

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
    Meters&     meters {state.meters};

    AudioBuffer processedMonoBuffer;
    AudioBuffer pannedLeadBuffer;

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
