#pragma once

#include <ImogenCommon/ImogenCommon.h>

#include <ImogenEngine/Harmonizer/Harmonizer.h>
#include <ImogenEngine/effects/EffectsManager.h>


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

    void updateStereoWidth (int width);

    State&      state;
    Parameters& parameters {state.parameters};

    Harmonizer< SampleType > harmonizer {state};

    EffectsManager< SampleType > effects {state};
};

}  // namespace Imogen
