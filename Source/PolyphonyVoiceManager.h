/*
  ==============================================================================

    PolyphonyVoiceManager.h
    Created: 3 Nov 2020 2:34:29am
    Author:  Ben Vining

 	This class allocates, stores & retrieves incoming MIDI into 12 polyphonic harmony voices, which directly control the 12 instances of HarmonyVoice
 
  ==============================================================================
*/

#pragma once

#ifndef NUMBER_OF_VOICES
#define NUMBER_OF_VOICES 12
#endif


class PolyphonyVoiceManager
{
	
public:
	
	PolyphonyVoiceManager() {
		clear();
	};
	
	
	void updatePitchCollection(const int voiceNumber, const int midiPitch)
	{
		harmonyPitches[voiceNumber] = midiPitch;
	};
	
	
	int nextAvailableVoice() const {
		
		bool foundNextVoice = false;
		int voiceTesting = 0;
		while(foundNextVoice == false && voiceTesting < NUMBER_OF_VOICES)
		{
			if(harmonyPitches[voiceTesting] == -1) {
				foundNextVoice = true;
				return voiceTesting;
				break;
			} else {
				++voiceTesting;
			}
		}
		if(foundNextVoice == false) { return -1; }
	};
	
	
	int turnOffNote(const int noteNumber)
	{
		int voicetest = 0;
		while(voicetest < NUMBER_OF_VOICES)
		{
			if(harmonyPitches[voicetest] == noteNumber) {
				harmonyPitches[voicetest] = -1;
				return voicetest;
				break;
			} else {
				++voicetest;
			}
		}
	};
	
	
	bool isPitchActive(const int midiPitch) const {
		bool foundIt = false;
		for(int i = 0; i < NUMBER_OF_VOICES; ++i) {
			if (harmonyPitches[i] == midiPitch) {
				foundIt = true;
				break;
			}
		}
		return foundIt;
	};
	
	
	int pitchAtIndex(const int index) const {
		return harmonyPitches[index];
	};
	
	
	void clear() {
		int i = 0;
		while (i < NUMBER_OF_VOICES) { harmonyPitches[i] = -1; ++i; }
	};
	
	
private:
	int harmonyPitches[NUMBER_OF_VOICES];
};
