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
        if ((voice->isVoiceActive() && ! voice->isKeyDown())
            && (includePedalPitchAndDescant || (! (voice->isCurrentPedalVoice() || voice->isCurrentDescantVoice())))
            && (overrideSostenutoPedal || ! voice->sustainingFromSostenutoPedal))
                { stopVoice (voice, velocity, allowTailOff); }
}
    
    
/***********************************************************************************************************************************************
 // functions for meta midi & note management --------------------------------------------------------------------------------------------------
***********************************************************************************************************************************************/

template<typename SampleType>
bool Harmonizer<SampleType>::isPitchActive (const int midiPitch, const bool countRingingButReleased, const bool countKeyUpNotes) const
{
    for (auto* voice : voices)
        if ((voice->isVoiceActive() && voice->getCurrentlyPlayingNote() == midiPitch)
            && (countRingingButReleased || ! voice->isPlayingButReleased())
            && (countKeyUpNotes || voice->isKeyDown()))
                { return true; }
    
    return false;
}


template<typename SampleType>
void Harmonizer<SampleType>::reportActiveNotes (Array<int>& outputArray,
                                                const bool includePlayingButReleased,
                                                const bool includeKeyUpNotes) const
{
    outputArray.clearQuick();
    
    for (auto* voice : voices)
        if ((voice->isVoiceActive())
            && (includePlayingButReleased || ! voice->isPlayingButReleased())
            && (includeKeyUpNotes || voice->isKeyDown()))
                { outputArray.add (voice->getCurrentlyPlayingNote()); }
    
    if (! outputArray.isEmpty())
        outputArray.sort();
}

    
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
}
    
    
template<typename SampleType>
void Harmonizer<SampleType>::processMidiEvent (const MidiMessage& m)
{
    handleMidiEvent (m, static_cast<int> (m.getTimeStamp()));
    pitchCollectionChanged();
}


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
}

    
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
}
    
    
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
}


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
    else  // same note retriggered: output aftertouch / channel pressure
    {
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
}


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
        }
        else
        {
            if (! (sustainPedalDown || voice->sustainingFromSostenutoPedal))
                stopVoice (voice, velocity, allowTailOff);
            else
                voice->setKeyDown (false);
        }
    }
    else  // this is an automated note-off event
    {
        if (! voice->isKeyDown()) // for automated note-off events, only actually stop the voice if its keyboard key isn't currently down
        {
            stopVoice (voice, velocity, allowTailOff);
        }
        else  // we're processing an automated note-off event, but the voice's keyboard key is still being held
        {
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
}


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
}


template<typename SampleType>
void Harmonizer<SampleType>::allNotesOff (const bool allowTailOff, const float velocity)
{
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            stopVoice (voice, velocity, allowTailOff);
    
    panner.reset();
}

    
/***********************************************************************************************************************************************
 // other midi events --------------------------------------------------------------------------------------------------------------------------
***********************************************************************************************************************************************/

template<typename SampleType>
void Harmonizer<SampleType>::handlePitchWheel (int wheelValue)
{
    wheelValue = jlimit (0, 127, wheelValue);
    
    if (lastPitchWheelValue == wheelValue)
        return;
    
    aggregateMidiBuffer.addEvent (MidiMessage::pitchWheel (lastMidiChannel, wheelValue),
                                  ++lastMidiTimeStamp);
    
    lastPitchWheelValue = wheelValue;
    bendTracker.newPitchbendRecieved (wheelValue);
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            voice->setCurrentOutputFreq (getOutputFrequency (voice->getCurrentlyPlayingNote()));
}


template<typename SampleType>
void Harmonizer<SampleType>::handleAftertouch (int midiNoteNumber, int aftertouchValue)
{
    if (midiNoteNumber < 0 || midiNoteNumber > 127)
        return;
    
    aftertouchValue = jlimit (0, 127, aftertouchValue);
    
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
}


template<typename SampleType>
void Harmonizer<SampleType>::handleChannelPressure (int channelPressureValue)
{
    channelPressureValue = jlimit (0, 127, channelPressureValue);
    
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
}

    
template<typename SampleType>
void Harmonizer<SampleType>::updateChannelPressure (int newIncomingAftertouch)
{
    newIncomingAftertouch = jlimit (0, 127, newIncomingAftertouch);
    
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
}
    

template<typename SampleType>
void Harmonizer<SampleType>::handleController (const int controllerNumber, int controllerValue)
{
    controllerValue = jlimit (0, 127, controllerValue);
    
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
}


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
}


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
        turnOffAllKeyupNotes (false, false, 0.0f, true);
    }
}


template<typename SampleType>
void Harmonizer<SampleType>::handleSoftPedal (const int value)
{
    const bool isDown = value >= 64;
    
    if (softPedalDown == isDown)
        return;
    
    softPedalDown = isDown;
    
    aggregateMidiBuffer.addEvent (MidiMessage::controllerEvent (lastMidiChannel, 0x43, value),
                                  ++lastMidiTimeStamp);
}


template<typename SampleType>
void Harmonizer<SampleType>::handleModWheel (const int wheelValue)
{
    ignoreUnused(wheelValue);
}

template<typename SampleType>
void Harmonizer<SampleType>::handleBreathController (const int controlValue)
{
    ignoreUnused(controlValue);
}

template<typename SampleType>
void Harmonizer<SampleType>::handleFootController (const int controlValue)
{
    ignoreUnused(controlValue);
}

template<typename SampleType>
void Harmonizer<SampleType>::handlePortamentoTime (const int controlValue)
{
    ignoreUnused(controlValue);
}

template<typename SampleType>
void Harmonizer<SampleType>::handleBalance (const int controlValue)
{
    ignoreUnused(controlValue);
}

template<typename SampleType>
void Harmonizer<SampleType>::handleLegato (const bool isOn)
{
    ignoreUnused(isOn);
}


} // namespace
