#pragma once

namespace Imogen
{
using namespace ParameterValueConversionLambdas;

struct EQState
{
    EQState (ParameterList& list);
    
    ToggleParam eqToggle {"EQ Toggle", "EQ toggle", false};
    HzParam     eqLowShelfFreq {"EQ low shelf freq", "EQ low shelf freq", 80.f};
    FloatParam  eqLowShelfQ {"EQ low shelf Q", "EQ low shelf Q", Qrange, 0.707f};
    FloatParam  eqLowShelfGain {"EQ low shelf gain", "EQ low shelf gain", juce::NormalisableRange< float > (0.f, 4.f, 0.01f), 1.f};
    HzParam     eqHighShelfFreq {"EQ high shelf freq", "EQ high shelf freq", 80.f};
    FloatParam  eqHighShelfQ {"EQ high shelf Q", "EQ high shelf Q", Qrange, 0.707f};
    FloatParam  eqHighShelfGain {"EQ high shelf gain", "EQ high shelf gain", juce::NormalisableRange< float > (0.f, 4.f, 0.01f), 1.f};
    HzParam     eqHighPassFreq {"EQ high pass freq", "EQ high pass freq", 80.f};
    FloatParam  eqHighPassQ {"EQ high pass Q", "EQ high pass Q", Qrange, 0.707f};
    HzParam     eqPeakFreq {"EQ peak freq", "EQ peak freq", 80.f};
    FloatParam  eqPeakQ {"EQ peak Q", "EQ peak Q", Qrange, 0.707f};
    FloatParam  eqPeakGain {"EQ peak gain", "EQ peak gain", juce::NormalisableRange< float > (0.f, 4.f, 0.01f), 1.f};
    
private:
    juce::NormalisableRange< float > Qrange {0.01f, 10.f, 0.01f};
};

}
