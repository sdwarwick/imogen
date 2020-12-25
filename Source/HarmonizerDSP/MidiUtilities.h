/*
  ==============================================================================

    MidiUtilities.h
    Created: 24 Dec 2020 5:39:46pm
    Author:  Ben Vining

  ==============================================================================
*/

#pragma once



class PitchConverter
{
public:
	
	PitchConverter(const int initialConcertPitch, const int initialRootNote, const int initialNotesPerOctave):
	concertPitchHz(initialConcertPitch), rootNote(initialRootNote), notesPerOctave(initialNotesPerOctave)
	{ };
	
	
	float mtof(const float midiNote) const // converts midiPitch to frequency in Hz
	{
		jassert(midiNote >= 0.0f && midiNote <= 127.0f);
		return concertPitchHz * std::pow(2.0f, ((midiNote - rootNote) / notesPerOctave));
	};
	
	int mtof(const int midiNote) const // midiPitch to frequency with integers instead of floats
	{
		jassert(isPositiveAndBelow(midiNote, 128));
		return round(concertPitchHz * std::pow(2.0f, ((midiNote - rootNote) / notesPerOctave)));
	};
	
	
	float ftom(const float inputFreq) const // converts frequency in Hz to midiPitch (as a float)
	{
		jassert(inputFreq >= 0);
		return notesPerOctave * log2(inputFreq / concertPitchHz) + rootNote;
	};
	
	int ftom(const int inputFreq) const // frequency to midiPitch with integers
	{
		jassert(inputFreq >= 0);
		return round(notesPerOctave * log2(inputFreq / concertPitchHz) + rootNote);
	};
	
	void setConcertPitchHz(const int newConcertPitch) noexcept
	{
		jassert(newConcertPitch >= 0);
		concertPitchHz = newConcertPitch;
	};
	
	int getCurrentConcertPitchHz() const noexcept { return concertPitchHz; };
	
	void setNotesPerOctave(const int newNPO) noexcept
	{
		jassert(newNPO > 0);
		notesPerOctave = newNPO;
	};
	
	int getCurrentNotesPerOctave() const noexcept { return notesPerOctave; };
	
	void setRootNote(const int newRoot) noexcept
	{
		jassert(newRoot >= 0);
		rootNote = newRoot;
	};
	
	int getCurrentRootNote() const noexcept { return rootNote; };
	
	
private:
	
	int concertPitchHz; // the frequency in Hz of the root note. Usually 440 in standard Western tuning.
	
	int rootNote; // the midiPitch that corresponds to concertPitchHz. Usually 69 (A4) in Western standard tuning.
	
	int notesPerOctave; // the number of notes per octave. Usually 12 in standard Western tuning.
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchConverter)
	
};


class PitchBendHelper
{
public:
	PitchBendHelper(const int initialStUp, const int initialStDwn):
	rangeUp(initialStUp), rangeDown(initialStDwn), lastRecievedNote(0), lastRecievedPitchbend(64)
	{ };
	
	void setRange(const int newStUp, const int newStDown) noexcept
	{
		jassert(newStUp >= 0 && newStDown >= 0);
		const ScopedLock sl (lock);
		rangeUp = newStUp;
		rangeDown = newStDown;
	};
	
	float setRangeAndUpdateLastPitch(const int newStUp, const int newStDown)
	{
		jassert(newStUp >= 0 && newStDown >= 0);
		const ScopedLock sl (lock);
		rangeUp = newStUp;
		rangeDown = newStDown;
		return getMidifloat(lastRecievedNote, lastRecievedPitchbend);
	};
	
	void setRangeUp(const int newStUp) noexcept
	{
		jassert(newStUp >= 0);
		rangeUp = newStUp;
	};
	
	float setRangeUpAndUpdateLastPitch(const int newStUp)
	{
		jassert(newStUp >= 0);
		const ScopedLock sl (lock);
		rangeUp = newStUp;
		return getMidifloat(lastRecievedNote, lastRecievedPitchbend);
	};
	
	void setRangeDown(const int newStDown) noexcept
	{
		jassert(newStDown >= 0);
		rangeDown = newStDown;
	};
	
	float setRangeDownAndUpdateLastPitch(const int newStDown)
	{
		jassert(newStDown >= 0);
		const ScopedLock sl (lock);
		rangeDown = newStDown;
		return getMidifloat(lastRecievedNote, lastRecievedPitchbend);
	};
	
	int getCurrentRangeUp() const noexcept { return rangeUp; };
	
	int getCurrentRangeDown() const noexcept { return rangeDown; };
	
	int getLastRecievedNote() const noexcept { return lastRecievedNote; };
	
	int getLastRecievedPitchbend() const noexcept { return lastRecievedPitchbend; };
	
	float newNoteRecieved(const int newMidiPitch)
	{
		jassert(isPositiveAndBelow(newMidiPitch, 128));
		const ScopedLock sl (lock);
		lastRecievedNote = newMidiPitch;
		return getMidifloat(newMidiPitch, lastRecievedPitchbend);
	};
	
	float newPitchbendRecieved(const int newPitchbend)
	{
		jassert(isPositiveAndBelow(newPitchbend, 128));
		const ScopedLock sl (lock);
		lastRecievedPitchbend = newPitchbend;
		return getMidifloat(lastRecievedNote, newPitchbend);
	};
	
	float newPitchbendAndNoteRecieved(const int newMidiPitch, const int newPitchbend)
	{
		jassert(isPositiveAndBelow(newMidiPitch, 128) && isPositiveAndBelow(newPitchbend, 128));
		const ScopedLock sl (lock);
		lastRecievedNote = newMidiPitch;
		lastRecievedPitchbend = newPitchbend;
		return getMidifloat(newMidiPitch, newPitchbend);
	};
	
