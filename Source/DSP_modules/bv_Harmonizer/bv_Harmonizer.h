/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_Harmonizer
 vendor:             Ben Vining
 version:            0.0.1
 name:               Harmonizer
 description:        base class for a polyphonic real-time pitch shifting instrument
 dependencies:       juce_audio_utils, bv_PitchDetector, bv_GeneralUtils
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


// class templates defined in this file: Harmonizer<SampleType>, HarmonizerVoice<SampleType>


#pragma once

// dependencies
#include <juce_audio_utils/juce_audio_utils.h>
#include "bv_GeneralUtils/bv_GeneralUtils.h"

// the rest of the harmonizer module
#include "bv_Harmonizer/bv_HarmonizerUtilities.h"
#include "bv_Harmonizer/PanningManager/PanningManager.h"
#include "bv_Harmonizer/GrainExtractor/GrainExtractor.h"


#ifndef BV_HARMONIZER_USE_VDSP
    #if (JUCE_MAC || JUCE_IOS)
        #define BV_HARMONIZER_USE_VDSP 1
    #else
        #define BV_HARMONIZER_USE_VDSP 0
    #endif
#endif

#ifdef BV_PITCH_DETECTOR_USE_VDSP
    #undef BV_PITCH_DETECTOR_USE_VDSP
#endif

#if BV_HARMONIZER_USE_VDSP
    #define BV_PITCH_DETECTOR_USE_VDSP 1
#else
    #define BV_PITCH_DETECTOR_USE_VDSP 0
#endif

#include "bv_PitchDetector/bv_PitchDetector.h"



namespace bav

{
    
using namespace juce;
    
    

template<typename SampleType>
class Harmonizer; // forward declaration...



/*
 HarmonizerVoice : represents a "voice" that the Harmonizer can use to generate one monophonic note. A voice plays a single note/sound at a time; the Harmonizer holds an array of voices so that it can play polyphonically.
*/
    
template<typename SampleType>
class HarmonizerVoice
{
public:
    
    // NB. I play a bit fast and loose with private vs public functions here, because really, you should never interface directly with any non-const methods of HarmonizerVoice from outside the Harmonizer class that owns it...
    
    HarmonizerVoice (Harmonizer<SampleType>* h);
    
    ~HarmonizerVoice();
    
    void renderNextBlock (const AudioBuffer<SampleType>& inputAudio, AudioBuffer<SampleType>& outputBuffer,
                          const int origPeriod, const Array<int>& indicesOfGrainOnsets,
                          const AudioBuffer<SampleType>& windowToUse);
    
    void prepare (const int blocksize);
    
    void releaseResources();
    
    int getCurrentlyPlayingNote() const noexcept { return currentlyPlayingNote; }
    
    bool isVoiceActive() const noexcept { return (currentlyPlayingNote >= 0); }
    
    bool isPlayingButReleased()   const noexcept { return playingButReleased; } // returns true if a voice is sounding, but its key has been released
    
    // Returns true if this voice started playing its current note before the other voice did.
    bool wasStartedBefore (const HarmonizerVoice& other) const noexcept { return noteOnTime < other.noteOnTime; }
    
    // Returns true if the key that triggered this voice is still held down. Note that the voice may still be playing after the key was released (e.g because the sostenuto pedal is down).
    bool isKeyDown() const noexcept { return keyIsDown; }
    
    int getCurrentMidiPan() const noexcept { return panner.getLastMidiPan(); }
    
    // DANGER!!! FOR NON REALTIME USE ONLY!
    void increaseBufferSizes (const int newMaxBlocksize);
    
    void clearBuffers();
    
    float getLastRecievedVelocity() const noexcept { return lastRecievedVelocity; }
    
    void updateSampleRate (const double newSamplerate);
    
    bool isCurrentPedalVoice()   const noexcept { return isPedalPitchVoice; }
    bool isCurrentDescantVoice() const noexcept { return isDescantVoice; }
    
    
protected:
    
    //  These functions will be called by the Harmonizer object that owns this voice
    
    void startNote (const int midiPitch,  const float velocity,
                    const uint32 noteOnTimestamp,
                    const bool keyboardKeyIsDown = true,
                    const bool isPedal = false, const bool isDescant = false);
    
    void stopNote (const float velocity, const bool allowTailOff);
    
    void aftertouchChanged (const int newAftertouchValue);
    
    void setVelocityMultiplier (const float newMultiplier) noexcept { currentVelocityMultiplier = newMultiplier; }
    
    void setCurrentOutputFreq (const float newFreq) noexcept { currentOutputFreq = newFreq; }
    
    void setKeyDown (bool isNowDown) noexcept;
    
    void setPan (int newPan);
    
