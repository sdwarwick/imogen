/*
  ==============================================================================

    VoiceStealingManager.h
    Created: 10 Nov 2020 7:36:24pm
    Author:  Ben Vining
 
 Helper class to keep track of voice #s that are active, so that it can return the voice # that has been active the LONGEST in order to "steal" that voice 

  ==============================================================================
*/

#pragma once


class VoiceStealingManager
{
	
public:
	
	VoiceStealingManager() {
		sentVoiceNumbers.clear();
	};
	
	void addSentVoice(const int sentVoiceNum) {
		sentVoiceNumbers.push_back(sentVoiceNum);
	};
	
	
	void removeSentVoice(const int voiceNumRemoving) {
		// removes voiceNumRemoving from the list of sent voice #s, regardless of where it is in the list, then moves all list elements together so there are no empty spaces
		for (auto it = sentVoiceNumbers.begin(); it != sentVoiceNumbers.end(); ) {
			if (*it == voiceNumRemoving) {
				it = sentVoiceNumbers.erase(it);
				break;
			} else {
				++it;
			}
		}
	};
	
	
	int voiceToSteal() {
		// returns the first element in the list of sent voice #s, then removes that element from the list and moves all other elements up 1 index
		const int voiceNumberToSteal = sentVoiceNumbers.front();
		return voiceNumberToSteal;
		sentVoiceNumbers.erase(sentVoiceNumbers.begin());
	};
	
	
	void clear() {
		sentVoiceNumbers.clear();
	};
	
	
private:
	std::vector <int> sentVoiceNumbers;
};
