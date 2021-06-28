#pragma once

#include <ImogenEngine/Harmonizer/Harmonizer.h>

namespace Imogen
{
template < typename SampleType >
class PitchCorrection : public dsp::psola::PitchCorrectorBase<SampleType>
{
public:
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using Harmonizer = Harmonizer<SampleType>;
    
    PitchCorrection (Harmonizer& harm, Internals& internalsToUse);
    
    void renderNextFrame();

    const AudioBuffer& getCorrectedSignal() const;

private:
    
    Internals& internals;

    AudioBuffer correctedBuffer;
};

}  // namespace Imogen
