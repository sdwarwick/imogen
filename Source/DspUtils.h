/*
  ==============================================================================

    DspUtils.h
    Created: 20 Dec 2020 1:13:39pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include "GlobalDefinitions.h"


class BenUtils
{
public:
	
	static bool isMidiNoteBlackKey(const int midipitch)
	{
		if(const int modulo = midipitch % 12; modulo == 1 || modulo == 3 || modulo == 6 || modulo == 8 || modulo == 10)
			return true;
		else
			return false;
	};
	
};



class MidiConverter
{
public:
	
	MidiConverter(const int initialConcertPitch): concertPitchHz(initialConcertPitch)
	{ };
	
	
	float mtof(const float midiNote) const // converts midiPitch to frequency in Hz
	{
		return concertPitchHz * std::pow(2.0f, ((midiNote - 69.0f) / 12.0f));
	};
	
	
	float ftom(const float inputFreq) const // converts frequency in Hz to midiPitch (as a float)
	{
		return 12.0f * log2(inputFreq / concertPitchHz) + 69.0f;
	};
	
	
	void setConcertPitchHz(const int newConcertPitch)
	{
		concertPitchHz = newConcertPitch;
	}
	
	
	int getCurrentConcertPitchHz() const
	{
		return concertPitchHz;
	}
	
	
private:
	
	int concertPitchHz;
	
};
