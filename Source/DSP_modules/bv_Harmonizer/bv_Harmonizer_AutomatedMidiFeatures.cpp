/*
 Part of module: bv_Harmonizer
 Direct parent file: bv_Harmonizer.h
 Classes: Harmonizer
 */

#include "bv_Harmonizer.h"


namespace bav

{
    

#define bvh_VOID_TEMPLATE template<typename SampleType> void Harmonizer<SampleType>
    

/***********************************************************************************************************************************************
 // automated midi events ----------------------------------------------------------------------------------------------------------------------
 ***********************************************************************************************************************************************/

// play chord: send an array of midi pitches into this function and it will ensure that only those desired pitches are being played.
bvh_VOID_TEMPLATE::playChord (const juce::Array<int>& desiredPitches,
                              const float velocity,
                              const bool allowTailOffOfOld)
{
    if (desiredPitches.isEmpty())
    {
        allNotesOff (allowTailOffOfOld);
        return;
    }
    
    // create array containing current pitches
    
    currentNotes.clearQuick();
    
    reportActiveNotes (currentNotes, false, true);
    
    if (currentNotes.isEmpty())
    {
        turnOnList (desiredPitches, velocity, true);
    }
    else
    {
        // 1. turn off the pitches that were previously on that are not included in the list of desired pitches
        
        desiredNotes.clearQuick();
        
        for (int note : currentNotes)
            if (! desiredPitches.contains (note))
                desiredNotes.add (note);
        
        turnOffList (desiredNotes, !allowTailOffOfOld, allowTailOffOfOld, true);
        
        // 2. turn on the desired pitches that aren't already on
        
        desiredNotes.clearQuick();
        
        for (int note : desiredPitches)
            if (! currentNotes.contains (note))
                desiredNotes.add (note);
        
        turnOnList (desiredNotes, velocity, true);
    }
}


bvh_VOID_TEMPLATE::turnOnList (const juce::Array<int>& toTurnOn, const float velocity, const bool partOfChord)
{
    if (toTurnOn.isEmpty())
        return;
    
    for (int note : toTurnOn)
        noteOn (note, velocity, false);
    
    if (! partOfChord)
        pitchCollectionChanged();
}


bvh_VOID_TEMPLATE::turnOffList (const juce::Array<int>& toTurnOff, const float velocity, const bool allowTailOff, const bool partOfChord)
{
    if (toTurnOff.isEmpty())
        return;
    
    for (int note : toTurnOff)
        noteOff (note, velocity, allowTailOff, false);
    
    if (! partOfChord)
        pitchCollectionChanged();
}


// automated midi "pedal pitch": creates a polyphonic doubling of the lowest note currently being played by a keyboard key at a specified interval below that keyboard key, IF that keyboard key is below a certain pitch threshold.
bvh_VOID_TEMPLATE::applyPedalPitch()
{
    if (pedal.interval == 0)
        return;
    
    int currentLowest = 128;
    Voice* lowestVoice = nullptr;
    
    for (auto* voice : voices) // find the current lowest note being played by a keyboard key
    {
        if (voice->isVoiceActive() && voice->isKeyDown())
        {
            const int note = voice->getCurrentlyPlayingNote();
            
            if (note < currentLowest)
            {
                currentLowest = note;
                lowestVoice = voice;
            }
        }
    }
    
    const int lastPitch = pedal.lastPitch;
    
    if (currentLowest > pedal.upperThresh) // only create a pedal voice if the current lowest keyboard key is below a specified threshold
    {
        if (lastPitch > -1)
            noteOff (lastPitch, 1.0f, false, false);
        
        return;
    }
    
    const int newPedalPitch = currentLowest - pedal.interval;
    
    if (newPedalPitch == lastPitch)  // pedal output note hasn't changed - do nothing
        return;
    
    if (newPedalPitch < 0 || isPitchActive (newPedalPitch, false, true))  // impossible midinote, or the new desired pedal pitch is already on
    {
        if (lastPitch > -1)
            noteOff (lastPitch, 1.0f, false, false);
        
        return;
    }
    
    auto* prevPedalVoice = getCurrentPedalPitchVoice();  // attempt to keep the pedal line consistent - using the same HarmonizerVoice
    
    if (prevPedalVoice != nullptr)
        if (prevPedalVoice->isKeyDown())  // can't "steal" the voice playing the last pedal note if its keyboard key is down
            prevPedalVoice = nullptr;
    
    pedal.lastPitch = newPedalPitch;
    
    if (prevPedalVoice != nullptr)
    {
        //  there was a previously active pedal voice, so steal it directly without calling noteOn:
        
        const float velocity = (lowestVoice != nullptr) ? lowestVoice->getLastRecievedVelocity() : prevPedalVoice->getLastRecievedVelocity();
        startVoice (prevPedalVoice, pedal.lastPitch, velocity, false);
    }
    else
    {
        if (lastPitch > -1)
            noteOff (lastPitch, 1.0f, false, false);
        
        const float velocity = (lowestVoice != nullptr) ? lowestVoice->getLastRecievedVelocity() : 1.0f;
        noteOn (pedal.lastPitch, velocity, false);
    }
}


// automated midi "descant": creates a polyphonic doubling of the highest note currently being played by a keyboard key at a specified interval above that keyboard key, IF that keyboard key is above a certain pitch threshold.
bvh_VOID_TEMPLATE::applyDescant()
{
    if (descant.interval == 0)
        return;
    
    int currentHighest = -1;
    Voice* highestVoice = nullptr;
    
    for (auto* voice : voices)  // find the current highest note being played by a keyboard key
    {
        if (voice->isVoiceActive() && voice->isKeyDown())
        {
            const int note = voice->getCurrentlyPlayingNote();
            
            if (note > currentHighest)
            {
                currentHighest = note;
                highestVoice = voice;
            }
        }
    }
    
    const int lastPitch = descant.lastPitch;
    
    if (currentHighest < descant.lowerThresh)  // only create a descant voice if the current highest keyboard key is above a specified threshold
    {
        if (lastPitch > -1)
            noteOff (lastPitch, 1.0f, false, false);
        
        return;
    }
    
    const int newDescantPitch = currentHighest + descant.interval;
    
    if (newDescantPitch == lastPitch)  // descant output note hasn't changed - do nothing
        return;
    
    if (newDescantPitch > 127 || isPitchActive (newDescantPitch, false, true)) // impossible midinote, or the new desired descant pitch is already on
    {
        if (lastPitch > -1)
            noteOff (lastPitch, 1.0f, false, false);
        
        return;
    }
    
    auto* prevDescantVoice = getCurrentDescantVoice();  // attempt to keep the descant line consistent - using the same HarmonizerVoice
    
    if (prevDescantVoice != nullptr)
        if (prevDescantVoice->isKeyDown())  // can't "steal" the voice playing the last descant note if its keyboard key is down
            prevDescantVoice = nullptr;
    
    descant.lastPitch = newDescantPitch;
    
    if (prevDescantVoice != nullptr)
    {
        //  there was a previously active descant voice, so steal it directly without calling noteOn:
        
        const float velocity = (highestVoice != nullptr) ? highestVoice->getLastRecievedVelocity() : prevDescantVoice->getLastRecievedVelocity();
        startVoice (prevDescantVoice, descant.lastPitch, velocity, false);
    }
    else
    {
        if (lastPitch > -1)
            noteOff (lastPitch, 1.0f, false, false);
        
        const float velocity = (highestVoice != nullptr) ? highestVoice->getLastRecievedVelocity() : 1.0f;
        noteOn (descant.lastPitch, velocity, false);
    }
}


#undef bvh_VOID_TEMPLATE

}  // namespace
