
/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 ImogenEngine
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenEngine
 description:        base class that wraps the Harmonizer class into a self-sufficient audio engine
 dependencies:       bv_dsp ImogenCommon
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include <bv_dsp/bv_dsp.h>

#include "ImogenCommon/ImogenCommon.h"

#include "Harmonizer/HarmonizerVoice/HarmonizerVoice.h"
#include "Harmonizer/Harmonizer.h"

namespace Imogen
{
template < typename SampleType >
class Engine : public dsp::LatencyEngine< SampleType >
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using MidiBuffer  = juce::MidiBuffer;

    Engine (Parameters& params, Meters& metersToUse, Internals& internalsToUse);

private:
    void renderChunk (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages, bool isBypassed) final;

    void onPrepare (int blocksize, double samplerate) final;
    void onRelease() final;

    void updateAllParameters();
    void updateStereoWidth (int width);
    void updateCompressorAmount (int amount);
    void updateReverbDecay (int decay);
    void updateStereoReductionMode (int mode);

    void processNoiseGate (AudioBuffer& audio);
    void processDeEsser (AudioBuffer& audio);
    void processCompressor (AudioBuffer& audio);
    void processDelay (AudioBuffer& audio);
    void processReverb (AudioBuffer& audio);
    void processLimiter (AudioBuffer& audio);

    Parameters& parameters;
    Meters&     meters;
    Internals&  internals;

    Harmonizer< SampleType > harmonizer;

    AudioBuffer monoBuffer, wetBuffer, dryBuffer;

    dsp::FX::MonoStereoConverter< SampleType > stereoReducer;
    dsp::FX::SmoothedGain< SampleType, 1 >     inputGain;
    dsp::FX::SmoothedGain< SampleType, 2 >     outputGain;
    dsp::FX::NoiseGate< SampleType >           gate;
    dsp::FX::DeEsser< SampleType >             deEsser;
    dsp::FX::Compressor< SampleType >          compressor;
    dsp::FX::Delay< SampleType >               delay;
    dsp::FX::Reverb                            reverb;
    dsp::FX::Limiter< SampleType >             limiter;

    dsp::FX::MonoToStereoPanner< SampleType > dryPanner;

    juce::dsp::ProcessSpec               dspSpec;
    juce::dsp::DryWetMixer< SampleType > dryWetMixer;
};

}  // namespace Imogen
