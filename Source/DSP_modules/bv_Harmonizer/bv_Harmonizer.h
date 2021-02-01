/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 bv_Harmonizer
 vendor:             Ben Vining
 version:            0.0.1
 name:               Harmonizer
 description:        base class for a polyphonic real-time pitch shifting instrument
 dependencies:       juce_audio_utils, bv_GeneralUtils
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/

#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "bv_GeneralUtils/bv_GeneralUtils.h"

#include "bv_Harmonizer/bv_HarmonizerUtilities.h"
#include "bv_Harmonizer/PanningManager/PanningManager.h"
#include "bv_Harmonizer/GrainExtractor/GrainExtractor.h"


template<typename SampleType>
class Harmonizer; // forward declaration...



/*
 HarmonizerVoice : represents a "voice" that the Harmonizer can use to generate one monophonic note. A voice plays a single note/sound at a time; the Harmonizer holds an array of voices so that it can play polyphonically.
 */
template<typename SampleType>
class HarmonizerVoice
{
public:
    HarmonizerVoice(Harmonizer<SampleType>* h);
    
    ~HarmonizerVoice();
    
    void renderNextBlock (const juce::AudioBuffer<SampleType>& inputAudio, juce::AudioBuffer<SampleType>& outputBuffer,
                          const int origPeriod, const juce::Array<int>& indicesOfGrainOnsets,
                          const juce::AudioBuffer<SampleType>& windowToUse);
    
    void prepare (const int blocksize);
    
    void releaseResources();
    
    
    int getCurrentlyPlayingNote() const noexcept { return currentlyPlayingNote; }
    
    bool isVoiceActive()          const noexcept { return currentlyPlayingNote >= 0; }
    
    bool isPlayingButReleased()   const noexcept { return playingButReleased; } // returns true if a voice is sounding, but its key has been released
    
    // Returns true if this voice started playing its current note before the other voice did.
    bool wasStartedBefore (const HarmonizerVoice& other) const noexcept { return noteOnTime < other.noteOnTime; }
    
    // Returns true if the key that triggered this voice is still held down. Note that the voice may still be playing after the key was released (e.g because the sostenuto pedal is down).
    bool isKeyDown() const noexcept { return keyIsDown; }
    void setKeyDown (bool isNowDown) noexcept;
    
    void setPan (const int newPan);
    int getCurrentMidiPan() const noexcept { return currentMidipan; }
    
    void startNote (const int midiPitch,  const float velocity,
                    const juce::uint32 noteOnTimestamp,
                    const bool wasStolen = false,
                    const bool isPedal = false, const bool isDescant = false);
    
    void stopNote (const float velocity, const bool allowTailOff);
    
    void aftertouchChanged(const int newAftertouchValue);
    
    // DANGER!!! FOR NON REALTIME USE ONLY!
    void increaseBufferSizes(const int newMaxBlocksize);
    
    juce::uint32 getNoteOnTime() const noexcept { return noteOnTime; }
    
    void clearBuffers();
    
    void setVelocityMultiplier(const float newMultiplier) noexcept { currentVelocityMultiplier = newMultiplier; }
    float getLastRecievedVelocity() const noexcept { return lastRecievedVelocity; }
    
    void setCurrentOutputFreq(const float newFreq) noexcept { currentOutputFreq = newFreq; }
    
    void updateSampleRate(const double newSamplerate);
    
    void setAdsrParameters(const juce::ADSR::Parameters newParams) { adsr.setParameters(newParams); }
    void setQuickReleaseParameters(const juce::ADSR::Parameters newParams) { quickRelease.setParameters(newParams); }
    void setQuickAttackParameters (const juce::ADSR::Parameters newParams) { quickAttack.setParameters(newParams); }
    
    bool isCurrentPedalVoice() const noexcept { return isPedalPitchVoice; }
    bool isCurrentDescantVoice() const noexcept { return isDescantVoice; }
    
    
private:
    
