/*
  ==============================================================================

    HarmonizerMidi.cpp
    Created: 22 Jan 2021 1:50:21am
    Author:  Ben Vining

  ==============================================================================
*/

#include "bv_Harmonizer/bv_Harmonizer.h"

template<typename SampleType>
void Harmonizer<SampleType>::turnOffAllKeyupNotes (const bool allowTailOff,
                                                   const bool includePedalPitchAndDescant)
{
    const juce::ScopedLock sl (lock);
    
    const float velocity = allowTailOff ? 0.0f : 1.0f;
    
    for (auto* voice : voices)
    {
        if (voice->isVoiceActive() && ! voice->isKeyDown())
        {
            if (includePedalPitchAndDescant)
                stopVoice (voice, velocity, allowTailOff);
            else if (! voice->isCurrentPedalVoice() && ! voice->isCurrentDescantVoice())
                stopVoice (voice, velocity, allowTailOff);
        }
    }
};


// MIDI events --------------------------------------------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// KEYBOARD / PLUGIN MIDI INPUT --------------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename SampleType>
void Harmonizer<SampleType>::processMidi (juce::MidiBuffer& midiMessages)
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
                   [&] (const juce::MidiMessageMetadata& meta)
                   {
                       handleMidiEvent (meta.getMessage(), meta.samplePosition);
                   } );
    
    pitchCollectionChanged();
    
    midiMessages.swapWith (aggregateMidiBuffer);
    
    lastMidiTimeStamp = -1;
};


template<typename SampleType>
void Harmonizer<SampleType>::handleMidiEvent (const juce::MidiMessage& m, const int samplePosition)
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AUTOMATED MIDI EVENTS ----------------------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
        turnOffAllKeyupNotes (allowTailOff, false);
    else
    {
        // turn off all voices whose key is up and who aren't being held by the interval latch function
        
        const juce::ScopedLock sl (lock);
        
        const int currentMidiPitch = juce::roundToInt (pitchConverter.ftom (currentInputFreq));
        
        juce::Array<int> intervalLatchNotes;
        intervalLatchNotes.ensureStorageAllocated (intervalsLatchedTo.size());
        
        for (int interval : intervalsLatchedTo)
            intervalLatchNotes.add (currentMidiPitch + interval);
        
        juce::Array< HarmonizerVoice<SampleType>* > toTurnOff;
        toTurnOff.ensureStorageAllocated (voices.size());
        
        for (auto* voice : voices)
        {
            if (voice->isVoiceActive() && ! voice->isKeyDown())
            {
                const int note = voice->getCurrentlyPlayingNote();
                
                if (note != lastPedalPitch && note != lastDescantPitch
                    && ! intervalLatchNotes.contains (note))
                {
                    toTurnOff.add (voice);
                }
            }
        }
        
        if (toTurnOff.isEmpty())
            return;
        
        const float velocity = allowTailOff ? 0.0f : 1.0f;
        
        for (auto* voice : toTurnOff)
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
        turnOffAllKeyupNotes (allowTailOff, false);
};


// used for interval latch -- saves the distance in semitones of each currently playing note from the current input pitch
template<typename SampleType>
void Harmonizer<SampleType>::updateIntervalsLatchedTo()
{
    const juce::ScopedLock sl (lock);
    
    intervalsLatchedTo.clearQuick();
    
    juce::Array<int> currentNotes;
    currentNotes.ensureStorageAllocated (voices.size());
    
    reportActivesNoReleased (currentNotes);
    
    if (currentNotes.isEmpty())
        return;
    
    const int currentMidiPitch = juce::roundToInt (pitchConverter.ftom (currentInputFreq));
    
    for (int note : currentNotes)
        intervalsLatchedTo.add (note - currentMidiPitch);
};


// plays a chord based on a given set of desired interval offsets from the current input pitch.
template<typename SampleType>
void Harmonizer<SampleType>::playChordFromIntervalSet (const juce::Array<int>& desiredIntervals)
{
    if (desiredIntervals.isEmpty())
    {
        allNotesOff (false);
        return;
    }
    
    const juce::ScopedLock sl (lock);
    
    const float currentInputPitch = pitchConverter.ftom (currentInputFreq);
    
    juce::Array<int> desiredNotes;
    desiredNotes.ensureStorageAllocated (desiredIntervals.size());
    
    for (int interval : desiredIntervals)
        desiredNotes.add (juce::roundToInt (currentInputPitch + interval));
    
    playChord (desiredNotes, 1.0f, false);
};


