/*
    Part of module: bv_Harmonizer
    Direct parent file: bv_Harmonizer.h
    Classes: Harmonizer
*/


#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{
    

template<typename SampleType>
void Harmonizer<SampleType>::turnOffAllKeyupNotes (const bool allowTailOff,
                                                   const bool includePedalPitchAndDescant,
                                                   const float velocity,
                                                   const bool overrideSostenutoPedal)
{
    for (auto* voice : voices)
        if (voice->isVoiceActive() && ! voice->isKeyDown())
            if (includePedalPitchAndDescant || (! (voice->isCurrentPedalVoice() || voice->isCurrentDescantVoice())))
                if (overrideSostenutoPedal || ! voice->sustainingFromSostenutoPedal)
                    stopVoice (voice, velocity, allowTailOff);
};
    
    
/***********************************************************************************************************************************************
 // functions for meta midi & note management --------------------------------------------------------------------------------------------------
***********************************************************************************************************************************************/

template<typename SampleType>
bool Harmonizer<SampleType>::isPitchActive (const int midiPitch, const bool countRingingButReleased, const bool countKeyUpNotes) const
{
    for (auto* voice : voices)
        if (voice->isVoiceActive() && voice->getCurrentlyPlayingNote() == midiPitch)
            if (countRingingButReleased || ! voice->isPlayingButReleased())
                if (countKeyUpNotes || voice->isKeyDown())
                    return true;
    
    return false;
};


template<typename SampleType>
void Harmonizer<SampleType>::reportActiveNotes (Array<int>& outputArray,
                                                const bool includePlayingButReleased,
                                                const bool includeKeyUpNotes) const
{
    outputArray.clearQuick();
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            if (includePlayingButReleased || ! voice->isPlayingButReleased())
                if (includeKeyUpNotes || voice->isKeyDown())
                    outputArray.add (voice->getCurrentlyPlayingNote());
    
    if (! outputArray.isEmpty())
        outputArray.sort();
};

    
/***********************************************************************************************************************************************
 // midi events from plugin's midi input -------------------------------------------------------------------------------------------------------
***********************************************************************************************************************************************/

template<typename SampleType>
void Harmonizer<SampleType>::processMidi (MidiBuffer& midiMessages)
{
    aggregateMidiBuffer.clear();
    
    auto midiIterator = midiMessages.findNextSamplePosition(0);
    
    if (midiIterator == midiMessages.cend())
    {
        lastMidiTimeStamp = -1;
        return;
    }
    
    lastMidiTimeStamp = 0;
    
    std::for_each (midiIterator,
                   midiMessages.cend(),
                   [&] (const MidiMessageMetadata& meta)
                   {
                       handleMidiEvent (meta.getMessage(), meta.samplePosition);
                   } );
    
    pitchCollectionChanged();
    
    midiMessages.swapWith (aggregateMidiBuffer);
    
    lastMidiTimeStamp = -1;
};
    
    
template<typename SampleType>
void Harmonizer<SampleType>::processMidiEvent (const MidiMessage& m)
{
    handleMidiEvent (m, static_cast<int> (m.getTimeStamp()));
    pitchCollectionChanged();
};


template<typename SampleType>
void Harmonizer<SampleType>::handleMidiEvent (const MidiMessage& m, const int samplePosition)
{
    // events coming from a midi keyboard, or the plugin's midi input, should be routed to this function.

    lastMidiChannel   = m.getChannel();
    lastMidiTimeStamp = samplePosition - 1;
    
    if (m.isNoteOn())
        noteOn (m.getNoteNumber(), m.getFloatVelocity(), true);
    else if (m.isNoteOff())
        noteOff (m.getNoteNumber(), m.getFloatVelocity(), true, true);
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        allNotesOff (false);
    else if (m.isPitchWheel())
        handlePitchWheel (m.getPitchWheelValue());
    else if (m.isAftertouch())
        handleAftertouch (m.getNoteNumber(), m.getAfterTouchValue());
    else if (m.isChannelPressure())
        handleChannelPressure (m.getChannelPressureValue());
    else if (m.isController())
        handleController (m.getControllerNumber(), m.getControllerValue());
};

    
// this function should be called once after each time the harmonizer's overall pitch collection has changed - so, after a midi buffer of keyboard inout events has been processed, or after a chord has been triggered, etc.
template<typename SampleType>
void Harmonizer<SampleType>::pitchCollectionChanged()
{
    if (pedal.isOn)
        applyPedalPitch();
    
    if (descant.isOn)
        applyDescant();
    
    if (intervalLatchIsOn)
        updateIntervalsLatchedTo();
};
    
    
/***********************************************************************************************************************************************
 // midi note events ---------------------------------------------------------------------------------------------------------------------------
***********************************************************************************************************************************************/
    
