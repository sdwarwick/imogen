/*
  ==============================================================================

    VoiceStealingManager.h
    Created: 10 Nov 2020 7:36:24pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once


class VoiceStealingManager
{
	
public:
	
	void addSentVoice(const int sentVoiceNum) {
		// appends sentVoiceNum to end of list of sent voice #s
	};
	
	
	void removeSentVoice(const int voiceNumRemoving) {
		// removes voiceNumRemoving from the list of sent voice #s, regardless of where it is in the list, then moves all list elements together so there are no empty spaces 
	};
	
	
	int voiceToSteal() {
		
		// returns the first element in the list of sent voice #s, then removes that element from the list and moves all other elements up 1 index
		
		int voiceNumberToSteal = 0;
		
		return voiceNumberToSteal;
	};
	
	
private:
	
};