// play chord: send an array of midi pitches into this function and it will ensure that only those desired pitches are being played.
template<typename SampleType>
void Harmonizer<SampleType>::playChord (const juce::Array<int>& desiredPitches,
                                        const float velocity,
                                        const bool allowTailOffOfOld,
                                        const bool isIntervalLatch)
{
    const juce::ScopedLock sl (lock);
    
    if (desiredPitches.isEmpty())
    {
        allNotesOff (allowTailOffOfOld);
        return;
    }
    
    // create array containing current pitches
    
    juce::Array<int> currentNotes;
    currentNotes.ensureStorageAllocated (voices.size());
    
    reportActivesNoReleased (currentNotes);
    
    // 1. turn off the pitches that were previously on that are not included in the list of desired pitches
    
    if (! currentNotes.isEmpty())
    {
        juce::Array<int> toTurnOff;
        toTurnOff.ensureStorageAllocated (currentNotes.size());
    
        for (int note : currentNotes)
            if (! desiredPitches.contains (note))
                toTurnOff.add (note);
    
        turnOffList (toTurnOff, !allowTailOffOfOld, allowTailOffOfOld, true);
    }

    // 2. turn on the desired pitches that aren't already on
    
    if (currentNotes.isEmpty())
    {
        turnOnList (desiredPitches, velocity, true);
    }
    else
    {
        juce::Array<int> toTurnOn;
        toTurnOn.ensureStorageAllocated (currentNotes.size());
        
        for (int note : desiredPitches)
        {
            if (! currentNotes.contains(note))
                toTurnOn.add (note);
        }
        
        turnOnList (toTurnOn, velocity, true);
    }
    
    if (! isIntervalLatch)
        pitchCollectionChanged();
};


// turn on list: turns on a list of specified notes in quick sequence.
template<typename SampleType>
void Harmonizer<SampleType>::turnOnList (const juce::Array<int>& toTurnOn, const float velocity, const bool partOfChord)
{
    const juce::ScopedLock sl (lock);
    
    if (toTurnOn.isEmpty())
        return;
    
    for (int note : toTurnOn)
        noteOn (note, velocity, false);
    
    if (! partOfChord)
        pitchCollectionChanged();
};


// turn off list: turns off a list of specified notes in quick sequence.
template<typename SampleType>
void Harmonizer<SampleType>::turnOffList (const juce::Array<int>& toTurnOff, const float velocity, const bool allowTailOff, const bool partOfChord)
{
    const juce::ScopedLock sl (lock);
    
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
    const juce::ScopedLock sl (lock);
    
    int currentLowest = 128; // find the current lowest note being played by a keyboard key
    
    for (auto* voice : voices)
    {
        if (voice->isVoiceActive() && voice->isKeyDown())
        {
            const int note = voice->getCurrentlyPlayingNote();
            
            if (note < currentLowest)
                currentLowest = note;
        }
    }
    
    if ((currentLowest == 128) || (currentLowest > pedalPitchUpperThresh))
    {
        if (lastPedalPitch > -1)
            noteOff (lastPedalPitch, 1.0f, false, false);
        
        return;
    }
    
    const int newPedalPitch = currentLowest - pedalPitchInterval;
    
    if (newPedalPitch == lastPedalPitch)
        return;
    
    if (lastPedalPitch > -1)
        noteOff (lastPedalPitch, 1.0f, false, false);
    
    if (newPedalPitch < 0)
    {
        lastPedalPitch = -1;
        return;
    }
    
    lastPedalPitch = newPedalPitch;
    
    float velocity;
    
    if (auto* voiceCopying = getVoicePlayingNote(currentLowest))
        velocity = voiceCopying->getLastRecievedVelocity();
    else
        velocity = 1.0f;
    
    noteOn (newPedalPitch, velocity, false);
};


