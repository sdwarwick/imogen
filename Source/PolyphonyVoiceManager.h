/*
  ==============================================================================

    PolyphonyVoiceManager.h
    Created: 3 Nov 2020 2:34:29am
    Author:  Ben Vining

 	This class allocates, stores & retrieves incoming MIDI into 12 polyphonic harmony voices, which directly control the 12 instances of HarmonyVoice
 
  ==============================================================================
*/

#pragma once


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
				break;
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
		int voicetest = 0;
		while(voicetest < numberOfVoices)
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
		for(int i = 0; i < numberOfVoices; ++i) {
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
	
	
private:
	const static int numberOfVoices = 12;  // link this to global # of voices setting
	int harmonyPitches[numberOfVoices];
};