template<typename SampleType>
void Harmonizer<SampleType>::noteOn (const int midiPitch, const float velocity, const bool isKeyboard)
{
    // N.B. the `isKeyboard` flag should be true if this note on event was triggered directly from the plugin's midi input; this flag should be false if this note event was automatically triggered by any internal function of Imogen (descant, pedal pitch, etc)
    
    HarmonizerVoice<SampleType>* newVoice;
    
    auto* prevVoice = getVoicePlayingNote (midiPitch);   //  if no voice is found playing this note, this will be a nullptr
    
    if (prevVoice != nullptr)
        newVoice = prevVoice;  // retrigger the same voice with the new velocity
    else
    {
        const bool isStealing = isKeyboard ? shouldStealNotes.load() : false;  // never steal voices for automated note events, only for keyboard triggered events
        
        newVoice = findFreeVoice (isStealing);
        
        if (newVoice == nullptr)
        {
            if (pedal.isOn && midiPitch == pedal.lastPitch)
                pedal.lastPitch = -1;
            
            if (descant.isOn && midiPitch == descant.lastPitch)
                descant.lastPitch = -1;
            
            return;
        }
    }
    
    startVoice (newVoice, midiPitch, velocity, isKeyboard);
};


template<typename SampleType>
void Harmonizer<SampleType>::startVoice (HarmonizerVoice<SampleType>* voice, const int midiPitch, const float velocity, const bool isKeyboard)
{
    if (voice == nullptr)
        return;
    
    const int prevNote = voice->getCurrentlyPlayingNote();
    const bool wasStolen = voice->isVoiceActive();  // we know the voice is being "stolen" from another note if it was already on before getting this start command
    const bool sameNoteRetriggered = (wasStolen && prevNote == midiPitch);
    
    if (! sameNoteRetriggered)  // don't output any note events if the same note is being retriggered
    {
        if (wasStolen)
            aggregateMidiBuffer.addEvent (MidiMessage::noteOff (lastMidiChannel, prevNote, 1.0f),   // voice was stolen: output a note off for the voice's previous note
                                          ++lastMidiTimeStamp);
            
        aggregateMidiBuffer.addEvent (MidiMessage::noteOn (lastMidiChannel, midiPitch, velocity),  // output the new note on
                                      ++lastMidiTimeStamp);
    }
    else
    {
        // same note retriggered: output aftertouch / channel pressure
        
        const int aftertouch = jlimit (0, 127,
                                       roundToInt ((velocity / std::max<float>(voice->getLastRecievedVelocity(), 0.001f)) * 127.0f));
        
        if (useChannelPressure)
        {
            updateChannelPressure (aftertouch);
        }
        else
        {
            aggregateMidiBuffer.addEvent (MidiMessage::aftertouchChange (lastMidiChannel, midiPitch, aftertouch),
                                          ++lastMidiTimeStamp);
            
            voice->aftertouchChanged (aftertouch);
        }
    }
    
    if (midiPitch < lowestPannedNote.load())  // set pan to 64 if note is below panning threshold
    {
        if (wasStolen)
            panner.panValTurnedOff (voice->getCurrentMidiPan());
        
        voice->setPan (64);
    }
    else if (! wasStolen)  // don't change pan if voice was stolen
    {
        voice->setPan (panner.getNextPanVal());
    }
    
    if (! wasStolen)
        voice->currentAftertouch = 0;
    
    const bool isPedal   = pedal.isOn   ? (midiPitch == pedal.lastPitch)   : false;
    const bool isDescant = descant.isOn ? (midiPitch == descant.lastPitch) : false;
    
    const uint32 timestamp = sameNoteRetriggered ? voice->noteOnTime : ++lastNoteOnCounter;  // leave the timestamp the same as it was if the same note is being retriggered
    
    const bool keydown = isKeyboard ? true : voice->isKeyDown();
    
    voice->startNote (midiPitch, velocity, timestamp, keydown, isPedal, isDescant);
};