// automated midi "descant": creates a polyphonic doubling of the highest note currently being played by a keyboard key at a specified interval above that keyboard key, IF that keyboard key is above a certain pitch threshold.
template<typename SampleType>
void Harmonizer<SampleType>::applyDescant()
{
    const juce::ScopedLock sl (lock);
    
    int currentHighest = -1;
    for (auto* voice : voices)
    {
        if (voice->isVoiceActive() && voice->isKeyDown())
        {
            const int note = voice->getCurrentlyPlayingNote();
            
            if (note > currentHighest)
                currentHighest = note;
        }
    }
    
    if ((currentHighest == -1) || (currentHighest < descantLowerThresh))
    {
        if (lastDescantPitch > -1)
            noteOff (lastDescantPitch, 1.0f, false, false);
        
        return;
    }
    
    const int newDescantPitch = currentHighest + descantInterval;
    
    if (newDescantPitch == lastDescantPitch)
        return;
    
    if (lastDescantPitch > -1)
        noteOff (lastDescantPitch, 1.0f, false, false);
    
    if (newDescantPitch > 127)
    {
        lastDescantPitch = -1;
        return;
    }
    
    lastDescantPitch = newDescantPitch;
    
    float velocity;
    
    if (auto* voiceCopying = getVoicePlayingNote(currentHighest))
        velocity = voiceCopying->getLastRecievedVelocity();
    else
        velocity = 1.0f;
    
    noteOn (newDescantPitch, velocity, false);
};


// this function should be called once after each time the harmonizer's overall pitch collection has changed - so, after a midi buffer of keyboard inout events has been processed, or after a chord has been triggered, etc.
template<typename SampleType>
void Harmonizer<SampleType>::pitchCollectionChanged()
{
    const juce::ScopedLock sl (lock);
    
    if (pedalPitchIsOn)
        applyPedalPitch();
    
    if (descantIsOn)
        applyDescant();
    
    if (intervalLatchIsOn)
    {
        updateIntervalsLatchedTo();
        playChordFromIntervalSet (intervalsLatchedTo);
    }
};


// NOTE EVENTS --------------------------------------------------------------------------------------------------------------------------------------

template<typename SampleType>
void Harmonizer<SampleType>::noteOn (const int midiPitch, const float velocity, const bool isKeyboard)
{
    // N.B. the `isKeyboard` flag should be true if this note on event was triggered directly from the midi keyboard input; this flag is false if this note on event was triggered automatically by pedal pitch or descant.
    
    const juce::ScopedLock sl (lock);
    
    if (isPitchActive (midiPitch, false))
        return;
    
    bool isStealing = isKeyboard ? shouldStealNotes : false; // never steal voices for automated note events, only for keyboard triggered events
    
    startVoice (findFreeVoice (midiPitch, isStealing), midiPitch, velocity, isKeyboard);
};


template<typename SampleType>
void Harmonizer<SampleType>::startVoice (HarmonizerVoice<SampleType>* voice, const int midiPitch, const float velocity, const bool isKeyboard)
{
    if (voice == nullptr)
        return;
    
    aggregateMidiBuffer.addEvent (juce::MidiMessage::noteOn (lastMidiChannel, midiPitch, velocity),
                                  ++lastMidiTimeStamp);
    
    const bool wasStolen = voice->isVoiceActive(); // we know the voice is being "stolen" from another note if it was already on before getting this start command
    
    if (! voice->isKeyDown()) // if the key wasn't already marked as down...
        voice->setKeyDown (isKeyboard); // then mark it as down IF this start command is because of a keyboard event
    
    if (midiPitch < lowestPannedNote)
        voice->setPan (64, wasStolen);
    else if (! wasStolen) // don't change pan if voice was stolen
        voice->setPan (panner.getNextPanVal(), false);
    
    const bool isPedal = pedalPitchIsOn ? (midiPitch == lastPedalPitch) : false;
    const bool isDescant = descantIsOn ? (midiPitch == lastDescantPitch) : false;
    
    voice->startNote (midiPitch, velocity, ++lastNoteOnCounter, wasStolen, isPedal, isDescant);
};


