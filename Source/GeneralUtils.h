/*
 ==============================================================================
 
 DspUtils.h
 Created: 20 Dec 2020 1:13:39pm
 Author:  Ben Vining
 
 ==============================================================================
 */

#pragma once


class MidiUtils
{
public:
    
    struct NoteHelpers
    {
        // is MIDI note a black key?
        static bool isMidiNoteBlackKey(const int midipitch)
        {
            jassert(midipitch >= 0);
            const int modulo = midipitch % 12;
            if(modulo == 1 || modulo == 3 || modulo == 6 || modulo == 8 || modulo == 10)
                return true;
            
            return false;
        };
        
        static bool isMidiNoteBlackKey(const float midipitch)
        {
            jassert(midipitch >= 0.0f);
            const int modulo = (roundToInt(midipitch)) % 12;
            if(modulo == 1 || modulo == 3 || modulo == 6 || modulo == 8 || modulo == 10)
                return true;
            
            return false;
        };
        
        // determine if two midiPitches in any octave are the same pitch class
        static bool areSamePitchClass(const int pitch1, const int pitch2)
        {
            jassert(pitch1 >= 0 && pitch2 >= 0);
            return (pitch1 % 12 == pitch2 % 12);
        };
        
        static bool areSamePitchClass(const float pitch1, const int pitch2)
        {
            jassert(pitch1 >= 0.0f && pitch2 >= 0);
            return (int(round(pitch1)) % 12 == pitch2 % 12);
        };
        
        static bool areSamePitchClass(const int pitch1, const float pitch2)
        {
            jassert(pitch1 >= 0 && pitch2 >= 0.0f);
            return (pitch1 % 12 == int(round(pitch2)) % 12);
        };
        
        static bool areSamePitchClass(const float pitch1, const float pitch2)
        {
            jassert(pitch1 >= 0.0f && pitch2 >= 0.0f);
            return (int(round(pitch1)) % 12 == int(round(pitch2)) % 12);
        };
        
    };
    
    struct PitchConversion
    {
        // midiPitch to frequency conversion
        static float mtof(const float midiNote)
        {
            jassert(midiNote >= 0.0f && midiNote <= 127.0f);
            return 440.0f * std::pow(2.0f, ((midiNote - 69.0f) / 12.0f));
        };
        
        static int mtof(const int midiNote)
        {
            jassert(isPositiveAndBelow(midiNote, 128));
            return round(440 * std::pow(2.0f, ((midiNote - 69) / 12)));
        };
        
        static float mtof(const float midiNote, const int concertPitchHz, const int rootNote, const int notesPerOctave)
        {
            jassert(midiNote >= 0.0f && midiNote <= 127.0f && concertPitchHz >= 0 && rootNote >= 0 && notesPerOctave > 0);
            return concertPitchHz * std::pow(2.0f, ((midiNote - rootNote) / notesPerOctave));
        };
        
        
        static int mtof(const int midiNote, const int concertPitchHz, const int rootNote, const int notesPerOctave)
        {
            jassert(isPositiveAndBelow(midiNote, 128) && concertPitchHz >= 0 && rootNote >= 0 && notesPerOctave > 0);
            return round(concertPitchHz * std::pow(2.0f, ((midiNote - rootNote) / notesPerOctave)));
        };
        
        
        // frequency to midiPitch conversion
        static float ftom(const float inputFreq)
        {
            jassert(inputFreq >= 0.0f);
            return 12.0f * log2(inputFreq / 440.0f) + 69.0f;
        };
        
        static int ftom(const int inputFreq)
        {
            jassert(inputFreq >= 0);
            return round(12 * log2(inputFreq / 440) + 69);
        };
        
        static int ftom(const int inputFreq, const int concertPitchHz, const int rootNote, const int notesPerOctave)
        {
            jassert(inputFreq >= 0 && concertPitchHz >= 0 && rootNote >= 0 && notesPerOctave > 0);
            return round(notesPerOctave * log2(inputFreq / concertPitchHz) + rootNote);
        };
    };
    
    static float getMidifloatFromPitchBend(const int midiPitch, const int pitchbend, const int rangeUp, const int rangeDown)
    {
        jassert(isPositiveAndBelow(midiPitch, 128) && isPositiveAndBelow(pitchbend, 128) && rangeUp >= 0 && rangeDown >= 0);
        if(pitchbend == 64)
            return midiPitch;
        
        if (pitchbend > 64)
            return ((rangeUp * (pitchbend - 65)) / 62) + midiPitch;
        
        return (((1 - rangeDown) * pitchbend) / 63) + midiPitch - rangeDown;
    };
    
    struct velocityHelpers
    {
        static float getGainMultFromVelocity(const int midiVelocity)
        {
            jassert(isPositiveAndBelow(midiVelocity, 128));
            return midiVelocity / 127.0f;
        };
        
        static float getGainMultFromVelocityWithSensitivity(const int midiVelocity, const int velocitySensitivity)
        {
            jassert(isPositiveAndBelow(midiVelocity, 128) && isPositiveAndBelow(velocitySensitivity, 101));
            const float velocity = midiVelocity / 127.0f;
            return ((1.0f - velocity) * (1.0f - (velocitySensitivity / 100.0f)) + velocity);
        };
        
        static float getGainMultFromFloatVelocityWithSensitivity(const float floatVelocity, const int velocitySensitivity)
        {
            jassert(floatVelocity >= 0.0f && floatVelocity <= 1.0f && isPositiveAndBelow(velocitySensitivity, 101));
            return ((1.0f - floatVelocity) * (1.0f - (velocitySensitivity / 100.0f)) + floatVelocity);
        };
    };
    
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiUtils)
};



class DspUtils
{
public:
    
    static std::pair<float, float> getPanningMultsFromMidiPan(const int midiPan)
    {
        jassert(isPositiveAndBelow(midiPan, 128));
        const float Rpan = midiPan / 127.0f;
        return std::make_pair(1.0f - Rpan, Rpan); // L, R
    };
    
    static void putPanningMultsFromMidiPanInArray(const int midiPan, int (&array)[2])
    {
        jassert(isPositiveAndBelow(midiPan, 128));
        const float Rpan = midiPan / 127.0f;
        array[0] = 1.0f - Rpan;
        array[1] = Rpan;
    };
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DspUtils)
};
