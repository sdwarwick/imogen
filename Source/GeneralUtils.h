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
        // is midi note a black key?
        template <typename T>
        static bool isMidiNoteBlackKey(const T midipitch)
        {
            jassert(midipitch >= 0);
            const int modulo = (roundToInt(midipitch)) % 12;
            if(modulo == 1 || modulo == 3 || modulo == 6 || modulo == 8 || modulo == 10)
                return true;
            return false;
        };
        
        // are two midi notes the same pitch class?
        template<typename T1, typename T2>
        static bool areSamePitchClass(const T1 pitch1, const T2 pitch2)
        {
            jassert(pitch1 >= 0 && pitch2 >= 0);
            return ((roundToInt(pitch1)) % 12) == ((roundToInt(pitch2)) % 12);
        };
   };
    
    struct PitchConversion
    {
        // MIDI TO FREQUENCY
        template<typename T>
        static T mtof(const T midiNote)
        {
            jassert(midiNote >= 0 && midiNote <= 127);
            return 440.0 * std::pow(2.0, ((midiNote - 69.0) / 12.0));
        };
        
        template<typename T1, typename T2, typename T3>
        static T1 mtof(const T1 midiNote, const T2 concertPitchHz, const T3 rootNote, const int notesPerOctave)
        {
            jassert(midiNote >= 0 && midiNote <= 127 && concertPitchHz >= 0 && rootNote >= 0 && notesPerOctave > 0);
            return concertPitchHz * std::pow(2.0, ((midiNote - rootNote) / notesPerOctave));
        };
        
        // FREQUENCY TO MIDI
        template<typename T>
        static T ftom(const T inputFreq)
        {
            jassert(inputFreq >= 0);
            return 12.0 * log2(inputFreq / 440.0) + 69.0;
        };
        
        template<typename T1, typename T2, typename T3>
        static T1 ftom(const T1 inputFreq, const T2 concertPitchHz, const T3 rootNote, const int notesPerOctave)
        {
            jassert(inputFreq >= 0 && concertPitchHz >= 0 && rootNote >= 0 && notesPerOctave > 0);
            return notesPerOctave * log2(inputFreq / concertPitchHz) + rootNote;
        };
    };
    
    template<typename T1, typename T2, typename T3, typename T4>
    static float getMidifloatFromPitchBend(const T1 midiPitch, const T2 pitchbend, const T3 rangeUp, const T4 rangeDown)
    {
        jassert(midiPitch >= 0 && midiPitch <= 127 && pitchbend >= 0 && pitchbend <= 127 && rangeUp >= 0 && rangeDown >= 0);
        if(pitchbend == 64)
            return float(midiPitch);
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
        jassert(isPositiveAndBelow (midiPan, 128));
        
        // convert midiPan [0-127] first to an angle between 0 & 90 degrees, then to radians
        const float panningAngle = 90 * midiPan / 127 * MathConstants<float>::pi / 180;
        
        float left  = std::sin (panningAngle);
        float right = std::cos (panningAngle);
        
        if (left < 0.0f)  left = 0.0f;
        if (left > 1.0f)  left = 1.0f;
        if (right < 0.0f) right = 0.0f;
        if (right > 1.0f) right = 1.0f;
        
        return std::make_pair(left, right); // L, R
    };
    
    
    static void putPanningMultsFromMidiPanInArray(const int midiPan, int (&array)[2])
    {
        jassert(isPositiveAndBelow(midiPan, 128));
        
        // convert midiPan [0-127] first to an angle between 0 & 90 degrees, then to radians
        const float panningAngle = 90 * midiPan / 127 * MathConstants<float>::pi / 180;
        
        float left  = std::sin (panningAngle);
        float right = std::cos (panningAngle);
        
        if (left < 0.0f)  left = 0.0f;
        if (left > 1.0f)  left = 1.0f;
        if (right < 0.0f) right = 0.0f;
        if (right > 1.0f) right = 1.0f;
        
        array[0] = left;
        array[1] = right;
    };
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DspUtils)
};