	void updatePitchbendWithNoReturn(const int newPitchbend) noexcept
	{
		jassert(isPositiveAndBelow(newPitchbend, 128));
		lastRecievedPitchbend = newPitchbend;
	};
	
	void updatePitchWithNoReturn(const int newPitch) noexcept
	{
		jassert(isPositiveAndBelow(newPitch, 128));
		lastRecievedNote = newPitch;
	};
	
	
private:
	
	CriticalSection lock;
	
	int rangeUp, rangeDown;
	
	int lastRecievedNote, lastRecievedPitchbend;
	
	
	float getMidifloat(const int midiPitch, const int pitchbend)
	{
		jassert(isPositiveAndBelow(midiPitch, 128) && isPositiveAndBelow(pitchbend, 128));
		if(pitchbend == 64)
			return midiPitch;
		else if (pitchbend > 64)
			return ((rangeUp * (pitchbend - 65)) / 62) + midiPitch;
		else
			return (((1 - rangeDown) * pitchbend) / 63) + midiPitch - rangeDown;
	};
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchBendHelper)
};


class VelocityHelper
{
public:
	VelocityHelper(const int initialSensitivity):
			sensitivity(initialSensitivity/100.0f), lastRecievedFloatVelocity(0.0f), lastSentFloatVelocity(0.0f)
	{ };
	
	void setSensitivity(const int newSensitivity) noexcept
	{
		jassert(isPositiveAndBelow(newSensitivity, 101));
		sensitivity = newSensitivity / 100.0f;
	};
	
	void setFloatSensitivity(const float newSensitivity) noexcept
	{
		jassert(newSensitivity >= 0.0f && newSensitivity <= 1.0f);
		sensitivity = newSensitivity;
	};
	
	float getCurrentSensitivity() const noexcept { return sensitivity; };
	
	float intVelocity(const int midiVelocity)
	{
		jassert(isPositiveAndBelow(midiVelocity, 128));
		const ScopedLock sl(lock);
		const float velocity = midiVelocity / 127.0f;
		lastRecievedFloatVelocity = velocity;
		lastSentFloatVelocity = getGainMult(velocity, sensitivity);
		return lastSentFloatVelocity;
	};
	
	float floatVelocity(const float floatVelocity)
	{
		jassert(floatVelocity >= 0.0f && floatVelocity <= 1.0f);
		const ScopedLock sl(lock);
		lastRecievedFloatVelocity = floatVelocity;
		lastSentFloatVelocity = getGainMult(floatVelocity, sensitivity);
		return lastSentFloatVelocity;
	};
	
	float setSensitivityWithReturn(const int newSensitivity)
	{
		jassert(isPositiveAndBelow(newSensitivity, 101));
		const ScopedLock sl(lock);
		sensitivity = newSensitivity / 100.0f;
		lastSentFloatVelocity = getGainMult(lastRecievedFloatVelocity, sensitivity);
		return lastSentFloatVelocity;
	};
	
	float setFloatSensitivityWithReturn(const float newSensitivity)
	{
		jassert(newSensitivity >= 0.0f && newSensitivity <= 1.0f);
		const ScopedLock sl(lock);
		sensitivity = newSensitivity;
		lastSentFloatVelocity = getGainMult(lastRecievedFloatVelocity, newSensitivity);
		return lastSentFloatVelocity;
	};
	
	float getLastFloatVelocity() const noexcept { return lastSentFloatVelocity; };
	
	int getLastVelocity() const noexcept
	{
		const ScopedLock sl(lock);
		int velocity = round(lastSentFloatVelocity * 127.0f);
		if (velocity > 127) { velocity = 127; };
		if (velocity < 0) { velocity = 0; };
		return velocity;
	};
	
	
private:
	CriticalSection lock;
	
	float sensitivity;
	
	float lastRecievedFloatVelocity;
	float lastSentFloatVelocity;
	
	float getGainMult(const float floatVelocity, const float floatSensitivity)
	{
		return ((1.0f - floatVelocity) * (1.0f - floatSensitivity) + floatVelocity);
	};
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VelocityHelper)
};


class PanningHelper
{
public:
	
	PanningHelper(): lastRecievedMidiPan(64), lastRpan(0.5f), lastLpan(0.5f)
	{ };
	
	float getRpan() const noexcept { return lastRpan; };
	
	float getLpan() const noexcept { return lastLpan; };
	
	std::pair<float, float> getLRpan() const noexcept { return std::make_pair(lastLpan, lastRpan); };
	
	int getLastMidipan() const noexcept { return lastRecievedMidiPan; };
	
	void updatePanWithNoReturn(const int newMidipan)
	{
		jassert(isPositiveAndBelow(newMidipan, 128));
		const ScopedLock sl(lock);
		lastRecievedMidiPan = newMidipan;
		lastRpan = newMidipan / 127.0f;
		lastLpan = 1.0f - lastRpan;
	};
	
	std::pair<float, float> updatePan(const int newPan)
	{
		jassert(isPositiveAndBelow(newPan, 128));
		const ScopedLock sl(lock);
		lastRecievedMidiPan = newPan;
		lastRpan = newPan / 127.0f;
		lastLpan = 1.0f - lastRpan;
		return std::make_pair(lastLpan, lastRpan);
	};
	
	void updatePan(const int newPan, int (&array)[2])
	{
		jassert(isPositiveAndBelow(newPan, 128));
		const ScopedLock sl(lock);
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
	CriticalSection lock;
	
	int lastRecievedMidiPan;
	
	float lastRpan, lastLpan;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PanningHelper)
};