template<typename SampleType>
void Harmonizer<SampleType>::noteOff (const int midiNoteNumber, const float velocity,
                                      const bool allowTailOff,
                                      const bool isKeyboard)
{
    // N.B. the `isKeyboard` flag should be true if this note off event was triggered directly from the midi keyboard input; this flag is false if this note off event was triggered automatically by pedal pitch, descant, latch, etc
    
    auto* voice = getVoicePlayingNote (midiNoteNumber);
    
    if (voice == nullptr)
        return;
    
    const juce::ScopedLock sl (lock);
    
    if (isKeyboard)
    {
        voice->setKeyDown (false);
        
        if (latchIsOn)
            return;
        
        if (! (sustainPedalDown || sostenutoPedalDown) )
            stopVoice (voice, velocity, allowTailOff);
    }
    else if (! voice->isKeyDown())
    {
        stopVoice (voice, velocity, allowTailOff);
    }
};


template<typename SampleType>
void Harmonizer<SampleType>::stopVoice (HarmonizerVoice<SampleType>* voice, const float velocity, const bool allowTailOff)
{
    if (voice == nullptr)
        return;
    
    const int note = voice->getCurrentlyPlayingNote();
    
    aggregateMidiBuffer.addEvent (juce::MidiMessage::noteOff (lastMidiChannel, note, velocity),
                                  ++lastMidiTimeStamp);
    
    if (pedalPitchIsOn)
        if (voice->isCurrentPedalVoice())
            lastPedalPitch = -1;
    
    if (descantIsOn)
        if (voice->isCurrentDescantVoice())
            lastDescantPitch = -1;
    
    voice->stopNote (velocity, allowTailOff);
};


template<typename SampleType>
void Harmonizer<SampleType>::allNotesOff (const bool allowTailOff)
{
    const juce::ScopedLock sl (lock);
    
    const float velocity = allowTailOff ? 0.0f : 1.0f;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            stopVoice (voice, velocity, allowTailOff);
    
    panner.reset (false);
};


// VOICE ALLOCATION ---------------------------------------------------------------------------------------------------------------------------------

template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::findFreeVoice (const int midiNoteNumber, const bool stealIfNoneAvailable)
{
    const juce::ScopedLock sl (lock);
    
    for (auto* voice : voices)
        if (! voice->isVoiceActive())
            return voice;
    
    if (stealIfNoneAvailable)
        return findVoiceToSteal (midiNoteNumber);
    
    return nullptr;
};


template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::findVoiceToSteal (const int midiNoteNumber)
{
    // This voice-stealing algorithm applies the following heuristics:
    // - Re-use the oldest notes first
    // - Protect the lowest & topmost notes, even if sustained, but not if they've been released.
    // - Protects the pedal & descant auto voices, if active, and only releases them as a last resort to avoid stealing the lowest & highest notes being played manually
    
    jassert (! voices.isEmpty());
    
    // These are the voices we want to protect
    HarmonizerVoice<SampleType>* low = nullptr; // Lowest sounding note, might be sustained, but NOT in release phase
    HarmonizerVoice<SampleType>* top = nullptr; // Highest sounding note, might be sustained, but NOT in release phase
    
    // protect these, only use if necessary. These will be nullptrs if pedal / descant is currently off
    HarmonizerVoice<SampleType>* descantVoice = getCurrentDescantVoice();
    HarmonizerVoice<SampleType>* pedalVoice = getCurrentPedalPitchVoice();
    
    // this is a list of voices we can steal, sorted by how long they've been running
    juce::Array< HarmonizerVoice<SampleType>* > usableVoices;
    usableVoices.ensureStorageAllocated (voices.size());
    
    for (auto* voice : voices)
    {
        if (voice == descantVoice || voice == pedalVoice)
            continue;
        
        usableVoices.add (voice);
        
        // NB: Using a functor rather than a lambda here due to scare-stories about compilers generating code containing heap allocations..
        struct Sorter
        {
            bool operator() (const HarmonizerVoice<SampleType>* a, const HarmonizerVoice<SampleType>* b) const noexcept
            { return a->wasStartedBefore (*b); }
        };
        
        std::sort (usableVoices.begin(), usableVoices.end(), Sorter());
        
        if (voice->isVoiceActive() && ! voice->isPlayingButReleased())
        {
            auto note = voice->getCurrentlyPlayingNote();
            
            if (low == nullptr || note < low->getCurrentlyPlayingNote())
                low = voice;
            
            if (top == nullptr || note > top->getCurrentlyPlayingNote())
                top = voice;
        }
    }
    
    if (top == low)  // Eliminate pathological cases (ie: only 1 note playing): we always give precedence to the lowest note(s)
        top = nullptr;
    
    for (auto* voice : usableVoices)
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
            return voice;
    
    for (auto* voice : usableVoices)
        if (voice != low && voice != top && ! voice->isKeyDown())
            return voice;
    
    for (auto* voice : usableVoices)
        if (voice != low && voice != top)
            return voice;
    
    // only protected top & bottom voices are left now - time to use the pedal pitch & descant voices...
    
    if (descantVoice != nullptr) // save bass
        return descantVoice;
    
    if (pedalVoice != nullptr)
        return pedalVoice;
    
    // return final top & bottom notes held with keyboard keys
    
    if (top != nullptr) // save bass
        return top;
    
    return low;
};

