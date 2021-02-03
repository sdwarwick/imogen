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
                                                   const bool includePedalPitchAndDescant)
{
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
    
        const int currentMidiPitch = juce::roundToInt (pitchConverter.ftom (currentInputFreq));
        
        Array<int> intervalLatchNotes;
        intervalLatchNotes.ensureStorageAllocated (intervalsLatchedTo.size());
        
        for (int interval : intervalsLatchedTo)
            intervalLatchNotes.add (currentMidiPitch + interval);
        
        const float velocity = allowTailOff ? 0.0f : 1.0f;
        
        for (auto* voice : voices)
            if (voice->isVoiceActive() && ! voice->isKeyDown())
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
        turnOffAllKeyupNotes (allowTailOff, false);
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
void Harmonizer<SampleType>::playChordFromIntervalSet (const Array<int>& desiredIntervals)
{
    if (desiredIntervals.isEmpty())
    {
        allNotesOff (false);
        return;
    }
    
    const float currentInputPitch = pitchConverter.ftom (currentInputFreq);
    
    Array<int> desiredNotes;
    desiredNotes.ensureStorageAllocated (desiredIntervals.size());
    
    for (int interval : desiredIntervals)
        desiredNotes.add (roundToInt (currentInputPitch + interval));
    
    playChord (desiredNotes, 1.0f, false);
};


// play chord: send an array of midi pitches into this function and it will ensure that only those desired pitches are being played.
template<typename SampleType>
void Harmonizer<SampleType>::playChord (const Array<int>& desiredPitches,
                                        const float velocity,
                                        const bool allowTailOffOfOld,
                                        const bool isIntervalLatch)
{
    if (desiredPitches.isEmpty())
    {
        allNotesOff (allowTailOffOfOld);
        return;
    }
    
    // create array containing current pitches
    
    Array<int> currentNotes;
    currentNotes.ensureStorageAllocated (voices.size());
    
    reportActiveNotes (currentNotes, false);
    
    // 1. turn off the pitches that were previously on that are not included in the list of desired pitches
    
    if (! currentNotes.isEmpty())
    {
        Array<int> toTurnOff;
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
        Array<int> toTurnOn;
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
void Harmonizer<SampleType>::turnOnList (const Array<int>& toTurnOn, const float velocity, const bool partOfChord)
{
    if (toTurnOn.isEmpty())
        return;
    
    for (int note : toTurnOn)
        noteOn (note, velocity, false);
    
    if (! partOfChord)
        pitchCollectionChanged();
};


// turn off list: turns off a list of specified notes in quick sequence.
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
    
    if ((currentLowest == 128) || (currentLowest > pedal.upperThresh))
    {
        if (pedal.lastPitch > -1)
            noteOff (pedal.lastPitch, 1.0f, false, false);
        
        return;
    }
    
    const int newPedalPitch = currentLowest - pedal.interval;
    
    if (newPedalPitch == pedal.lastPitch)
        return;
    
    if (pedal.lastPitch > -1)
        noteOff (pedal.lastPitch, 1.0f, false, false);
    
    if (newPedalPitch < 0)
        return;
    
    pedal.lastPitch = newPedalPitch;
    
    float velocity;
    
    if (auto* voiceCopying = getVoicePlayingNote (currentLowest))
        velocity = voiceCopying->getLastRecievedVelocity();
    else
        velocity = 1.0f;
    
    noteOn (newPedalPitch, velocity, false);
};


// automated midi "descant": creates a polyphonic doubling of the highest note currently being played by a keyboard key at a specified interval above that keyboard key, IF that keyboard key is above a certain pitch threshold.
template<typename SampleType>
void Harmonizer<SampleType>::applyDescant()
{
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
    
    if ((currentHighest == -1) || (currentHighest < descant.lowerThresh))
    {
        if (descant.lastPitch > -1)
            noteOff (descant.lastPitch, 1.0f, false, false);
        
        return;
    }
    
    const int newDescantPitch = currentHighest + descant.interval;
    
    if (newDescantPitch == descant.lastPitch)
        return;
    
    if (descant.lastPitch > -1)
        noteOff (descant.lastPitch, 1.0f, false, false);
    
    if (newDescantPitch > 127)
        return;
    
    descant.lastPitch = newDescantPitch;
    
    float velocity;
    
    if (auto* voiceCopying = getVoicePlayingNote (currentHighest))
        velocity = voiceCopying->getLastRecievedVelocity();
    else
        velocity = 1.0f;
    
    noteOn (newDescantPitch, velocity, false);
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
    
    //  I don't see a useful reason to allow multiple instances of the same midi note to be retriggered:
    if (isPitchActive (midiPitch, false))
    {
        if (pedal.isOn && midiPitch == pedal.lastPitch)
            pedal.lastPitch = -1;
        
        if (descant.isOn && midiPitch == descant.lastPitch)
            descant.lastPitch = -1;
        
        return;
    }
    
    bool isStealing = isKeyboard ? shouldStealNotes : false; // never steal voices for automated note events, only for keyboard triggered events
    
    startVoice (findFreeVoice (midiPitch, isStealing), midiPitch, velocity, isKeyboard);
};


template<typename SampleType>
void Harmonizer<SampleType>::startVoice (HarmonizerVoice<SampleType>* voice, const int midiPitch, const float velocity, const bool isKeyboard)
{
    if (voice == nullptr)
    {
        // this function will be called with a null voice ptr if a note on event was requested, but a voice was not available (ie, could not be stolen)
        
        if (pedal.isOn && midiPitch == pedal.lastPitch)
            pedal.lastPitch = -1;
            
        if (descant.isOn && midiPitch == descant.lastPitch)
            descant.lastPitch = -1;
        
        return;
    }
    
    aggregateMidiBuffer.addEvent (MidiMessage::noteOn (lastMidiChannel, midiPitch, velocity),
                                  ++lastMidiTimeStamp);
    
    const bool wasStolen = voice->isVoiceActive(); // we know the voice is being "stolen" from another note if it was already on before getting this start command
    
    if (! voice->isKeyDown())           // if the key wasn't already marked as down...
        voice->setKeyDown (isKeyboard); // then mark it as down IF this start command is because of a keyboard event
    
    
    if (midiPitch < lowestPannedNote)
    {
        if (wasStolen)
            panner.panValTurnedOff (voice->getCurrentMidiPan());
        
        voice->setPan (64);
    }
    else if (! wasStolen) // don't change pan if voice was stolen
        voice->setPan (panner.getNextPanVal());
    
    
    const bool isPedal   = pedal.isOn   ? (midiPitch == pedal.lastPitch)   : false;
    const bool isDescant = descant.isOn ? (midiPitch == descant.lastPitch) : false;
    
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
    {
        if (pedal.isOn && midiNoteNumber == pedal.lastPitch)
            pedal.lastPitch = -1;
        
        if (descant.isOn && midiNoteNumber == descant.lastPitch)
            descant.lastPitch = -1;
        
        return;
    }
    
    if (isKeyboard)
    {
        voice->setKeyDown (false);
        
        if (latchIsOn)
            return;
        
        if (! (sustainPedalDown || sostenutoPedalDown))
            stopVoice (voice, velocity, allowTailOff);
    }
    else if (! voice->isKeyDown()) // for automated note-off events, only actually stop the voice if its keyboard key isn't currently down
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
    
    aggregateMidiBuffer.addEvent (MidiMessage::noteOff (lastMidiChannel, note, velocity),
                                  ++lastMidiTimeStamp);
    
    if (pedal.isOn && voice->isCurrentPedalVoice())
        pedal.lastPitch = -1;
    
    if (descant.isOn && voice->isCurrentDescantVoice())
        descant.lastPitch = -1;
    
    voice->stopNote (velocity, allowTailOff);
};


template<typename SampleType>
void Harmonizer<SampleType>::allNotesOff (const bool allowTailOff)
{
    const float velocity = allowTailOff ? 0.0f : 1.0f;
    
    for (auto* voice : voices)
    {
        voice->setKeyDown (false);
        
        if (voice->isVoiceActive())
            stopVoice (voice, velocity, allowTailOff);
    }
    
    panner.reset (false);
};


// VOICE ALLOCATION ---------------------------------------------------------------------------------------------------------------------------------

template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::findFreeVoice (const int midiNoteNumber, const bool stealIfNoneAvailable)
{
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
    Array< HarmonizerVoice<SampleType>* > usableVoices;
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
    
    aggregateMidiBuffer.addEvent (MidiMessage::pitchWheel (lastMidiChannel, wheelValue),
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
    aggregateMidiBuffer.addEvent (MidiMessage::aftertouchChange (lastMidiChannel, midiNoteNumber, aftertouchValue),
                                  ++lastMidiTimeStamp);
    
    for (auto* voice : voices)
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
            voice->aftertouchChanged (aftertouchValue);
};


template<typename SampleType>
void Harmonizer<SampleType>::handleChannelPressure (const int channelPressureValue)
{
    aggregateMidiBuffer.addEvent (MidiMessage::channelPressureChange (lastMidiChannel, channelPressureValue),
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
    
    aggregateMidiBuffer.addEvent (MidiMessage::controllerEvent (lastMidiChannel, 0x40, value),
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
    
    aggregateMidiBuffer.addEvent (MidiMessage::controllerEvent (lastMidiChannel, 0x42, value),
                                  ++lastMidiTimeStamp);
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


}; // namespace