    void clearCurrentNote(); // this function resets the voice's internal state & marks it as avaiable to accept a new note
    
    void sola (const juce::AudioBuffer<SampleType>& inputAudio,
               const int origPeriod, const int newPeriod, const juce::Array<int>& indicesOfGrainOnsets,
               const SampleType* window);
    
    void olaFrame (const SampleType* inputAudio, const int frameStartSample, const int frameEndSample,
                   const SampleType* window, const int newPeriod);
    
    void moveUpSamples (const int numSamplesUsed);
    
    juce::ADSR adsr;         // the main/primary ADSR driven by MIDI input to shape the voice's amplitude envelope. May be turned off by the user.
    juce::ADSR quickRelease; // used to quickly fade out signal when stopNote() is called with the allowTailOff argument set to false, instead of jumping signal to 0
    juce::ADSR quickAttack;  // used for if normal ADSR user toggle is OFF, to prevent jumps/pops at starts of notes.
    
    Harmonizer<SampleType>* parent; // this is a pointer to the Harmonizer object that controls this HarmonizerVoice
    
    int currentlyPlayingNote;
    float currentOutputFreq;
    juce::uint32 noteOnTime;
    int currentMidipan;
    
    float currentVelocityMultiplier, prevVelocityMultiplier;
    float lastRecievedVelocity;
    
    bool isQuickFading;
    bool noteTurnedOff;
    
    bool keyIsDown;
    float panningMults[2];
    float prevPanningMults[2];
    
    int currentAftertouch;
    
    juce::AudioBuffer<SampleType> synthesisBuffer; // mono buffer that this voice's synthesized samples are written to
    int nextSBindex; // highest synthesis buffer index written to + 1
    juce::AudioBuffer<SampleType> copyingBuffer;
    juce::AudioBuffer<SampleType> windowingBuffer;
    
    float prevSoftPedalMultiplier;
    
    Panner panner;
    
    bool isPedalPitchVoice, isDescantVoice;
    
    bool playingButReleased;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonizerVoice)
};


/****************************************************************************************************************************************************
****************************************************************************************************************************************************/


template<typename SampleType>
class Harmonizer
{
public:
    Harmonizer();
    
    ~Harmonizer();
    
    void renderVoices (const juce::AudioBuffer<SampleType>& inputAudio,
                       juce::AudioBuffer<SampleType>& outputBuffer,
                       const float inputFrequency, const bool frameIsPitched,
                       juce::MidiBuffer& midiMessages);
    
    void prepare (const int blocksize);
    
    void releaseResources();
    
    void clearBuffers();
    
    int getNumActiveVoices() const;
    
    bool isPitchActive(const int midiPitch, const bool countRingingButReleased) const;
    bool isPitchHeldByKeyboardKey (const int midipitch) const;
    
    void updateMidiVelocitySensitivity(const int newSensitivity);
    
    void setCurrentPlaybackSampleRate(const double newRate);
    double getSamplerate() const noexcept { return sampleRate; }
    
    void setConcertPitchHz(const int newConcertPitchhz);
    
    void updateStereoWidth(const int newWidth);
    void updateLowestPannedNote(const int newPitchThresh) noexcept;
    
    void setNoteStealingEnabled (const bool shouldSteal) noexcept { shouldStealNotes = shouldSteal; }
    
    void reportActiveNotes(juce::Array<int>& outputArray) const; // returns an array of the currently active pitches
    void reportActivesNoReleased(juce::Array<int>& outputArray) const; // the same, but excludes notes that are still ringing but whose key has been released
    
    // turn off all notes
    void allNotesOff(const bool allowTailOff);
    
    // takes a list of desired pitches & sends the appropriate note & note off messages in sequence to leave only the desired notes playing.
    void playChord (const juce::Array<int>& desiredPitches, const float velocity, const bool allowTailOffOfOld, const bool isIntervalLatch = false);
    