// OTHER MIDI ---------------------------------------------------------------------------------------------------------------------------------------

template<typename SampleType>
void Harmonizer<SampleType>::handlePitchWheel (const int wheelValue)
{
    if (lastPitchWheelValue == wheelValue)
        return;
    
    const juce::ScopedLock sl (lock);
    
    aggregateMidiBuffer.addEvent (juce::MidiMessage::pitchWheel (lastMidiChannel, wheelValue),
                                  ++lastMidiTimeStamp);
    
    lastPitchWheelValue = wheelValue;
    bendTracker.newPitchbendRecieved(wheelValue);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
};


template<typename SampleType>
void Harmonizer<SampleType>::handleAftertouch (const int midiNoteNumber, const int aftertouchValue)
{
    const juce::ScopedLock sl (lock);
    
    aggregateMidiBuffer.addEvent (juce::MidiMessage::aftertouchChange (lastMidiChannel, midiNoteNumber, aftertouchValue),
                                  ++lastMidiTimeStamp);
    
    for (auto* voice : voices)
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
            voice->aftertouchChanged (aftertouchValue);
};


template<typename SampleType>
void Harmonizer<SampleType>::handleChannelPressure (const int channelPressureValue)
{
    const juce::ScopedLock sl (lock);
    
    aggregateMidiBuffer.addEvent (juce::MidiMessage::channelPressureChange (lastMidiChannel, channelPressureValue),
                                  ++lastMidiTimeStamp);
    
    for (auto* voice : voices)
        voice->aftertouchChanged(channelPressureValue);
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
    
    sustainPedalDown = isDown;
    
    if (isDown || latchIsOn || intervalLatchIsOn)
        return;
    
    turnOffAllKeyupNotes (false, false);
    
    aggregateMidiBuffer.addEvent (juce::MidiMessage::controllerEvent (lastMidiChannel, 0x40, value),
                                  ++lastMidiTimeStamp);
};


template<typename SampleType>
void Harmonizer<SampleType>::handleSostenutoPedal (const int value)
{
    const bool isDown = (value >= 64);
    
    if (sostenutoPedalDown == isDown)
        return;
    
    sostenutoPedalDown = isDown;
    
    if (isDown || latchIsOn || intervalLatchIsOn)
        return;
    
    turnOffAllKeyupNotes (false, false);
    
    aggregateMidiBuffer.addEvent (juce::MidiMessage::controllerEvent (lastMidiChannel, 0x42, value),
                                  ++lastMidiTimeStamp);
};


template<typename SampleType>
void Harmonizer<SampleType>::handleSoftPedal (const int value)
{
    const bool isDown = value >= 64;
    
    if (softPedalDown == isDown)
        return;
    
    softPedalDown = isDown;
    
    aggregateMidiBuffer.addEvent (juce::MidiMessage::controllerEvent (lastMidiChannel, 0x43, value),
                                  ++lastMidiTimeStamp);
};


template<typename SampleType>
void Harmonizer<SampleType>::handleModWheel (const int wheelValue)
{
    juce::ignoreUnused(wheelValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleBreathController (const int controlValue)
{
    juce::ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleFootController (const int controlValue)
{
    juce::ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handlePortamentoTime (const int controlValue)
{
    juce::ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleBalance (const int controlValue)
{
    juce::ignoreUnused(controlValue);
};

template<typename SampleType>
void Harmonizer<SampleType>::handleLegato (const bool isOn)
{
    juce::ignoreUnused(isOn);
};