template<typename SampleType>
void Harmonizer<SampleType>::noteOff (const int midiNoteNumber, const float velocity,
                                      const bool allowTailOff,
                                      const bool isKeyboard)
{
    // N.B. the `isKeyboard` flag should be true if this note on event was triggered directly from the plugin's midi input; this flag should be false if this note event was automatically triggered by any internal function of Imogen (descant, latch, etc)
    
    auto* voice = getVoicePlayingNote (midiNoteNumber);
    
    if (voice == nullptr)
    {
        if (pedal.isOn && midiNoteNumber == pedal.lastPitch)
            pedal.lastPitch = -1;
        
        if (descant.isOn && midiNoteNumber == descant.lastPitch)
            descant.lastPitch = -1;
        
        return;
    }
    
    if (isKeyboard)
    {
        if (latchIsOn)
        {
            voice->setKeyDown (false);
            return;
        }
        
        if (! (sustainPedalDown || sostenutoPedalDown))
            stopVoice (voice, velocity, allowTailOff);
    }
    else  // this is an automated note-off event
    {
        if (! voice->isKeyDown()) // for automated note-off events, only actually stop the voice if its keyboard key isn't currently down
        {
            stopVoice (voice, velocity, allowTailOff);
        }
        else
        {
            // we're processing an automated note-off event, but the voice's keyboard key is still being held
            
            if (pedal.isOn && midiNoteNumber == pedal.lastPitch)
            {
                pedal.lastPitch = -1;
                voice->isPedalPitchVoice = false;
                voice->setKeyDown (true);  // refresh the voice's own internal tracking of its key state
            }
            
            if (descant.isOn && midiNoteNumber == descant.lastPitch)
            {
                descant.lastPitch = -1;
                voice->isDescantVoice = false;
                voice->setKeyDown (true);  // refresh the voice's own internal tracking of its key state
            }
        }
    }
};


template<typename SampleType>
void Harmonizer<SampleType>::stopVoice (HarmonizerVoice<SampleType>* voice, const float velocity, const bool allowTailOff)
{
    if (voice == nullptr)
        return;
    
    if (sostenutoPedalDown && voice->sustainingFromSostenutoPedal)
        return;
    
    const int note = voice->getCurrentlyPlayingNote();
    
    aggregateMidiBuffer.addEvent (MidiMessage::noteOff (lastMidiChannel, note, velocity),
                                  ++lastMidiTimeStamp);
    
    if (voice->isCurrentPedalVoice())
        pedal.lastPitch = -1;
    
    if (voice->isCurrentDescantVoice())
        descant.lastPitch = -1;
    
    voice->stopNote (velocity, allowTailOff);
};


template<typename SampleType>
void Harmonizer<SampleType>::allNotesOff (const bool allowTailOff, const float velocity)
{
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            stopVoice (voice, velocity, allowTailOff);
    
    panner.reset();
};

    
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
            if (voice->isVoiceActive() && ! voice->isKeyDown() && ! intervalLatchNotes.contains (voice->getCurrentlyPlayingNote()))
                if (! voice->isCurrentPedalVoice() && ! voice->isCurrentDescantVoice())
                    stopVoice (voice, velocity, allowTailOff);
    }
    
    pitchCollectionChanged();
};


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
};


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
};


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
};


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
        {
            if (! currentNotes.contains (note))
                toTurnOn.add (note);
        }
        
        turnOnList (toTurnOn, velocity, true);
    }
};


template<typename SampleType>
void Harmonizer<SampleType>::turnOnList (const Array<int>& toTurnOn, const float velocity, const bool partOfChord)
{
    if (toTurnOn.isEmpty())
        return;
    
    for (int note : toTurnOn)
        noteOn (note, velocity, false);
    
    if (! partOfChord)
        pitchCollectionChanged();
};


template<typename SampleType>
void Harmonizer<SampleType>::turnOffList (const Array<int>& toTurnOff, const float velocity, const bool allowTailOff, const bool partOfChord)
{
    if (toTurnOff.isEmpty())
        return;
    
    for (int note : toTurnOff)
        noteOff (note, velocity, allowTailOff, false);
    
    if (! partOfChord)
        pitchCollectionChanged();
};


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
};


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
};

    
/***********************************************************************************************************************************************
 // other midi events --------------------------------------------------------------------------------------------------------------------------
***********************************************************************************************************************************************/

template<typename SampleType>
void Harmonizer<SampleType>::handlePitchWheel (const int wheelValue)
{
    if (lastPitchWheelValue == wheelValue)
        return;
    
    aggregateMidiBuffer.addEvent (MidiMessage::pitchWheel (lastMidiChannel, wheelValue),
                                  ++lastMidiTimeStamp);
    
    lastPitchWheelValue = wheelValue;
    bendTracker.newPitchbendRecieved (wheelValue);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
};


template<typename SampleType>
void Harmonizer<SampleType>::handleAftertouch (const int midiNoteNumber, const int aftertouchValue)
{
    if (useChannelPressure)
    {
        updateChannelPressure (aftertouchValue);
    }
    else
    {
        aggregateMidiBuffer.addEvent (MidiMessage::aftertouchChange (lastMidiChannel, midiNoteNumber, aftertouchValue),
                                      ++lastMidiTimeStamp);
        
        for (auto* voice : voices)
            if (voice->isVoiceActive() && voice->getCurrentlyPlayingNote() == midiNoteNumber)
                voice->aftertouchChanged (aftertouchValue);
    }
};