    void setMidiLatch (const bool shouldBeOn, const bool allowTailOff);
    bool isLatched()  const noexcept { return latchIsOn; }
    
    void setIntervalLatch (const bool shouldBeOn, const bool allowTailOff);
    bool isIntervalLatchOn() const noexcept { return intervalLatchIsOn; }
    
    void updateADSRsettings(const float attack, const float decay, const float sustain, const float release);
    void setADSRonOff(const bool shouldBeOn) noexcept{ adsrIsOn = shouldBeOn; }
    bool isADSRon() const noexcept { return adsrIsOn; }
    void updateQuickReleaseMs(const int newMs);
    void updateQuickAttackMs(const int newMs);
    juce::ADSR::Parameters getCurrentAdsrParams() const noexcept { return adsrParams; }
    juce::ADSR::Parameters getCurrentQuickReleaseParams() const noexcept { return quickReleaseParams; }
    juce::ADSR::Parameters getCurrentQuickAttackParams()  const noexcept { return quickAttackParams; }
    
    void updatePitchbendSettings(const int rangeUp, const int rangeDown);
    
    // Adds a new voice to the harmonizer. The object passed in will be managed by the synthesiser, which will delete it later on when no longer needed. The caller should not retain a pointer to the voice.
    HarmonizerVoice<SampleType>* addVoice(HarmonizerVoice<SampleType>* newVoice);
    
    // removes a specified # of voices, attempting to remove inactive voices first
    void removeNumVoices(const int voicesToRemove);
    
    int getNumVoices() const noexcept { return voices.size(); }
    
    void setPedalPitch(const bool isOn);
    void setPedalPitchUpperThresh(const int newThresh);
    void setPedalPitchInterval(const int newInterval);
    HarmonizerVoice<SampleType>* getCurrentPedalPitchVoice() const;
    
    void setDescant(const bool isOn);
    void setDescantLowerThresh(const int newThresh);
    void setDescantInterval(const int newInterval);
    HarmonizerVoice<SampleType>* getCurrentDescantVoice() const;
    
    // returns a float velocity weighted according to the current midi velocity sensitivity settings
    float getWeightedVelocity (const float inputFloatVelocity) const { return velocityConverter.floatVelocity(inputFloatVelocity); }
    
    // returns the actual frequency in Hz a HarmonizerVoice needs to output for its latest recieved midiNote, as an integer -- weighted for pitchbend with the current settings & pitchwheel position, then converted to frequency with the current concert pitch settings.
    float getOutputFrequency (const int midipitch) const { return pitchConverter.mtof (bendTracker.newNoteRecieved(midipitch)); }
    
    // DANGER!!! FOR NON REAL TIME USE ONLY!!!
    void newMaxNumVoices(const int newMaxNumVoices);
    
    bool isSustainPedalDown()   const noexcept { return sustainPedalDown;   }
    bool isSostenutoPedalDown() const noexcept { return sostenutoPedalDown; }
    bool isSoftPedalDown()      const noexcept { return softPedalDown;      }
    float getSoftPedalMultiplier() const noexcept { return softPedalMultiplier; }
    void setSoftPedalGainMultiplier (const float newGain) { softPedalMultiplier = newGain; }
    
    HarmonizerVoice<SampleType>* getVoicePlayingNote (const int midiPitch) const;
    
    // turns off any pitches whose keys are not being held anymore
    void turnOffAllKeyupNotes (const bool allowTailOff, const bool includePedalPitchAndDescant);
    
    
private:
    
    juce::CriticalSection lock;
    
    static constexpr int unpitchedGrainRate = 50;
    
    juce::OwnedArray< HarmonizerVoice<SampleType> > voices;
    
    GrainExtractor<SampleType> grains;
    juce::Array<int> indicesOfGrainOnsets;
    
    void setCurrentInputFreq (const float newInputFreq);
    
