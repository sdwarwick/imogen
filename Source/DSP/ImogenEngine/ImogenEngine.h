
/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 ImogenEngine
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenEngine
 description:        base class that wraps the Harmonizer class into a self-sufficient audio engine
 dependencies:       bv_dsp
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include "bv_dsp/bv_dsp.h"

#include "Harmonizer/HarmonizerVoice/HarmonizerVoice.h"
#include "Harmonizer/Harmonizer.h"


struct ImogenMeterData
{
    float inputLevel {};
    float outputLevelL {};
    float outputLevelR {};
    float noiseGateGainReduction {};
    float compressorGainReduction {};
    float deEsserGainReduction {};
    float limiterGainReduction {};
    float reverbLevel {};
    float delayLevel {};
};


struct ImogenInternalsData
{
    bool mtsEspConnected {};
    juce::String mtsEspScaleName {};
    int lastMovedMidiController {};
    int lastMovedControllerValue {};
    int currentCentsSharp {};
    int currentPitch {};
};


namespace bav
{
template < typename SampleType >
class ImogenEngine : public bav::dsp::FIFOWrappedEngine< SampleType >
{
    using FIFOEngine  = bav::dsp::FIFOWrappedEngine< SampleType >;
    using AudioBuffer = juce::AudioBuffer< SampleType >;
    using MidiBuffer  = juce::MidiBuffer;
    using uint32      = juce::uint32;


public:
    ImogenEngine();

    void killAllMidi();

    int reportLatency() const noexcept { return FIFOEngine::getLatency(); }

    int getCurrentNumVoices() const { return harmonizer.getNumVoices(); }

    void returnActivePitches (juce::Array< int >& outputArray) const;

    void recieveExternalPitchbend (const int bend);

    ImogenMeterData getLatestMeterData() const { return meterData; }
    ImogenInternalsData getLatestInternalsData() const { return internalsData; }

    /*=========================================================================================*/
    /* Parameter updating functions */

    void updateLeadBypass (bool leadIsBypassed) { leadBypass.store (leadIsBypassed); }
    void updateHarmonyBypass (bool harmoniesAreBypassed) { harmonyBypass.store (harmoniesAreBypassed); }
    void updateDryWet (float percentWet) { dryWetMixer.setWetMixProportion (percentWet); }
    void updateLowestPannedNote (int note) { harmonizer.updateLowestPannedNote (note); }
    void updateMidiVelocitySensitivity (const int newSensitivity) { harmonizer.updateMidiVelocitySensitivity (newSensitivity); }
    void updatePitchbendRange (const int rangeST) { harmonizer.updatePitchbendSettings (rangeST, rangeST); }
    void updatePedalToggle (bool isOn) { harmonizer.setPedalPitch (isOn); }
    void updatePedalThresh (int note) { harmonizer.setPedalPitchUpperThresh (note); }
    void updatePedalInterval (int st) { harmonizer.setPedalPitchInterval (st); }
    void updateDescantToggle (bool isOn) { harmonizer.setDescant (isOn); }
    void updateDescantThresh (int note) { harmonizer.setDescantLowerThresh (note); }
    void updateDescantInterval (int st) { harmonizer.setDescantInterval (st); }
    void updateNoteStealing (const bool shouldSteal) { harmonizer.setNoteStealingEnabled (shouldSteal); }
    void updateInputGain (const float newInGain) { inputGain.setGain (newInGain); }
    void updateOutputGain (const float newOutGain) { outputGain.setGain (newOutGain); }
    void updateLimiter (const bool isOn) { limiterIsOn.store (isOn); }
    void updateNoiseGateToggle (bool isOn) { noiseGateIsOn.store (isOn); }
    void updateNoiseGateThresh (float threshDB) { gate.setThreshold (threshDB); }
    void updateCompressorToggle (bool isOn) { compressorIsOn.store (isOn); }
    void updateAftertouchGainOnOff (const bool shouldBeOn) { harmonizer.setAftertouchGainOnOff (shouldBeOn); }
    void updateDeEsserToggle (bool isOn) { deEsserIsOn.store (isOn); }
    void updateDeEsserThresh (float threshDB) { deEsser.setThresh (threshDB); }
    void updateDeEsserAmount (int amount) { deEsser.setDeEssAmount (float(amount) * 0.01f); }
    void updateReverbToggle (bool isOn) { reverbIsOn.store (isOn); }
    void updateReverbDryWet (int wetPcnt) { reverb.setDryWet (wetPcnt); }
    void updateReverbDuck (int duck) { reverb.setDuckAmount (float(duck) * 0.01f); }
    void updateReverbLoCut (float loCutFreq) { reverb.setLoCutFrequency (loCutFreq); }
    void updateReverbHiCut (float hiCutFreq) { reverb.setHiCutFrequency (hiCutFreq); }
    void updateDelayToggle (bool isOn) { delayIsOn.store (isOn); }
    void updateDelayDryWet (int pcntWet) { delay.setDryWet (pcntWet); }

    void updateDryVoxPan (int newMidiPan) { dryPanner.setMidiPan (newMidiPan); }

    void updateAdsrAttack (float attack)
    {
        adsrSettings.attack = attack;
        harmonizer.updateADSRsettings (attack, adsrSettings.decay, adsrSettings.sustain, adsrSettings.release);
    }

    void updateAdsrDecay (float decay)
    {
        adsrSettings.decay = decay;
        harmonizer.updateADSRsettings (adsrSettings.attack, decay, adsrSettings.sustain, adsrSettings.release);
    }

    void updateAdsrSustain (int sustain)
    {
        adsrSettings.sustain = float(sustain) * 0.01f;
        harmonizer.updateADSRsettings (adsrSettings.attack, adsrSettings.decay, adsrSettings.sustain, adsrSettings.release);
    }

