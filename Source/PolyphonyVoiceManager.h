/*
  ==============================================================================

    PolyphonyVoiceManager.h
    Created: 3 Nov 2020 2:34:29am
    Author:  Ben Vining

 	This class allocates, stores & retrieves incoming MIDI into 12 polyphonic harmony voices, which directly control the 12 instances of HarmonyVoice
 
 	Stores midiPitch values. -1 signifies an inactive voice.
 
  ==============================================================================
*/

#pragma once

#include "GlobalDefinitions.h"
#include "MidiPanningManager.h"
#include "VoiceStealingManager.h"
#include "MidiLatchManager.h"


class PolyphonyVoiceManager
{
	
public:
	
	PolyphonyVoiceManager(MidiPanningManager& p, VoiceStealingManager& v, MidiLatchManager& l): panningManager(p), stealingManager(v), latchManager(l) {
		clear();
	};
	
	
	void updatePitchCollection(const int voiceNumber, const int midiPitch)
	{
		harmonyPitches[voiceNumber] = midiPitch;
		
		if(midiPitch == -1)
		{
			if(areAllVoicesOff() == true) {
				panningManager.reset();
				stealingManager.clear();
				latchManager.clear();
			}
		}
	};
	
	
	int nextAvailableVoice() const {
		int voiceNumber = -1;
		int voiceTesting = 0;
		while(voiceTesting < NUMBER_OF_VOICES)
		{
			if(harmonyPitches[voiceTesting] == -1) {
				voiceNumber = voiceTesting;
				break;
			} else {
				++voiceTesting;
			}
		}
		return voiceNumber; // returns -1 if no voices are available (ie, a voice must be "stolen")
	};
	
	
	int getIndex(const int pitch) const {
		int index = -1;
		int testing = 0;
		while(testing < NUMBER_OF_VOICES)
		{
			if(harmonyPitches[testing] == pitch) {
				index = testing;
				break;
			} else {
				++testing;
			}
		}
		return index;
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
		while (i < NUMBER_OF_VOICES)
		{
			harmonyPitches[i] = -1;
			++i;
		}
		panningManager.reset();
		stealingManager.clear();
		latchManager.clear();
	};
	
	
	bool areAllVoicesOff() const
	{
		bool allareoff = true;
		
		for (int i = 0; i < NUMBER_OF_VOICES; ++i)
		{
			if(harmonyPitches[i] != -1) {
				allareoff = false;
				break;
			}
		}
		
		return allareoff;
	};
	
	
private:
	int harmonyPitches[NUMBER_OF_VOICES];
	
	MidiPanningManager& panningManager;
	VoiceStealingManager& stealingManager;
	MidiLatchManager& latchManager;
};
