/*
  ==============================================================================

    DspUtils.h
    Created: 20 Dec 2020 1:13:39pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once

#include "GlobalDefinitions.h"


class MidiUtils
{
public:
	
	struct NoteHelpers
	{
		// is MIDI note a black key?
		static bool isMidiNoteBlackKey(const int midipitch)
		{
			if(const int modulo = midipitch % 12; modulo == 1 || modulo == 3 || modulo == 6 || modulo == 8 || modulo == 10)
				return true;
			else
				return false;
		};

		static bool isMidiNoteBlackKey(const float midipitch)
		{
			const int roundedpitch = round(midipitch);
			if(const int modulo = roundedpitch % 12; modulo == 1 || modulo == 3 || modulo == 6 || modulo == 8 || modulo == 10)
				return true;
			else
				return false;
		};
	
		// find midi octave #
		static int getMidiOctaveNumber(const int midiPitch)
		{
			return ceil(midiPitch / 12) - 1;
		};
	
		static int getMidiOctaveNumber(const float midiPitch)
		{
			return ceil(midiPitch / 12) - 1;
		};
	
		// return note name as string
		static std::string getMidiNoteName(const int midipitch, bool useFlats)
		{
			const int pitchclass = ceil(midipitch % 12);
			
			switch(pitchclass)
			{
				case 0: return "C"; break;
				case 1:
					if(useFlats) { return "Db"; break; }
					else { return "C#"; break; }
				case 2: return "D"; break;
				case 3:
					if(useFlats) { return "Eb"; break; }
					else { return "D#"; break; }
				case 4: return "E"; break;
				case 5: return "F"; break;
				case 6:
					if(useFlats) { return "Gb"; break; }
					else { return "F#"; break; }
				case 7: return "G"; break;
				case 8:
					if(useFlats) { return "Ab"; break; }
					else { return "G#"; break; }
				case 9: return "A"; break;
				case 10:
					if(useFlats) { return "Bb"; break; }
					else { return "A#"; break; }
				case 11: return "B"; break;
					
				default: return "error"; break;
			}
		};
	
		static std::string getMidiNoteName(const float midipitch, bool useFlats)
		{
			return getMidiNoteName(int(round(midipitch)), useFlats);
		};
	};
	
	struct PitchConversion
	{
		// midiPitch to frequency conversion
		static float mtof(const float midiNote)
		{
			return 440.0f * std::pow(2.0f, ((midiNote - 69.0f) / 12.0f));
		};

		static int mtof(const int midiNote)
		{
			return round(440 * std::pow(2.0f, ((midiNote - 69) / 12)));
		};

		static float mtof(const float midiNote, const int concertPitchHz, const int rootNote, const int notesPerOctave)
		{
			return concertPitchHz * std::pow(2.0f, ((midiNote - rootNote) / notesPerOctave));
		};


		static int mtof(const int midiNote, const int concertPitchHz, const int rootNote, const int notesPerOctave)
		{
			return round(concertPitchHz * std::pow(2.0f, ((midiNote - rootNote) / notesPerOctave)));
		};
	
	
		// frequency to midiPitch conversion
		static float ftom(const float inputFreq)
		{
			return 12.0f * log2(inputFreq / 440.0f) + 69.0f;
		};

		static int ftom(const int inputFreq)
		{
			return round(12 * log2(inputFreq / 440) + 69);
		};

		static float ftom(const float inputFreq, const int concertPitchHz, const int rootNote, const int notesPerOctave)
		{
			return notesPerOctave * log2(inputFreq / concertPitchHz) + rootNote;
		};

		static int ftom(const int inputFreq, const int concertPitchHz, const int rootNote, const int notesPerOctave)
		{
			return round(notesPerOctave * log2(inputFreq / concertPitchHz) + rootNote);
		};
	};
	
	
	static float getMidifloatFromPitchBend(const int midiPitch, const int pitchbend, const int rangeUp, const int rangeDown)
	{
		if(pitchbend == 64)
			return midiPitch;
		else if (pitchbend > 64)
			return ((rangeUp * (pitchbend - 65)) / 62) + midiPitch;
		else
			return (((1 - rangeDown) * pitchbend) / 63) + midiPitch - rangeDown;
	};
	
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiUtils)
};



class DspUtils
{
public:
	
	static std::pair<float, float> getPanningMultsFromMidiPan(const int midiPan)
	{
		const float Rpan = midiPan / 127.0f;
		return std::make_pair(1.0f - Rpan, Rpan); // L, R
	};
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DspUtils)
};



class MidiPitchConverter
{
public:
	
	MidiPitchConverter(const int initialConcertPitch, const int initialRootNote, const int initialNotesPerOctave):
				concertPitchHz(initialConcertPitch), rootNote(initialRootNote), notesPerOctave(initialNotesPerOctave)
	{ };
	
	
	float mtof(const float midiNote) const // converts midiPitch to frequency in Hz
	{
		return concertPitchHz * std::pow(2.0f, ((midiNote - rootNote) / notesPerOctave));
	};
	
	int mtof(const int midiNote) const // midiPitch to frequency with integers instead of floats
	{
		return round(concertPitchHz * std::pow(2.0f, ((midiNote - rootNote) / notesPerOctave)));
	};
	
	
	float ftom(const float inputFreq) const // converts frequency in Hz to midiPitch (as a float)
	{
		return notesPerOctave * log2(inputFreq / concertPitchHz) + rootNote;
	};
	
	int ftom(const int inputFreq) const // frequency to midiPitch with integers
	{
		return round(notesPerOctave * log2(inputFreq / concertPitchHz) + rootNote);
	};
	
	void setConcertPitchHz(const int newConcertPitch) noexcept { concertPitchHz = newConcertPitch; };
	
	int getCurrentConcertPitchHz() const noexcept { return concertPitchHz; };
	
	void setNotesPerOctave(const int newNPO) noexcept { notesPerOctave = newNPO; };
	
	int getCurrentNotesPerOctave() const noexcept { return notesPerOctave; };
	
	void setRootNote(const int newRoot) noexcept { rootNote = newRoot; };
	
	int getCurrentRootNote() const noexcept { return rootNote; };
	
	
private:
	
	int concertPitchHz; // the frequency in Hz of the root note. Usually 440 in standard Western tuning.
	
	int rootNote; // the midiPitch that corresponds to concertPitchHz. Usually 69 (A4) in Western standard tuning.
	
	int notesPerOctave; // the number of notes per octave. Usually 12 in standard Western tuning.
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiPitchConverter)
	
};


class PitchBendHelper
{
public:
	PitchBendHelper(const int initialStUp, const int initialStDwn):
				rangeUp(initialStUp), rangeDown(initialStDwn), lastRecievedNote(0), lastRecievedPitchbend(64)
	{ };
	
	void setRange(const int newStUp, const int newStDown) noexcept
	{
		rangeUp = newStUp;
		rangeDown = newStDown;
	};
	
	float setRangeAndUpdateLastPitch(const int newStUp, const int newStDown)
	{
		rangeUp = newStUp;
		rangeDown = newStDown;
		return getMidifloat(lastRecievedNote, lastRecievedPitchbend);
	};
	
	void setRangeUp(const int newStUp) noexcept { rangeUp = newStUp; };
	
	float setRangeUpAndUpdateLastPitch(const int newStUp)
	{
		rangeUp = newStUp;
		return getMidifloat(lastRecievedNote, lastRecievedPitchbend);
	};
	
	void setRangeDown(const int newStDown) noexcept { rangeDown = newStDown; };
	
	float setRangeDownAndUpdateLastPitch(const int newStDown)
	{
		rangeDown = newStDown;
		return getMidifloat(lastRecievedNote, lastRecievedPitchbend);
	};
	
	int getCurrentRangeUp() const noexcept { return rangeUp; };
	
	int getCurrentRangeDown() const noexcept { return rangeDown; };
	
	int getLastRecievedNote() const noexcept { return lastRecievedNote; };
	
	int getLastRecievedPitchbend() const noexcept { return lastRecievedPitchbend; };
	
	float newNoteRecieved(const int newMidiPitch)
	{
		lastRecievedNote = newMidiPitch;
		return getMidifloat(newMidiPitch, lastRecievedPitchbend);
	};
	
	float newPitchbendRecieved(const int newPitchbend)
	{
		lastRecievedPitchbend = newPitchbend;
		return getMidifloat(lastRecievedNote, newPitchbend);
	};
	
	float newPitchbendAndNoteRecieved(const int newMidiPitch, const int newPitchbend)
	{
		lastRecievedNote = newMidiPitch;
		lastRecievedPitchbend = newPitchbend;
		return getMidifloat(newMidiPitch, newPitchbend);
	};
	
	void updatePitchbendWithNoReturn(const int newPitchbend) noexcept { lastRecievedPitchbend = newPitchbend; };
	
	void updatePitchWithNoReturn(const int newPitch) noexcept { lastRecievedNote = newPitch; };
	
	
private:
	
	int rangeUp, rangeDown;
	
	int lastRecievedNote, lastRecievedPitchbend;
	
	
	float getMidifloat(const int midiPitch, const int pitchbend)
	{
		if(pitchbend == 64)
			return midiPitch;
		else if (pitchbend > 64)
			return ((rangeUp * (pitchbend - 65)) / 62) + midiPitch;
		else
			return (((1 - rangeDown) * pitchbend) / 63) + midiPitch - rangeDown;
	};
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchBendHelper)
};


class Panner
{
public:
	
	Panner(): lastRecievedMidiPan(64), lastRpan(0.5f), lastLpan(0.5f)
	{ };
	
	float getRpan() const noexcept { return lastRpan; };
	
	float getLpan() const noexcept { return lastLpan; };
	
	std::pair<float, float> getLRpan() const noexcept { return std::make_pair(lastLpan, lastRpan); };
	
	int getLastMidipan() const noexcept { return lastRecievedMidiPan; };
	
	void updatePanWithNoReturn(const int newMidipan)
	{
		lastRecievedMidiPan = newMidipan;
		lastRpan = newMidipan / 127.0f;
		lastLpan = 1.0f - lastRpan;
	};
	
	std::pair<float, float> updatePan(const int newPan)
	{
		lastRecievedMidiPan = newPan;
		lastRpan = newPan / 127.0f;
		lastLpan = 1.0f - lastRpan;
		return std::make_pair(lastLpan, lastRpan);
	};
	
	void updatePan(const int newPan, int (&array)[2])
	{
		lastRecievedMidiPan = newPan;
		lastRpan = newPan / 127.0f;
		lastLpan = 1.0f - lastRpan;
		array[0] = lastLpan;
		array[1] = lastRpan;
	};
	
	void putMultipliersInArray(int (&array)[2])
	{
		array[0] = lastLpan;
		array[1] = lastRpan;
	};
	
	
private:
	
	int lastRecievedMidiPan;
	
	float lastRpan, lastLpan;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Panner)
};