    void setAdsrParameters (const ADSR::Parameters newParams) { adsr.setParameters(newParams); }
    void setQuickReleaseParameters (const ADSR::Parameters newParams) { quickRelease.setParameters(newParams); }
    void setQuickAttackParameters  (const ADSR::Parameters newParams) { quickAttack.setParameters(newParams); }
    
    
private:
    
    friend class Harmonizer<SampleType>;
    
    void clearCurrentNote(); // this function resets the voice's internal state & marks it as avaiable to accept a new note
    
    void sola (const SampleType* input, const int totalNumInputSamples,
               const int origPeriod, const int newPeriod, const Array<int>& indicesOfGrainOnsets,
               const SampleType* window);
    
    void olaFrame (const SampleType* inputAudio, const int frameStartSample, const int frameEndSample,
                   const SampleType* window, const int newPeriod);
    
    void moveUpSamples (const int numSamplesUsed);
    
    ADSR adsr;         // the main/primary ADSR driven by MIDI input to shape the voice's amplitude envelope. May be turned off by the user.
    ADSR quickRelease; // used to quickly fade out signal when stopNote() is called with the allowTailOff argument set to false, instead of jumping signal to 0
    ADSR quickAttack;  // used for if normal ADSR user toggle is OFF, to prevent jumps/pops at starts of notes.
    
    Harmonizer<SampleType>* parent; // this is a pointer to the Harmonizer object that controls this HarmonizerVoice
    
    int currentlyPlayingNote;
    float currentOutputFreq;
    uint32 noteOnTime;
    
    float currentVelocityMultiplier, prevVelocityMultiplier;
    float lastRecievedVelocity;
    
    bool isQuickFading;
    bool noteTurnedOff;
    
    bool keyIsDown;
    
    int currentAftertouch;
    
    AudioBuffer<SampleType> synthesisBuffer; // mono buffer that this voice's synthesized samples are written to
    int nextSBindex; // highest synthesis buffer index written to + 1
    AudioBuffer<SampleType> copyingBuffer;
    AudioBuffer<SampleType> windowingBuffer; // used to apply the window to the analysis grains before OLA, so windowing only needs to be done once per analysis grain
    
    float prevSoftPedalMultiplier;
    
    Panner panner;
    
    bool isPedalPitchVoice, isDescantVoice;
    
    bool playingButReleased;
    
    float lastPBRmult = 1.0f;
    
    bool sustainingFromSostenutoPedal = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonizerVoice)
};


/***********************************************************************************************************************************************
***********************************************************************************************************************************************/

/*
    Harmonizer: base class for the polyphonic instrument owning & managing a collection of HarmonizerVoices
*/

template<typename SampleType>
class Harmonizer
{
public:
    Harmonizer();
    
    ~Harmonizer();
    
    void initialize (const int initNumVoices, const double initSamplerate, const int initBlocksize);
    
    void prepare (const int blocksize);
    
    void releaseResources();
    
    void clearBuffers();
    
    void renderVoices (const AudioBuffer<SampleType>& inputAudio,
                       AudioBuffer<SampleType>& outputBuffer,
                       MidiBuffer& midiMessages);
    
    void processMidi (MidiBuffer& midiMessages);
    
    void processMidiEvent (const MidiMessage& m);
    
    void playChord (const Array<int>& desiredPitches,
                    const float velocity = 1.0f,
                    const bool allowTailOffOfOld = false);
    
    void playIntervalSet (const Array<int>& desiredIntervals,
                          const float velocity = 1.0f,
                          const bool allowTailOffOfOld = false,
                          const bool isIntervalLatch = false);
    
    void allNotesOff (const bool allowTailOff, const float velocity = 1.0f);
    
    void turnOffAllKeyupNotes (const bool allowTailOff,  
                               const bool includePedalPitchAndDescant,
                               const float velocity,
                               const bool overrideSostenutoPedal);
    
    bool isPitchActive (const int midiPitch, const bool countRingingButReleased = false, const bool countKeyUpNotes = false) const;
    
    void reportActiveNotes (Array<int>& outputArray,
                            const bool includePlayingButReleased = false,
                            const bool includeKeyUpNotes = true) const;
    
    int getNumActiveVoices() const;
    
    int getNumVoices() const noexcept { return voices.size(); }
    
    // adds a specified # of voices
    void addNumVoices (const int voicesToAdd);
    
    // removes a specified # of voices, attempting to remove inactive voices first, and only removes active voices if necessary
    void removeNumVoices (const int voicesToRemove);
    
    void setNoteStealingEnabled (const bool shouldSteal) noexcept { shouldStealNotes.store(shouldSteal); }
    
