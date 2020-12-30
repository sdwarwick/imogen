/*
  ==============================================================================

    alternateInputModes.h
    Created: 29 Dec 2020 10:24:47pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once


class ChordHolder
{
public:
	
	ChordHolder() { midiNoteMappings.fill(-1); };
	~ChordHolder() { };
	
	
	Array<int> returnChord(const int index)
	{
		return { };
	};
	
	
	int indexMappedToMidiNote(const int midiNote)
	{
		return midiNoteMappings[midiNote - 1];
	};
	
	
	void saveChord(Array<int>& chordNotes, const int index)
	{
		ignoreUnused(chordNotes);
		ignoreUnused(index);
	};
	
	
	void updateIndexMappings(const int index, const int newMidiNoteMapping)
	{
		midiNoteMappings[newMidiNoteMapping - 1] = index;
	};
	
	
private:
	
	std::array<int, 127> midiNoteMappings; // in this array - index is midi note #, stored value is mapped chord index #
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordHolder)
};






class IntervalHolder
{
public:
	
	IntervalHolder() { midiNoteMappings.fill(-1); };
	~IntervalHolder() { };
	
	
	Array<int> returnIntervalSet(const int index)
	{
		return { };
	};
	
	
	int indexMappedToMidiNote(const int midiNote)
	{
		return midiNoteMappings[midiNote - 1];
	};
	
	
	void saveIntervalSet(Array<int>& desiredIntervals, const int index)
	{
		ignoreUnused(desiredIntervals);
		ignoreUnused(index);
	};
	
	
	void updateIndexMappings(const int index, const int newMidiNoteMapping)
	{
		midiNoteMappings[newMidiNoteMapping - 1] = index;
	};
	
	
private:
	
	std::array<int, 127> midiNoteMappings;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntervalHolder)
};
