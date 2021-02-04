/*
     Part of module: bv_Harmonizer
     Direct parent file: bv_Harmonizer.h
     Classes: Harmonizer
*/


#include "bv_Harmonizer/bv_Harmonizer.h"


namespace bav

{
    
    
template<typename SampleType>
int Harmonizer<SampleType>::getNumActiveVoices() const noexcept
{
    int actives = 0;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive())
            ++actives;
    
    return actives;
};
    
    
template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::findFreeVoice (const bool stealIfNoneAvailable)
{
    for (auto* voice : voices)
        if (! voice->isVoiceActive())
            return voice;
    
    if (! stealIfNoneAvailable)
        return nullptr;

    auto* stolenVoice = findVoiceToSteal();
    
    if (stolenVoice == nullptr)
        return nullptr;
    
    if (stolenVoice->isVoiceActive())
        aggregateMidiBuffer.addEvent (MidiMessage::noteOff (lastMidiChannel, stolenVoice->getCurrentlyPlayingNote(), 1.0f),
                                      ++lastMidiTimeStamp);
    
    return stolenVoice;
};
    
    
template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::findVoiceToSteal() const
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
        
        // NB: Using a functor rather than a lambda here due to scare-stories about compilers generating code containing heap allocations...
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
    
    
template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getCurrentDescantVoice() const noexcept
{
    if (! descant.isOn)
        return nullptr;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive() && voice->isCurrentDescantVoice())
            return voice;
    
    return nullptr;
};


template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getCurrentPedalPitchVoice() const noexcept
{
    if (! pedal.isOn)
        return nullptr;
    
    for (auto* voice : voices)
        if (voice->isVoiceActive() && voice->isCurrentPedalVoice())
            return voice;
    
    return nullptr;
};
    
    
template<typename SampleType>
HarmonizerVoice<SampleType>* Harmonizer<SampleType>::getVoicePlayingNote (const int midiPitch) const noexcept
{
    for (auto* voice : voices)
        if (voice->isVoiceActive() && voice->getCurrentlyPlayingNote() == midiPitch)
            return voice;
    
    return nullptr;
};
    
    
template<typename SampleType>
void Harmonizer<SampleType>::addVoice (HarmonizerVoice<SampleType>* newVoice)
{
    voices.add (newVoice);
    
    panner.setNumberOfVoices (voices.size());
};
    
    
template<typename SampleType>
void Harmonizer<SampleType>::removeNumVoices (const int voicesToRemove)
{
    if (voicesToRemove == 0)
        return;
    
    const int shouldBeLeft = voices.size() - voicesToRemove;
    
    int voicesRemoved = 0;
    
    while (voicesRemoved < voicesToRemove)
    {
        if (voices.isEmpty())
            break;
        
        HarmonizerVoice<SampleType>* removing = nullptr;
        
        for (auto* voice : voices)
        {
            if (! voice->isVoiceActive())
            {
                removing = voice;
                break;
            }
        }
        
        if (removing == nullptr)
            removing = findVoiceToSteal();
        
        if (removing == nullptr)
            removing = voices[0];
        
        if (removing->isVoiceActive())
        {
            panner.panValTurnedOff (removing->getCurrentMidiPan());
            aggregateMidiBuffer.addEvent (MidiMessage::noteOff (lastMidiChannel, removing->getCurrentlyPlayingNote(), 1.0f),
                                          ++lastMidiTimeStamp);
        }
        
        voices.removeObject (removing, true);
        
        ++voicesRemoved;
    }
    
    jassert (voices.isEmpty() || voices.size() == shouldBeLeft);
    
    panner.setNumberOfVoices (std::max (voices.size(), 1));
};
    
    
};  // namespace
