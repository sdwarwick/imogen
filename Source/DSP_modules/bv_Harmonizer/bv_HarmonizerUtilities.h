/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: PitchConverter, PitchBendHelper, VelocityHelper
*/


#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{
    
using namespace juce;
    

class PitchConverter
{
public:
    
    PitchConverter(const int initialConcertPitch, const int initialRootNote, const int initialNotesPerOctave):
        concertPitchHz(initialConcertPitch), rootNote(initialRootNote), notesPerOctave(initialNotesPerOctave)
    { }
    
    // converts midi pitch to frequency in Hz
    template<typename T>
    T mtof(const T midiNote) const
    {
        jassert(midiNote >= 0 && midiNote <= 127);
        return concertPitchHz.load() * std::pow(T(2.0), ((midiNote - rootNote.load()) / notesPerOctave.load()));
    }
    
    // converts frequency in Hz to midipitch
    template<typename T>
    T ftom(const T inputFreq) const
    {
        jassert(inputFreq >= 0);
        return notesPerOctave.load() * log2(inputFreq / concertPitchHz.load()) + rootNote.load();
    }
    
    void setConcertPitchHz(const int newConcertPitch) noexcept
    {
        jassert(newConcertPitch >= 0);
        concertPitchHz.store(newConcertPitch);
    }
    
    int getCurrentConcertPitchHz() const noexcept { return concertPitchHz.load(); }
    
    void setNotesPerOctave(const int newNPO) noexcept
    {
        jassert(newNPO > 0);
        notesPerOctave.store(newNPO);
    }
    
    int getCurrentNotesPerOctave() const noexcept { return notesPerOctave.load(); }
    
    void setRootNote(const int newRoot) noexcept
    {
        jassert(newRoot >= 0);
        rootNote.store(newRoot);
    }
    
    int getCurrentRootNote() const noexcept { return rootNote.load(); }
    
    
private:
    
    std::atomic<int> concertPitchHz; // the frequency in Hz of the root note. Usually 440 in standard Western tuning.
    
    std::atomic<int> rootNote; // the midiPitch that corresponds to concertPitchHz. Usually 69 (A4) in Western standard tuning.
    
    std::atomic<int> notesPerOctave; // the number of notes per octave. Usually 12 in standard Western tuning.
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchConverter)
    
};



class PitchBendHelper
{
    
public:
    PitchBendHelper(const int initialStUp, const int initialStDwn):
        rangeUp(initialStUp), rangeDown(initialStDwn), lastRecievedPitchbend(64)
    { }
    
    void setRange (const int newStUp, const int newStDown) noexcept
    {
        jassert(newStUp >= 0 && newStDown >= 0);
        rangeUp.store(newStUp);
        rangeDown.store(newStDown);
    }
    
    int getCurrentRangeUp()        const noexcept { return rangeUp.load(); }
    
    int getCurrentRangeDown()      const noexcept { return rangeDown.load(); }
    
    int getLastRecievedPitchbend() const noexcept { return lastRecievedPitchbend.load(); }
    
    float newNoteRecieved (const int newMidiPitch) const
    {
        jassert(isPositiveAndBelow(newMidiPitch, 128));
        return getMidifloat (newMidiPitch, lastRecievedPitchbend.load());
    }
    
    void newPitchbendRecieved (const int newPitchbend) noexcept
    {
        jassert(isPositiveAndBelow(newPitchbend, 128));
        lastRecievedPitchbend.store(newPitchbend);
    }
    
    
private:
    
    std::atomic<int> rangeUp, rangeDown, lastRecievedPitchbend;
    
    float getMidifloat (const int midiPitch, const int pitchbend) const
    {
        jassert(isPositiveAndBelow(midiPitch, 128) && isPositiveAndBelow(pitchbend, 128));
        
        if(pitchbend == 64)
            return midiPitch;
        
        if (pitchbend > 64)
            return ((rangeUp.load() * (pitchbend - 65)) / 62) + midiPitch;
        
        const int currentdownrange = rangeDown.load();
        return (((1 - currentdownrange) * pitchbend) / 63) + midiPitch - currentdownrange;
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchBendHelper)
};



class VelocityHelper
{
public:
    VelocityHelper(const int initialSensitivity): sensitivity(initialSensitivity/100.0f)
    { }
    
    void setSensitivity(const int newSensitivity) noexcept
    {
        jassert(isPositiveAndBelow(newSensitivity, 101));
        sensitivity.store(newSensitivity / 100.0f);
    }
    
    void setFloatSensitivity(const float newSensitivity) noexcept
    {
        jassert(newSensitivity >= 0.0f && newSensitivity <= 1.0f);
        sensitivity.store(newSensitivity);
    }
    
    float getCurrentSensitivity() const noexcept { return sensitivity.load(); }
    
    float intVelocity(const int midiVelocity)
    {
        jassert(isPositiveAndBelow(midiVelocity, 128));
        return getGainMult (midiVelocity / 127.0f, sensitivity.load());
    }
    
    float floatVelocity(const float floatVelocity) const
    {
        jassert(floatVelocity >= 0.0f && floatVelocity <= 1.0f);
        return getGainMult (floatVelocity, sensitivity.load());
    }
    
    
private:
    
    std::atomic<float> sensitivity;
    
    float getGainMult (const float floatVelocity, const float floatSensitivity) const
    {
        return ((1.0f - floatVelocity) * (1.0f - floatSensitivity) + floatVelocity);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VelocityHelper)
};


}; // namespace
