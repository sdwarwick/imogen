/*
  ==============================================================================

    VoiceStealingManager.h
    Created: 10 Nov 2020 7:36:24pm
    Author:  Ben Vining
 
 		Helper class to keep track of voice #s that are active, so that it can return the voice # that has been active the LONGEST in order to "steal" that voice if all voices are on and a new note on comes in and VOICE STEALING is ON

  ==============================================================================
*/

#pragma once

#ifndef NUMBER_OF_VOICES
#define NUMBER_OF_VOICES 12
#endif


class VoiceStealingManager
{
	
public:
	
	VoiceStealingManager() {
		sentVoiceNumbers.ensureStorageAllocated(NUMBER_OF_VOICES);
		sentVoiceNumbers.clearQuick();
	};
	
	
	void addSentVoice(const int sentVoiceNum) {
		if(sentVoiceNumbers.contains(sentVoiceNum) != true) {
			sentVoiceNumbers.add(sentVoiceNum);
		}
	};
	
	
	void removeSentVoice(const int voiceNumRemoving) {
		// removes voiceNumRemoving from the list of sent voice #s, regardless of where it is in the list, then moves all list elements together so there are no empty spaces
		sentVoiceNumbers.removeFirstMatchingValue(voiceNumRemoving);
	};
	
	
	int voiceToSteal() {
		// returns the first element in the list of sent voice #s, then removes that element from the list and moves all other elements up 1 index
		if(sentVoiceNumbers.isEmpty() != true) {
			return sentVoiceNumbers.getUnchecked(0);
			sentVoiceNumbers.remove(0);
		} else {
			return -1;
		}
	};
	
	
	void clear() {
		sentVoiceNumbers.clearQuick();
	};
	
	
private:

	Array<int> sentVoiceNumbers;
};
