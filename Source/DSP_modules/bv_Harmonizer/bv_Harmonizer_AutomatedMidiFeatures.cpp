/*
 Part of module: bv_Harmonizer
 Direct parent file: bv_Harmonizer.h
 Classes: Harmonizer
 */

#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{
    

/***********************************************************************************************************************************************
 // automated midi events ----------------------------------------------------------------------------------------------------------------------
 ***********************************************************************************************************************************************/

// midi latch: when active, holds note offs recieved from the keyboard instead of sending them immediately; held note offs are sent once latch is deactivated.
template<typename SampleType>
void Harmonizer<SampleType>::setMidiLatch (const bool shouldBeOn, const bool allowTailOff)
{
    if (latchIsOn == shouldBeOn)
        return;
    
    latchIsOn = shouldBeOn;
    
    if (shouldBeOn)
        return;
    
    if (! intervalLatchIsOn || intervalsLatchedTo.isEmpty())
        turnOffAllKeyupNotes (allowTailOff, false, !allowTailOff, false);
    else
    {
        // turn off all voices whose key is up and who aren't being held by the interval latch function
        
        const int currentMidiPitch = roundToInt (pitchConverter.ftom (currentInputFreq));
        
        Array<int> intervalLatchNotes;
        intervalLatchNotes.ensureStorageAllocated (intervalsLatchedTo.size());
        
        for (int interval : intervalsLatchedTo)
            intervalLatchNotes.add (currentMidiPitch + interval);
        
        const float velocity = allowTailOff ? 0.0f : 1.0f;
        
        for (auto* voice : voices)
            if ((voice->isVoiceActive() && ! voice->isKeyDown() && ! intervalLatchNotes.contains (voice->getCurrentlyPlayingNote()))
                && (! voice->isCurrentPedalVoice() && ! voice->isCurrentDescantVoice()))
            { stopVoice (voice, velocity, allowTailOff); }
    }
    
    pitchCollectionChanged();
}


// interval latch
template<typename SampleType>
void Harmonizer<SampleType>::setIntervalLatch (const bool shouldBeOn, const bool allowTailOff)
{
    if (intervalLatchIsOn == shouldBeOn)
        return;
    
    intervalLatchIsOn = shouldBeOn;
    
    if (shouldBeOn)
        updateIntervalsLatchedTo();
    else if (! latchIsOn)
    {
        turnOffAllKeyupNotes (allowTailOff, false, !allowTailOff, false);
        pitchCollectionChanged();
    }
}


// used for interval latch -- saves the distance in semitones of each currently playing note from the current input pitch
template<typename SampleType>
void Harmonizer<SampleType>::updateIntervalsLatchedTo()
{
    intervalsLatchedTo.clearQuick();
    
    Array<int> currentNotes;
    currentNotes.ensureStorageAllocated (voices.size());
    
    reportActiveNotes (currentNotes, false);
    
    if (currentNotes.isEmpty())
        return;
    
    const int currentMidiPitch = roundToInt (pitchConverter.ftom (currentInputFreq));
    
    for (int note : currentNotes)
        intervalsLatchedTo.add (note - currentMidiPitch);
}


// plays a chord based on a given set of desired interval offsets from the current input pitch.
template<typename SampleType>
void Harmonizer<SampleType>::playIntervalSet (const Array<int>& desiredIntervals,
                                              const float velocity,
                                              const bool allowTailOffOfOld,
                                              const bool isIntervalLatch)
{
    if (desiredIntervals.isEmpty())
    {
        allNotesOff (allowTailOffOfOld);
        return;
    }
    
    const int currentInputPitch = roundToInt (pitchConverter.ftom (currentInputFreq));
    
    Array<int> desiredNotes;
    desiredNotes.ensureStorageAllocated (desiredIntervals.size());
    
    for (int interval : desiredIntervals)
        desiredNotes.add (currentInputPitch + interval);
    
    playChord (desiredNotes, velocity, allowTailOffOfOld);
    
    if (! isIntervalLatch)
        pitchCollectionChanged();
}


// play chord: send an array of midi pitches into this function and it will ensure that only those desired pitches are being played.
template<typename SampleType>
void Harmonizer<SampleType>::playChord (const Array<int>& desiredPitches,
                                        const float velocity,
                                        const bool allowTailOffOfOld)
{
    if (desiredPitches.isEmpty())
    {
        allNotesOff (allowTailOffOfOld);
        return;
    }
    
    // create array containing current pitches
    
    Array<int> currentNotes;
    currentNotes.ensureStorageAllocated (voices.size());
    
    reportActiveNotes (currentNotes, false, true);
    
    if (currentNotes.isEmpty())
    {
        turnOnList (desiredPitches, velocity, true);
    }
    else
    {
        // 1. turn off the pitches that were previously on that are not included in the list of desired pitches
        
        Array<int> toTurnOff;
        toTurnOff.ensureStorageAllocated (currentNotes.size());
        
        for (int note : currentNotes)
            if (! desiredPitches.contains (note))
                toTurnOff.add (note);
        
        turnOffList (toTurnOff, !allowTailOffOfOld, allowTailOffOfOld, true);
        
        // 2. turn on the desired pitches that aren't already on
        
        Array<int> toTurnOn;
        toTurnOn.ensureStorageAllocated (desiredPitches.size());
        
        for (int note : desiredPitches)
            if (! currentNotes.contains (note))
                toTurnOn.add (note);
        
        turnOnList (toTurnOn, velocity, true);
    }
}


