/*
  ==============================================================================

    PolyphonyVoiceManager.h
    Created: 3 Nov 2020 2:34:29am
    Author:  Ben Vining

 	This class allocates, stores & retrieves incoming MIDI into 12 polyphonic harmony voices, which directly control the 12 instances of HarmonyVoice
 
  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


class PolyphonyVoiceManager
{
	
public:
	
	void updatePitchCollection(const int voiceNumber, const int midiPitch)
	{
		harmonyPitches[voiceNumber] = midiPitch;
	};
	
	
	int nextAvailableVoice() const {
		
		bool foundNextVoice = false;
		int voiceTesting = 0;
		while(foundNextVoice == false && voiceTesting < numberOfVoices)
		{
			if(harmonyPitches[voiceTesting] == -1) {
				foundNextVoice = true;
				return voiceTesting;
			} else {
				++voiceTesting;
			}
		}
		if(foundNextVoice == false) {
			return -1;  // returns -1 if no voices are available
		}
	};
	
	
	int turnOffNote(const int noteNumber)
	{
		bool foundVoice = false;
		int voicetest = 0;
		while(foundVoice == false && voicetest < numberOfVoices)
		{
			if(harmonyPitches[voicetest] == noteNumber) {
				foundVoice = true;
				harmonyPitches[voicetest] = -1;
				return voicetest;
			} else {
				++voicetest;
			}
		}
	};
	
	
private:
	const static int numberOfVoices = 12;  // link this to global # of voices setting
	int harmonyPitches[numberOfVoices];
};
