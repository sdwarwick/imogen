/*
  ==============================================================================

    PolyphonyVoiceManager.h
    Created: 3 Nov 2020 2:34:29am
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PolyphonyVoiceManager {
	
public:
	
	const static int numberOfVoices = 12;
	int harmonyPitches[numberOfVoices];
	
	void updatePitchCollection(int voiceNumber, int midiPitch)
	{
		harmonyPitches[voiceNumber] = midiPitch;
	}
	
	
	int nextAvailableVoice()
	{
		bool foundNextVoice = false;
		int voiceTesting = 0;
		while(foundNextVoice == false && voiceTesting < numberOfVoices)
		{
			if(harmonyPitches[voiceTesting] == -1) {
				foundNextVoice = true;
				return voiceTesting;
			} else {
				voiceTesting++;
			}
		}
	}
	
	
	int turnOffNote(int noteNumber)
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
				voicetest++;
			}
		}
	}
	
	
private:
	
	
	
};