    void updateAdsrRelease (float release)
    {
        adsrSettings.release = release;
        harmonizer.updateADSRsettings (adsrSettings.attack, adsrSettings.decay, adsrSettings.sustain, release);
    }

    void updateStereoWidth (int width)
    {
        harmonizer.updateStereoWidth (width);
        reverb.setWidth (static_cast< float > (width) * 0.01f);
    }

    void updateCompressorAmount (int amount)
    {
        const auto a = float(amount) * 0.01f;
        compressor.setThreshold (juce::jmap (a, 0.0f, -60.0f));
        compressor.setRatio (juce::jmap (a, 1.0f, 10.0f));
    }

    void updateReverbDecay (int decay)
    {
        const auto d = float(decay) * 0.01f;
        reverb.setDamping (1.0f - d);
        reverb.setRoomSize (d);
    }


    /*=========================================================================================*/

    int getModulatorSource() const
    {
        using Mode = typename bav::dsp::FX::MonoStereoConverter< SampleType >::StereoReductionMode;

        switch (stereoReducer.getStereoReductionMode())
        {
            case (Mode::leftOnly) : return 0;
            case (Mode::rightOnly) : return 1;
            case (Mode::mixToMono) : return 2;
        }
    }

    void setModulatorSource (const int newSource)
    {
        using Mode = typename bav::dsp::FX::MonoStereoConverter< SampleType >::StereoReductionMode;

        switch (newSource)
        {
            case (1) : stereoReducer.setStereoReductionMode (Mode::rightOnly);
            case (2) : stereoReducer.setStereoReductionMode (Mode::mixToMono);
            default : stereoReducer.setStereoReductionMode (Mode::leftOnly);
        }
    }

    /*=========================================================================================*/

    void playChord (const juce::Array< int >& desiredNotes, const float velocity, const bool allowTailOffOfOld);

    bool isMidiLatched() const { return harmonizer.isLatched(); }
    void updateMidiLatch (const bool isLatched) { harmonizer.setMidiLatch (isLatched, true); }

    bool         isConnectedToMtsEsp() const noexcept { return harmonizer.isConnectedToMtsEsp(); }
    juce::String getScaleName() const { return harmonizer.getScaleName(); }

    /*=========================================================================================*/

private:
    void renderBlock (const AudioBuffer& input, AudioBuffer& output, MidiBuffer& midiMessages) override;

    void bypassedBlock (const AudioBuffer& input, MidiBuffer& midiMessages) override;

    void initialized (int newInternalBlocksize, double samplerate) override;

    void prepareToPlay (double samplerate) override;

    void latencyChanged (int newInternalBlocksize) override;

    void release() override;

    void resetSmoothedValues (int blocksize);

    /*=========================================================================================*/

    Harmonizer< SampleType > harmonizer;

    AudioBuffer monoBuffer;  // this buffer is used to store the mono input signal so that input gain can be applied
    AudioBuffer wetBuffer;   // this buffer is where the 12 harmony voices' output gets added together
    AudioBuffer dryBuffer;   // this buffer is used for panning & delaying the dry signal

    juce::dsp::ProcessSpec dspSpec;

    bav::dsp::FX::MonoStereoConverter< SampleType > stereoReducer;  // the harmonizer only accepts mono input

    bav::dsp::FX::SmoothedGain< SampleType > inputGain, outputGain;

    bav::dsp::FX::NoiseGate< SampleType > gate;
    std::atomic< bool >                   noiseGateIsOn;

    bav::dsp::FX::DeEsser< SampleType > deEsser;
    std::atomic< bool >                 deEsserIsOn;

    juce::dsp::DryWetMixer< SampleType > dryWetMixer;

    juce::dsp::IIR::Filter< SampleType > initialHiddenLoCut;

    bav::dsp::FX::Compressor< SampleType > compressor;
    std::atomic< bool >                    compressorIsOn;

    bav::dsp::FX::Reverb reverb;
    std::atomic< bool >  reverbIsOn;

    bav::dsp::FX::Limiter< SampleType > limiter;
    std::atomic< bool >                 limiterIsOn;
    std::atomic< float >                limiterThresh, limiterRelease;

    bav::dsp::FX::MonoToStereoPanner< SampleType > dryPanner;

    bav::dsp::FX::Delay< SampleType > delay;
    std::atomic< bool >               delayIsOn;

    std::atomic< bool > leadBypass, harmonyBypass;

    struct ADSRsettings
    {
        float attack, decay, sustain, release;
    };

    ADSRsettings adsrSettings;


    ImogenMeterData meterData;
    
    ImogenInternalsData internalsData;

    inline void resetMeterData()
    {
        meterData.inputLevel              = 0.0f;
        meterData.noiseGateGainReduction  = 0.0f;
        meterData.deEsserGainReduction    = 0.0f;
        meterData.compressorGainReduction = 0.0f;
        meterData.delayLevel              = 0.0f;
        meterData.reverbLevel             = 0.0f;
        meterData.limiterGainReduction    = 0.0f;
        meterData.outputLevelL            = 0.0f;
        meterData.outputLevelR            = 0.0f;
    }
    
    void udpateInternalsData();

    static constexpr auto limiterThreshDb     = 0.0f;
    static constexpr auto limiterReleaseMs    = 35.0f;
    static constexpr auto noiseGateAttackMs   = 25.0f;
    static constexpr auto noiseGateReleaseMs  = 100.0f;
    static constexpr auto noiseGateFloorRatio = 10.0f;  // ratio to one when the noise gate is activated
    static constexpr auto compressorAttackMs  = 4.0f;
    static constexpr auto compressorReleaseMs = 200.0f;

    static constexpr auto initialHiddenHiPassFreq = SampleType (65);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenEngine)
};


}  // namespace bav