    void shouldUseChannelPressure (const bool shouldUse) noexcept { useChannelPressure = shouldUse; }
    
    void updateMidiVelocitySensitivity (int newSensitivity);
    void updatePitchbendSettings (const int rangeUp, const int rangeDown);
    void setSoftPedalGainMultiplier (const float newGain) { softPedalMultiplier.store(newGain); }
    void setAftertouchGainOnOff (const bool shouldBeOn) { aftertouchGainIsOn = shouldBeOn; }
    
    void setPlayingButReleasedGain (const float newMultiplier) { playingButReleasedMultiplier = newMultiplier; }
    
    void setPedalPitch (const bool isOn);
    void setPedalPitchUpperThresh (int newThresh);
    void setPedalPitchInterval (const int newInterval);
    
    void setDescant (const bool isOn);
    void setDescantLowerThresh (int newThresh);
    void setDescantInterval (const int newInterval);
    
    void setCurrentPlaybackSampleRate (const double newRate);
    
    void setConcertPitchHz (const int newConcertPitchhz);
    
    void updateStereoWidth (int newWidth);
    void updateLowestPannedNote (int newPitchThresh);
    
    void setMidiLatch (const bool shouldBeOn, const bool allowTailOff);
    bool isLatched()  const noexcept { return latchIsOn; }
    
    void setIntervalLatch (const bool shouldBeOn, const bool allowTailOff);
    bool isIntervalLatchOn() const noexcept { return intervalLatchIsOn; }
    
    void updateADSRsettings (const float attack, const float decay, const float sustain, const float release);
    void setADSRonOff (const bool shouldBeOn) noexcept { adsrIsOn.store(shouldBeOn); }
    void updateQuickReleaseMs (const int newMs);
    void updateQuickAttackMs  (const int newMs);

    void updatePitchDetectionHzRange (const int minHz, const int maxHz) { pitchDetector.setHzRange (minHz, maxHz); }
    
    void updatePitchDetectionConfidenceThresh (const float newUpperThresh, const float newLowerThresh)
        { pitchDetector.setConfidenceThresh (static_cast<SampleType>(newUpperThresh), static_cast<SampleType>(newLowerThresh)); }
    
    
protected:
    
    // these functions will be called by the harmonizer's voices, to query for important harmonizer-wide info
    
    // returns a float velocity weighted according to the current midi velocity sensitivity settings
    float getWeightedVelocity (const float inputFloatVelocity) const
    {
        return velocityConverter.floatVelocity(inputFloatVelocity);
    }
    
    // returns the actual frequency in Hz a HarmonizerVoice needs to output for its latest recieved midiNote, as an integer -- weighted for pitchbend with the current settings & pitchwheel position, then converted to frequency with the current concert pitch settings.
    float getOutputFrequency (const int midipitch) const
    {
        return pitchConverter.mtof (bendTracker.newNoteRecieved(midipitch));
    }
    
    bool isSustainPedalDown()   const noexcept { return sustainPedalDown;   }
    bool isSostenutoPedalDown() const noexcept { return sostenutoPedalDown; }
    bool isSoftPedalDown()      const noexcept { return softPedalDown;      }
    float getSoftPedalMultiplier() const noexcept { return softPedalMultiplier.load(); }
    float getPlayingButReleasedMultiplier() const noexcept { return playingButReleasedMultiplier; }
    bool isAftertouchGainOn() const noexcept { return aftertouchGainIsOn; }
    
    bool isADSRon() const noexcept { return adsrIsOn.load(); }
    ADSR::Parameters getCurrentAdsrParams() const noexcept { return adsrParams; }
    ADSR::Parameters getCurrentQuickReleaseParams() const noexcept { return quickReleaseParams; }
    ADSR::Parameters getCurrentQuickAttackParams()  const noexcept { return quickAttackParams; }
    
    double getSamplerate() const noexcept { return sampleRate; }
    
    
private:
    
    void setCurrentInputFreq (const float newInputFreq);
    
    void numVoicesChanged();
    
    // MIDI
    void handleMidiEvent (const MidiMessage& m, const int samplePosition);
    void noteOn (const int midiPitch, const float velocity, const bool isKeyboard = true);
    void noteOff (const int midiNoteNumber, const float velocity, const bool allowTailOff, const bool isKeyboard = true);
    void handlePitchWheel (int wheelValue);
    void handleAftertouch (int midiNoteNumber, int aftertouchValue);
    void handleChannelPressure (int channelPressureValue);
    void updateChannelPressure (int newIncomingAftertouch);
    void handleController (const int controllerNumber, int controllerValue);
    void handleSustainPedal (const int value);
    void handleSostenutoPedal (const int value);
    void handleSoftPedal (const int value);
    void handleModWheel (const int wheelValue);
    void handleBreathController (const int controlValue);
    void handleFootController (const int controlValue);
    void handlePortamentoTime (const int controlValue);
    void handleBalance (const int controlValue);
    void handleLegato (const bool isOn);
    
