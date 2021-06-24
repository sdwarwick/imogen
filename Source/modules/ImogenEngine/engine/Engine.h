#pragma once

#include <bv_synth/bv_synth.h>
#include <bv_psola/bv_psola.h>
#include <ImogenCommon/ImogenCommon.h>


#include <ImogenEngine/effects/StereoReducer.h>
#include <ImogenEngine/effects/InputGain.h>
#include <ImogenEngine/effects/NoiseGate.h>
#include <ImogenEngine/effects/DryPanner.h>

#include <ImogenEngine/Harmonizer/Harmonizer.h>

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
class Engine : public dsp::LatencyEngine< SampleType >
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using MidiBuffer  = juce::MidiBuffer;

    Engine (State& stateToUse);

private:
    void renderChunk (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages, bool isBypassed) final;

    void onPrepare (int blocksize, double samplerate) final;
    void onRelease() final;

    void updateStereoWidth (int width);

    State&      state;
    Parameters& parameters {state.parameters};
    Meters&     meters {state.meters};
    Internals&  internals {state.internals};

    AudioBuffer monoBuffer, wetBuffer, dryBuffer;

    StereoReducer< SampleType >   stereoReducer {parameters};
    dsp::FX::Filter< SampleType > initialLoCut {dsp::FX::FilterType::HighPass, 65.f};
    InputGain< SampleType >       inputGain {state};
    NoiseGate< SampleType >       gate {state};
    DryPanner< SampleType >       dryPanner {parameters};

    Harmonizer< SampleType > harmonizer {state};

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