    // MIDI
    void processMidi (juce::MidiBuffer& midiMessages);
    void handleMidiEvent(const juce::MidiMessage& m, const int samplePosition);
    void noteOn(const int midiPitch, const float velocity, const bool isKeyboard);
    void noteOff (const int midiNoteNumber, const float velocity, const bool allowTailOff, const bool isKeyboard);
    void handlePitchWheel(const int wheelValue);
    void handleAftertouch(const int midiNoteNumber, const int aftertouchValue);
    void handleChannelPressure(const int channelPressureValue);
    void handleController(const int controllerNumber, const int controllerValue);
    void handleSustainPedal (const int value);
    void handleSostenutoPedal (const int value);
    void handleSoftPedal (const int value);
    void handleModWheel(const int wheelValue);
    void handleBreathController(const int controlValue);
    void handleFootController(const int controlValue);
    void handlePortamentoTime(const int controlValue);
    void handleBalance(const int controlValue);
    void handleLegato(const bool isOn);
    
    // voice allocation
    HarmonizerVoice<SampleType>* findFreeVoice (const int midiNoteNumber, const bool stealIfNoneAvailable);
    HarmonizerVoice<SampleType>* findVoiceToSteal (const int midiNoteNumber);
    
    void startVoice (HarmonizerVoice<SampleType>* voice, const int midiPitch, const float velocity, const bool isKeyboard);
    void stopVoice  (HarmonizerVoice<SampleType>* voice, const float velocity, const bool allowTailOff);
    
    // turns on a list of given pitches at once
    void turnOnList (const juce::Array<int>& toTurnOn, const float velocity, const bool partOfChord);
    
    // turns off a list of given pitches at once. Used for turning off midi latch
    void turnOffList (const juce::Array<int>& toTurnOff, const float velocity, const bool allowTailOff, const bool partOfChord);
    
    // this function is called any time the collection of pitches is changed (ie, with regular keyboard input, on each note on/off, or for chord input, once after each chord is triggered). Used for things like pedal pitch, descant, etc
    void pitchCollectionChanged();
    
    void applyPedalPitch();
    
    void applyDescant();
    
    bool latchIsOn;
    
    bool intervalLatchIsOn;
    juce::Array<int> intervalsLatchedTo;
    
    void updateIntervalsLatchedTo();
    
    void playChordFromIntervalSet (const juce::Array<int>& desiredIntervals);
    
    juce::ADSR::Parameters adsrParams;
    juce::ADSR::Parameters quickReleaseParams;
    juce::ADSR::Parameters quickAttackParams;
    
    float currentInputFreq;
    int currentInputPeriod;
    
    double sampleRate;
    bool shouldStealNotes;
    juce::uint32 lastNoteOnCounter;
    int lowestPannedNote;
    int lastPitchWheelValue;
    
    bool pedalPitchIsOn;
    int lastPedalPitch;
    int pedalPitchUpperThresh;
    int pedalPitchInterval;
    
    bool descantIsOn;
    int lastDescantPitch;
    int descantLowerThresh;
    int descantInterval;
    
    PanningManager  panner;
    VelocityHelper  velocityConverter;
    PitchConverter  pitchConverter;
    PitchBendHelper bendTracker;
    
    bool adsrIsOn;
    
    juce::MidiBuffer aggregateMidiBuffer; // this midi buffer will be used to collect the harmonizer's aggregate MIDI output
    int lastMidiTimeStamp;
    int lastMidiChannel;
    
    bool sustainPedalDown, sostenutoPedalDown, softPedalDown;
    
    float softPedalMultiplier; // the multiplier by which each voice's output will be multiplied when the soft pedal is down
    
    juce::AudioBuffer<SampleType> windowBuffer;
    void fillWindowBuffer (const int numSamples);
    int windowSize;
    
    juce::AudioBuffer<SampleType> unpitchedWindow;
    void initializeUnpitchedWindow();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Harmonizer)
};