    void startVoice (HarmonizerVoice<SampleType>* voice, const int midiPitch, const float velocity, const bool isKeyboard);
    void stopVoice  (HarmonizerVoice<SampleType>* voice, const float velocity, const bool allowTailOff);
    
    void turnOnList  (const Array<int>& toTurnOn,  const float velocity, const bool partOfChord = false);
    void turnOffList (const Array<int>& toTurnOff, const float velocity, const bool allowTailOff, const bool partOfChord = false);
    
    // this function is called any time the collection of pitches is changed (ie, with regular keyboard input, on each note on/off, or for chord input, once after each chord is triggered). Used for things like pedal pitch, descant, etc
    void pitchCollectionChanged();
    
    void applyPedalPitch();
    void applyDescant();
    
    // voice allocation
    HarmonizerVoice<SampleType>* findFreeVoice (const bool stealIfNoneAvailable);
    HarmonizerVoice<SampleType>* findVoiceToSteal();
    HarmonizerVoice<SampleType>* getVoicePlayingNote (const int midiPitch) const;
    HarmonizerVoice<SampleType>* getCurrentDescantVoice() const;
    HarmonizerVoice<SampleType>* getCurrentPedalPitchVoice() const;
    
    void fillWindowBuffer (const int numSamples);
    
    
    // *** //
    
    friend class HarmonizerVoice<SampleType>;
    
    CriticalSection lock;
    
    OwnedArray< HarmonizerVoice<SampleType> > voices;
    
    PitchDetector<SampleType> pitchDetector;
    
    GrainExtractor<SampleType> grains;
    Array<int> indicesOfGrainOnsets;
    
    // the arbitrary "period" imposed on the signal for analysis for unpitched frames of audio will be randomized within this range
    // NB max value should be 1 greater than the largest possible generated number 
    const Range<int> unpitchedArbitraryPeriodRange = Range<int> (50, 201);
    
    bool latchIsOn;
    
    bool intervalLatchIsOn;
    Array<int> intervalsLatchedTo;
    void updateIntervalsLatchedTo();
    
    ADSR::Parameters adsrParams;
    ADSR::Parameters quickReleaseParams;
    ADSR::Parameters quickAttackParams;
    
    float currentInputFreq;
    int currentInputPeriod;
    
    double sampleRate;
    uint32 lastNoteOnCounter;
    int lastPitchWheelValue;
    
    std::atomic<bool> shouldStealNotes;
    
    // *********************************
    
    // for clarity & cleanliness, the individual descant & pedal preferences are each encapsulated into their own struct:
    
    struct pedalPitchPrefs
    {
        bool isOn;
        int lastPitch;
        int upperThresh; // pedal pitch has an UPPER thresh - the auto harmony voice is only activated if the lowest keyboard note is BELOW a certain thresh
        int interval;
    };
    
    struct descantPrefs
    {
        bool isOn;
        int lastPitch;
        int lowerThresh; // descant has a LOWER thresh - the auto harmony voice is only activated if the highest keyboard note is ABOVE a certain thresh
        int interval;
    };
    
    pedalPitchPrefs pedal;
    descantPrefs descant;
    
    // *********************************
    
    PanningManager  panner;
    std::atomic<int> lowestPannedNote;
    
    VelocityHelper  velocityConverter;
    PitchConverter  pitchConverter;
    PitchBendHelper bendTracker;
    
    std::atomic<bool> adsrIsOn;
    
    MidiBuffer aggregateMidiBuffer; // this midi buffer will be used to collect the harmonizer's aggregate MIDI output
    int lastMidiTimeStamp;
    int lastMidiChannel;
    bool useChannelPressure;  // all the voices will keep track of & respond to their individual aftertouch values by default; if this is true then the harmonizer will also output an aggregate "channel pressure", which will be the maximum of any voice's recived aftertouch value.
    
    bool aftertouchGainIsOn;
    
    float playingButReleasedMultiplier = 1.0f;
    
    bool sustainPedalDown, sostenutoPedalDown, softPedalDown;
    
    std::atomic<float> softPedalMultiplier; // the multiplier by which each voice's output will be multiplied when the soft pedal is down
    
    AudioBuffer<SampleType> windowBuffer;
    int windowSize;
    
    AudioBuffer<SampleType> polarityReversalBuffer;
    
    Array< HarmonizerVoice<SampleType>* > usableVoices; // this array is used to sort the voices when a 'steal' is requested
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};


} // namespace
