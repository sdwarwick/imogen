/*
  ==============================================================================

    DspUtils.h
    Created: 20 Dec 2020 1:13:39pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include "GlobalDefinitions.h"


class MidiConverter
{
public:
	
	MidiConverter(int initialConcertPitch): concertPitchHz(initialConcertPitch)
	{ };
	
	
	float mtof(const float midiNote) // converts midiPitch to frequency in Hz
	{
		return concertPitchHz * std::pow(2.0f, ((midiNote - 69.0f) / 12.0f));
	};
	
	
	float ftom(const float inputFreq) // converts frequency in Hz to midiPitch (as a float)
	{
		return 12.0f * log2(inputFreq / concertPitchHz) + 69.0f;
	};
	
	
	void setConcertPitchHz(const int newConcertPitch)
	{
		concertPitchHz = newConcertPitch;
	}
	
	
	int getCurrentConcertPitchHz()
	{
		return concertPitchHz;
	}
	
	
private:
	
	int concertPitchHz;
	
};