template<typename SampleType>
void Harmonizer<SampleType>::turnOnList (const Array<int>& toTurnOn, const float velocity, const bool partOfChord)
{
    if (toTurnOn.isEmpty())
        return;
    
    for (int note : toTurnOn)
        noteOn (note, velocity, false);
    
    if (! partOfChord)
        pitchCollectionChanged();
}


template<typename SampleType>
void Harmonizer<SampleType>::turnOffList (const Array<int>& toTurnOff, const float velocity, const bool allowTailOff, const bool partOfChord)
{
    if (toTurnOff.isEmpty())
        return;
    
    for (int note : toTurnOff)
        noteOff (note, velocity, allowTailOff, false);
    
    if (! partOfChord)
        pitchCollectionChanged();
}


// automated midi "pedal pitch": creates a polyphonic doubling of the lowest note currently being played by a keyboard key at a specified interval below that keyboard key, IF that keyboard key is below a certain pitch threshold.
template<typename SampleType>
void Harmonizer<SampleType>::applyPedalPitch()
{
    int currentLowest = 128;
    HarmonizerVoice<SampleType>* lowestVoice = nullptr;
    
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
    
    if (currentLowest > pedal.upperThresh) // only create a pedal voice if the current lowest keyboard key is below a specified threshold
    {
        if (pedal.lastPitch > -1)
            noteOff (pedal.lastPitch, 1.0f, false, false);
        
        return;
    }
    
    const int newPedalPitch = currentLowest - pedal.interval;
    
    if (newPedalPitch == pedal.lastPitch)  // pedal output note hasn't changed - do nothing
        return;
    
    if (newPedalPitch < 0 || isPitchActive (newPedalPitch, false, true))  // impossible midinote, or the new desired pedal pitch is already on
    {
        if (pedal.lastPitch > -1)
            noteOff (pedal.lastPitch, 1.0f, false, false);
        
        return;
    }
    
    auto* prevPedalVoice = getCurrentPedalPitchVoice();  // attempt to keep the pedal line consistent - using the same HarmonizerVoice
    
    if (prevPedalVoice != nullptr)
        if (prevPedalVoice->isKeyDown())  // can't "steal" the voice playing the last pedal note if its keyboard key is down
            prevPedalVoice = nullptr;
    
    if (prevPedalVoice != nullptr)
    {
        //  there was a previously active pedal voice, so steal it directly without calling noteOn:
        
        const float velocity = (lowestVoice != nullptr) ? lowestVoice->getLastRecievedVelocity() : prevPedalVoice->getLastRecievedVelocity();
        pedal.lastPitch = newPedalPitch;
        startVoice (prevPedalVoice, pedal.lastPitch, velocity, false);
    }
    else
    {
        if (pedal.lastPitch > -1)
            noteOff (pedal.lastPitch, 1.0f, false, false);
        
        const float velocity = (lowestVoice != nullptr) ? lowestVoice->getLastRecievedVelocity() : 1.0f;
        pedal.lastPitch = newPedalPitch;
        noteOn (pedal.lastPitch, velocity, false);
    }
}


// automated midi "descant": creates a polyphonic doubling of the highest note currently being played by a keyboard key at a specified interval above that keyboard key, IF that keyboard key is above a certain pitch threshold.
template<typename SampleType>
void Harmonizer<SampleType>::applyDescant()
{
    int currentHighest = -1;
    HarmonizerVoice<SampleType>* highestVoice = nullptr;
    
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
    
    if (currentHighest < descant.lowerThresh)  // only create a descant voice if the current highest keyboard key is above a specified threshold
    {
        if (descant.lastPitch > -1)
            noteOff (descant.lastPitch, 1.0f, false, false);
        
        return;
    }
    
    const int newDescantPitch = currentHighest + descant.interval;
    
    if (newDescantPitch == descant.lastPitch)  // descant output note hasn't changed - do nothing
        return;
    
    if (newDescantPitch > 127 || isPitchActive (newDescantPitch, false, true)) // impossible midinote, or the new desired descant pitch is already on
    {
        if (descant.lastPitch > -1)
            noteOff (descant.lastPitch, 1.0f, false, false);
        
        return;
    }
    
    auto* prevDescantVoice = getCurrentDescantVoice();  // attempt to keep the descant line consistent - using the same HarmonizerVoice
    
    if (prevDescantVoice != nullptr)
        if (prevDescantVoice->isKeyDown())  // can't "steal" the voice playing the last descant note if its keyboard key is down
            prevDescantVoice = nullptr;
    
    if (prevDescantVoice != nullptr)
    {
        //  there was a previously active descant voice, so steal it directly without calling noteOn:
        
        const float velocity = (highestVoice != nullptr) ? highestVoice->getLastRecievedVelocity() : prevDescantVoice->getLastRecievedVelocity();
        descant.lastPitch = newDescantPitch;
        startVoice (prevDescantVoice, descant.lastPitch, velocity, false);
    }
    else
    {
        if (descant.lastPitch > -1)
            noteOff (descant.lastPitch, 1.0f, false, false);
        
        const float velocity = (highestVoice != nullptr) ? highestVoice->getLastRecievedVelocity() : 1.0f;
        descant.lastPitch = newDescantPitch;
        noteOn (descant.lastPitch, velocity, false);
    }
}


}  // namespace
