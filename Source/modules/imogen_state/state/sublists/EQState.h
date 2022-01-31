#pragma once

namespace Imogen
{
struct EQState
{
	EQState (plugin::ParameterList& list);

	ToggleParam eqToggle { "EQ toggle", false };

	HzParam	   eqLowShelfFreq { "EQ low shelf freq", 80.f };
	FloatParam eqLowShelfQ { 0.01f, 10.f, 0.707f, "EQ low shelf Q" };
	FloatParam eqLowShelfGain { 0.f, 4.f, 1.f, "EQ low shelf gain" };

	HzParam	   eqHighShelfFreq { "EQ high shelf freq", 80.f };
	FloatParam eqHighShelfQ { 0.01f, 10.f, 0.707f, "EQ high shelf Q" };
	FloatParam eqHighShelfGain { 0.f, 4.f, 1.f, "EQ high shelf gain" };

	HzParam	   eqHighPassFreq { "EQ high pass freq", 80.f };
	FloatParam eqHighPassQ { 0.01f, 10.f, 0.707f, "EQ high pass Q" };

	HzParam	   eqPeakFreq { "EQ peak freq", 80.f };
	FloatParam eqPeakQ { 0.01f, 10.f, 0.707f, "EQ peak Q" };
	FloatParam eqPeakGain { 0.f, 4.f, 1.f, "EQ peak gain" };
};

}  // namespace Imogen
