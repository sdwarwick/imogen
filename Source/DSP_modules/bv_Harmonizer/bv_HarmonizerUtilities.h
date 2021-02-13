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
    T mtof (T midiNote) const
    {
        midiNote = jlimit (T(0.0), T(127.0), midiNote);
        return concertPitchHz.load() * std::pow(T(2.0), ((midiNote - rootNote.load()) / notesPerOctave.load()));
    }
    
    // converts frequency in Hz to midipitch
    template<typename T>
    T ftom (const T inputFreq) const
    {
        jassert (inputFreq >= 0);
        return notesPerOctave.load() * log2(inputFreq / concertPitchHz.load()) + rootNote.load();
    }
    
    void setConcertPitchHz (const int newConcertPitch) noexcept
    {
        if (newConcertPitch <= 0)
            return;
        
        concertPitchHz.store (newConcertPitch);
    }
    
    int getCurrentConcertPitchHz() const noexcept { return concertPitchHz.load(); }
    
    void setNotesPerOctave (const int newNPO) noexcept
    {
        if (newNPO <= 0)
            return;
        
        notesPerOctave.store (newNPO);
    }
    
    int getCurrentNotesPerOctave() const noexcept { return notesPerOctave.load(); }
    
    void setRootNote (const int newRoot) noexcept
    {
        if (newRoot <= 0)
            return;
        
        rootNote.store (newRoot);
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
        rangeUp.store (newStUp);
        rangeDown.store (newStDown);
    }
    
    int getCurrentRangeUp()   const noexcept { return rangeUp.load(); }
    int getCurrentRangeDown() const noexcept { return rangeDown.load(); }
    
    int getLastRecievedPitchbend() const noexcept { return lastRecievedPitchbend.load(); }
    
    float newNoteRecieved (const int newMidiPitch) const
    {
        return getMidifloat (jlimit (0, 127, newMidiPitch),
                             lastRecievedPitchbend.load());
    }
    
    void newPitchbendRecieved (const int newPitchbend) noexcept
    {
        lastRecievedPitchbend.store (jlimit (0, 127, newPitchbend));
    }
    
    
private:
    
    std::atomic<int> rangeUp, rangeDown, lastRecievedPitchbend;
    
    float getMidifloat (const int midiPitch, const int pitchbend) const
    {
        jassert (isPositiveAndBelow(midiPitch, 128) && isPositiveAndBelow(pitchbend, 128));
        
        if (pitchbend == 64)
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
    
    void setSensitivity (int newSensitivity) noexcept
    {
        newSensitivity = jlimit (0, 100, newSensitivity);
        sensitivity.store (newSensitivity / 100.0f);
    }
    
    void setFloatSensitivity (const float newSensitivity) noexcept
    {
        sensitivity.store (jlimit (0.0f, 1.0f, newSensitivity));
    }
    
    float getCurrentSensitivity() const noexcept { return sensitivity.load(); }
    
    float intVelocity (int midiVelocity)
    {
        midiVelocity = jlimit (0, 127, midiVelocity);
        return getGainMult (midiVelocity / 127.0f, sensitivity.load());
    }
    
    float floatVelocity (const float floatVelocity) const
    {
        return getGainMult (jlimit (0.0f, 1.0f, floatVelocity),
                            sensitivity.load());
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