template<typename SampleType>
void Harmonizer<SampleType>::handleChannelPressure (const int channelPressureValue)
{
    if (useChannelPressure)
        aggregateMidiBuffer.addEvent (MidiMessage::channelPressureChange (lastMidiChannel, channelPressureValue),
                                      ++lastMidiTimeStamp);
    
    for (auto* voice : voices)
    {
        if (voice->isVoiceActive())
        {
            voice->aftertouchChanged (channelPressureValue);
            
            if (! useChannelPressure)
                aggregateMidiBuffer.addEvent (MidiMessage::aftertouchChange (lastMidiChannel, voice->getCurrentlyPlayingNote(), channelPressureValue),
                                              ++lastMidiTimeStamp);
        }
    }
};

    
template<typename SampleType>
void Harmonizer<SampleType>::updateChannelPressure (const int newIncomingAftertouch)
{
    int highestAftertouch = -1;
    
    for (auto* voice : voices)
    {
        if (! voice->isVoiceActive())
            continue;
        
        const int at = voice->currentAftertouch;
        
        if (at > highestAftertouch)
            highestAftertouch = at;
    }
    
    if (newIncomingAftertouch < highestAftertouch)
        return;
    
    handleChannelPressure (newIncomingAftertouch);
};
    

template<typename SampleType>
void Harmonizer<SampleType>::handleController (const int controllerNumber, const int controllerValue)
{
    switch (controllerNumber)
    {
        case 0x1:   handleModWheel        (controllerValue);    return;
        case 0x2:   handleBreathController(controllerValue);    return;
        case 0x4:   handleFootController  (controllerValue);    return;
        case 0x5:   handlePortamentoTime  (controllerValue);    return;
        case 0x8:   handleBalance         (controllerValue);    return;
        case 0x40:  handleSustainPedal    (controllerValue);    return;
        case 0x42:  handleSostenutoPedal  (controllerValue);    return;
        case 0x43:  handleSoftPedal       (controllerValue);    return;
        case 0x44:  handleLegato          (controllerValue >= 64);  return;
        default:    return;
    }
};


template<typename SampleType>
void Harmonizer<SampleType>::handleSustainPedal (const int value)
{
    const bool isDown = (value >= 64);
    
    if (sustainPedalDown == isDown)
        return;
    
    aggregateMidiBuffer.addEvent (MidiMessage::controllerEvent (lastMidiChannel, 0x40, value),
                                  ++lastMidiTimeStamp);
    
    sustainPedalDown = isDown;
    
    if (! isDown)
        if (! (latchIsOn || intervalLatchIsOn))
            turnOffAllKeyupNotes (false, false, 0.0f, false);
};


template<typename SampleType>
void Harmonizer<SampleType>::handleSostenutoPedal (const int value)
{
    const bool isDown = (value >= 64);
    
    if (sostenutoPedalDown == isDown)
        return;
    
    aggregateMidiBuffer.addEvent (MidiMessage::controllerEvent (lastMidiChannel, 0x42, value),
                                  ++lastMidiTimeStamp);
    
    sostenutoPedalDown = isDown;
    
    if (isDown)
    {
        for (auto* voice : voices)
            if (voice->isVoiceActive() && !voice->isPedalPitchVoice && !voice->isDescantVoice)
                voice->sustainingFromSostenutoPedal = (! (latchIsOn || intervalLatchIsOn));
    }
    else
    {
        for (auto* voice : voices)
            if (voice->isVoiceActive() && voice->sustainingFromSostenutoPedal)
                stopVoice (voice, 0.0f, false);
    }
};


template<typename SampleType>
void Harmonizer<SampleType>::handleSoftPedal (const int value)
{
    const bool isDown = value >= 64;
    
    if (softPedalDown == isDown)
        return;
    
    softPedalDown = isDown;
    
    aggregateMidiBuffer.addEvent (MidiMessage::controllerEvent (lastMidiChannel, 0x43, value),
                                  ++lastMidiTimeStamp);
};


template<typename SampleType>
void Harmonizer<SampleType>::handleModWheel (const int wheelValue)
{
    ignoreUnused(wheelValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleBreathController (const int controlValue)
{
    ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleFootController (const int controlValue)
{
    ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handlePortamentoTime (const int controlValue)
{
    ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleBalance (const int controlValue)
{
    ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleLegato (const bool isOn)
{
    ignoreUnused(isOn);
};


}; // namespace
