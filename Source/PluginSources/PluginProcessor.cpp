/*
    This file defines Imogen's internal audio processor as a whole, when Imogen is built as a plugin target
    Parent file: PluginProcessor.h
*/


#include "../../Source/PluginSources/PluginProcessor.h"
#include "../../Source/PluginSources/PluginEditor.h"


ImogenAudioProcessor::ImogenAudioProcessor():
    AudioProcessor(makeBusProperties()),
    tree(*this, nullptr, "PARAMETERS", createParameters()),
    wasBypassedLastCallback(true)
{
    dryPan             = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("dryPan"));                         jassert(dryPan);
    dryWet             = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("masterDryWet"));                   jassert(dryWet);
    adsrAttack         = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrAttack"));                     jassert(adsrAttack);
    adsrDecay          = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrDecay"));                      jassert(adsrDecay);
    adsrSustain        = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrSustain"));                    jassert(adsrSustain);
    adsrRelease        = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("adsrRelease"));                    jassert(adsrRelease);
    adsrToggle         = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("adsrOnOff"));                      jassert(adsrToggle);
    quickKillMs        = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("quickKillMs"));                    jassert(quickKillMs);
    quickAttackMs      = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("quickAttackMs"));                  jassert(quickAttackMs);
    stereoWidth        = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("stereoWidth"));                    jassert(stereoWidth);
    lowestPanned       = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("lowestPan"));                      jassert(lowestPanned);
    velocitySens       = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("midiVelocitySensitivity"));        jassert(velocitySens);
    pitchBendUp        = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("PitchBendUpRange"));               jassert(pitchBendUp);
    pitchBendDown      = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("PitchBendDownRange"));             jassert(pitchBendDown);
    pedalPitchIsOn     = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("pedalPitchToggle"));               jassert(pedalPitchIsOn);
    pedalPitchThresh   = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("pedalPitchThresh"));               jassert(pedalPitchThresh);
    pedalPitchInterval = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("pedalPitchInterval"));             jassert(pedalPitchInterval);
    descantIsOn        = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("descantToggle"));                  jassert(descantIsOn);
    descantThresh      = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("descantThresh"));                  jassert(descantThresh);
    descantInterval    = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("descantInterval"));                jassert(descantInterval);
    concertPitchHz     = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("concertPitch"));                   jassert(concertPitchHz);
    voiceStealing      = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("voiceStealing"));                  jassert(voiceStealing);
    latchIsOn          = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("latchIsOn"));                      jassert(latchIsOn);
    intervalLockIsOn   = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("intervalLock"));                   jassert(intervalLockIsOn);
    inputGain          = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("inputGain"));                      jassert(inputGain);
    outputGain         = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("outputGain"));                     jassert(outputGain);
    limiterToggle      = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("limiterIsOn"));                    jassert(limiterToggle);
    limiterThresh      = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("limiterThresh"));                  jassert(limiterThresh);
    limiterRelease     = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("limiterRelease"));                 jassert(limiterRelease);
    dryGain            = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("dryGain"));                        jassert(dryGain);
    wetGain            = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("wetGain"));                        jassert(wetGain);
    softPedalGain      = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("softPedalGain"));                  jassert(softPedalGain);
    minDetectedHz      = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("pitchDetectionMinHz"));            jassert(minDetectedHz);
    maxDetectedHz      = dynamic_cast<juce::AudioParameterInt*>  (tree.getParameter("pitchDetectionMaxHz"));            jassert(maxDetectedHz);
    pitchDetectionConfidenceUpperThresh = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("pitchDetectionConfidenceUpperThresh")); jassert(pitchDetectionConfidenceUpperThresh);
    pitchDetectionConfidenceLowerThresh = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("pitchDetectionConfidenceLowerThresh")); jassert(pitchDetectionConfidenceLowerThresh);
    aftertouchGainToggle = dynamic_cast<juce::AudioParameterBool*> (tree.getParameter("aftertouchGainToggle")); jassert(aftertouchGainToggle);
    playingButReleasedGain = dynamic_cast<juce::AudioParameterFloat*>(tree.getParameter("playingButReleasedGain")); jassert(playingButReleasedGain);
    
    if (isUsingDoublePrecision())
        initialize (doubleEngine);
    else
        initialize (floatEngine);
}

ImogenAudioProcessor::~ImogenAudioProcessor()
{ }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename SampleType>
void ImogenAudioProcessor::initialize (bav::ImogenEngine<SampleType>& activeEngine)
{
    double initSamplerate = getSampleRate();
    
    if (initSamplerate <= 0)
        initSamplerate = 44100.0;
    
    int initBlockSize = getBlockSize();
    
    if (initBlockSize <= 0)
        initBlockSize = 512;
    
    activeEngine.initialize (initSamplerate, initBlockSize, 12);
    
    updateAllParameters (activeEngine);
    
    latencySamples = activeEngine.reportLatency();
    
    setLatencySamples (latencySamples);
}


void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock)
{
    if (isUsingDoublePrecision())
        prepareToPlayWrapped (sampleRate, samplesPerBlock, doubleEngine, floatEngine);
    else
        prepareToPlayWrapped (sampleRate, samplesPerBlock, floatEngine,  doubleEngine);
}


template <typename SampleType1, typename SampleType2>
void ImogenAudioProcessor::prepareToPlayWrapped (const double sampleRate, const int samplesPerBlock,
                                                 bav::ImogenEngine<SampleType1>& activeEngine,
                                                 bav::ImogenEngine<SampleType2>& idleEngine)
{
    if (! activeEngine.hasBeenInitialized())
        activeEngine.initialize (sampleRate, samplesPerBlock, 12);
    else
        activeEngine.prepare (sampleRate, samplesPerBlock);
    
    if (! idleEngine.hasBeenReleased())
        idleEngine.releaseResources();
    
    updateAllParameters (activeEngine);
    
    const int newLatency = activeEngine.reportLatency();
    
    if (latencySamples != newLatency)
    {
        latencySamples = newLatency;
        setLatencySamples (newLatency);
    }
}


void ImogenAudioProcessor::releaseResources()
{
    if (! doubleEngine.hasBeenReleased())
        doubleEngine.releaseResources();
    
    if (! floatEngine.hasBeenReleased())
        floatEngine.releaseResources();
}


void ImogenAudioProcessor::reset()
{
    if (isUsingDoublePrecision())
        doubleEngine.reset();
    else
        floatEngine.reset();
}


void ImogenAudioProcessor::killAllMidi()
{
    if (isUsingDoublePrecision())
        doubleEngine.killAllMidi();
    else
        floatEngine.killAllMidi();
}


/*
 These two functions represent the top-level callbacks made by the host during audio processing. Audio samples may be sent to us as float or double values; both of these functions redirect to the templated processBlockWrapped() function below.
 The buffers sent to this function by the host may be variable in size, so I have coded defensively around several edge cases & possible buggy host behavior and created several layers of checks that each callback passes through before individual chunks of audio are actually rendered.
 In this first layer, we just check that the host has initialzed the processor with the correct processing precision mode...
 */
void ImogenAudioProcessor::processBlock (juce::AudioBuffer<float>&  buffer, juce::MidiBuffer& midiMessages)
{
    if (isUsingDoublePrecision()) // this would be a REALLY stupid host, butttt ¯\_(ツ)_/¯
        return;
    
    processBlockWrapped (buffer, midiMessages, floatEngine);
}


void ImogenAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    if (! isUsingDoublePrecision())
        return;
    
    processBlockWrapped (buffer, midiMessages, doubleEngine);
}

// LAYER 2:
template <typename SampleType>
void ImogenAudioProcessor::processBlockWrapped (juce::AudioBuffer<SampleType>& buffer,
                                                juce::MidiBuffer& midiMessages,
                                                bav::ImogenEngine<SampleType>& engine)
{
    // at this level, we check that our input is not disabled, the processing engine has been initialized, and that the buffer sent to us is not empty.
    // NB at this stage, the buffers may still exceed the default blocksize and/or the value prepared for with the last prepareToPlay() call, and they may also be as short as 1 sample long.
    
    if (! engine.hasBeenInitialized())
        return;
    
    if (! juce::JUCEApplicationBase::isStandaloneApp())
        if ((host.isLogic() || host.isGarageBand()) && (getBusesLayout().getChannelSet(true, 1) == juce::AudioChannelSet::disabled()))
            return;   // our audio input is disabled! can't do processing
    
    updateAllParameters (engine); // the host might use a 0-sample long audio buffer to tell us to update our state with new automation values, which is why the check for that is AFTER this call.
    
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
        return;
    
    juce::AudioBuffer<SampleType> outBus = AudioProcessor::getBusBuffer (buffer, false, 0);
    juce::AudioBuffer<SampleType>& inBus = outBus;
    
    if (juce::JUCEApplicationBase::isStandaloneApp())
        inBus = AudioProcessor::getBusBuffer (buffer, true, 0);
    else
        inBus = AudioProcessor::getBusBuffer (buffer, true, (host.isLogic() || host.isGarageBand()));
    
    engine.process (inBus, outBus, midiMessages, wasBypassedLastCallback, false);
    
    wasBypassedLastCallback = false;
}



/*
 These two functions represent the top-level callbacks made by the host during audio processing when the plugin is bypassed in the signal chain. Audio samples may be sent to us as float or double values; both of these functions redirect to the templated processBlockWrapped() function below.
 The buffers sent to this function by the host may be variable in size, so I have coded defensively around several edge cases & possible buggy host behavior and created several layers of checks that each callback passes through before individual chunks of audio are actually rendered.
 In this first layer, we just check that the host has initialzed the processor with the correct processing precision mode...
 */
void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (isUsingDoublePrecision())
        return;
    
    processBlockBypassedWrapped (buffer, midiMessages, floatEngine);
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    if (! isUsingDoublePrecision())
        return;
    
    processBlockBypassedWrapped (buffer, midiMessages, doubleEngine);
}

// LAYER 2:
template <typename SampleType>
void ImogenAudioProcessor::processBlockBypassedWrapped (juce::AudioBuffer<SampleType>& buffer,
                                                        juce::MidiBuffer& midiMessages,
                                                        bav::ImogenEngine<SampleType>& engine)
{
    // at this level, we check that our input is not disabled, the processing engine has been initialized, and that the buffer sent to us is not empty.
    // NB at this stage, the buffers may still exceed the default blocksize and/or the value prepared for with the last prepareToPlay() call, and they may also be as short as 1 sample long.
    
    if (! engine.hasBeenInitialized())
        return;
    
    if (! juce::JUCEApplicationBase::isStandaloneApp())
        if ((host.isLogic() || host.isGarageBand()) && (getBusesLayout().getChannelSet(true, 1) == juce::AudioChannelSet::disabled()))
            return;   // our audio input is disabled! can't do processing
    
    updateAllParameters (engine);
    
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
        return;
    
    juce::AudioBuffer<SampleType> outBus = AudioProcessor::getBusBuffer (buffer, false, 0);
    juce::AudioBuffer<SampleType>& inBus = outBus;
    
    if (juce::JUCEApplicationBase::isStandaloneApp())
        inBus = AudioProcessor::getBusBuffer (buffer, true, 0);
    else
        inBus = AudioProcessor::getBusBuffer (buffer, true, (host.isLogic() || host.isGarageBand()));
    
    if (! wasBypassedLastCallback)
        engine.process (inBus, outBus, midiMessages, false, true); // render 1 more output frame & ramp gain to 0
    else
        engine.processBypassed (inBus, outBus); // N.B. midi passes through unaffected when plugin is bypassed
    
    wasBypassedLastCallback = true;
}


/*===========================================================================================================================
 ============================================================================================================================*/

bool ImogenAudioProcessor::shouldWarnUserToEnableSidechain() const
{
    if (juce::JUCEApplicationBase::isStandaloneApp())
        return false;

    if (! (host.isLogic() || host.isGarageBand()))
        return false;
    
    return (getBusesLayout().getChannelSet(true, 1) == juce::AudioChannelSet::disabled());
}


// functions for updating parameters ----------------------------------------------------------------------------------------------------------------

template<typename SampleType>
void ImogenAudioProcessor::updateAllParameters (bav::ImogenEngine<SampleType>& activeEngine)
{
    updatePitchDetectionWrapped (activeEngine);
    
    updateGainsPrivate (activeEngine);
    
    activeEngine.updateLimiter     (limiterThresh->get(), limiterRelease->get(), limiterToggle->get());
    activeEngine.updateDryWet      (dryWet->get());
    activeEngine.updateQuickKill   (quickKillMs->get());
    activeEngine.updateQuickAttack (quickAttackMs->get());
    activeEngine.updateNoteStealing(voiceStealing->get());
    activeEngine.updateDryVoxPan   (dryPan->get());
    
    // these parameter functions have the potential to alter the pitch & other properties of currently playing harmonizer voices:
    activeEngine.updateConcertPitch           (concertPitchHz->get());
    activeEngine.updatePitchbendSettings      (pitchBendUp->get(), pitchBendDown->get());
    activeEngine.updateStereoWidth            (stereoWidth->get(), lowestPanned->get());
    activeEngine.updateMidiVelocitySensitivity(velocitySens->get());
    activeEngine.updateAdsr                   (adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
    
    // these parameter functions have the potential to trigger or turn off midi notes / harmonizer voices:
    activeEngine.updateMidiLatch (latchIsOn->get());
    activeEngine.updateIntervalLock(intervalLockIsOn->get());
    activeEngine.updatePedalPitch(pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
    activeEngine.updateDescant   (descantIsOn->get(), descantThresh->get(), descantInterval->get());
    
    // update num voices
    
    activeEngine.updateAftertouchGainOnOff (aftertouchGainToggle->get());
    activeEngine.updatePlayingButReleasedGain(playingButReleasedGain->get());
}


void ImogenAudioProcessor::updateGains()
{
    if (isUsingDoublePrecision())
        updateGainsPrivate (doubleEngine);
    else
        updateGainsPrivate (floatEngine);
}


template <typename SampleType>
void ImogenAudioProcessor::updateGainsPrivate (bav::ImogenEngine<SampleType>& activeEngine)
{
    activeEngine.updateInputGain    (juce::Decibels::decibelsToGain (inputGain->get()));
    activeEngine.updateOutputGain   (juce::Decibels::decibelsToGain (outputGain->get()));
    activeEngine.updateDryGain      (juce::Decibels::decibelsToGain (dryGain->get()));
    activeEngine.updateWetGain      (juce::Decibels::decibelsToGain (wetGain->get()));
    activeEngine.updateSoftPedalGain(juce::Decibels::decibelsToGain (softPedalGain->get()));
}


void ImogenAudioProcessor::updateDryVoxPan()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateDryVoxPan(dryPan->get());
    else
        floatEngine.updateDryVoxPan(dryPan->get());
}

void ImogenAudioProcessor::updateDryWet()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateDryWet(dryWet->get());
    else
        floatEngine.updateDryWet(dryWet->get());
}

void ImogenAudioProcessor::updateAdsr()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateAdsr(adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
    else
        floatEngine .updateAdsr(adsrAttack->get(), adsrDecay->get(), adsrSustain->get(), adsrRelease->get(), adsrToggle->get());
}

void ImogenAudioProcessor::updateQuickKillMs()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateQuickKill(quickKillMs->get());
    else
        floatEngine .updateQuickKill(quickKillMs->get());
}

void ImogenAudioProcessor::updateQuickAttackMs()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateQuickAttack(quickAttackMs->get());
    else
        floatEngine .updateQuickAttack(quickAttackMs->get());
}

void ImogenAudioProcessor::updateStereoWidth()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateStereoWidth(stereoWidth->get(), lowestPanned->get());
    else
        floatEngine .updateStereoWidth(stereoWidth->get(), lowestPanned->get());
}

void ImogenAudioProcessor::updateMidiVelocitySensitivity()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateMidiVelocitySensitivity(velocitySens->get());
    else
        floatEngine .updateMidiVelocitySensitivity(velocitySens->get());
}

void ImogenAudioProcessor::updatePitchbendSettings()
{
    if (isUsingDoublePrecision())
        doubleEngine.updatePitchbendSettings(pitchBendUp->get(), pitchBendDown->get());
    else
        floatEngine .updatePitchbendSettings(pitchBendUp->get(), pitchBendDown->get());
}

void ImogenAudioProcessor::updatePedalPitch()
{
    if (isUsingDoublePrecision())
        doubleEngine.updatePedalPitch(pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
    else
        floatEngine .updatePedalPitch(pedalPitchIsOn->get(), pedalPitchThresh->get(), pedalPitchInterval->get());
}

void ImogenAudioProcessor::updateDescant()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateDescant(descantIsOn->get(), descantThresh->get(), descantInterval->get());
    else
        floatEngine .updateDescant(descantIsOn->get(), descantThresh->get(), descantInterval->get());
}

void ImogenAudioProcessor::updateConcertPitch()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateConcertPitch(concertPitchHz->get());
    else
        floatEngine .updateConcertPitch(concertPitchHz->get());
}

void ImogenAudioProcessor::updateNoteStealing()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateNoteStealing(voiceStealing->get());
    else
        floatEngine .updateNoteStealing(voiceStealing->get());
}

void ImogenAudioProcessor::updateMidiLatch()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateMidiLatch(latchIsOn->get());
    else
        floatEngine .updateMidiLatch(latchIsOn->get());
}

void ImogenAudioProcessor::updateIntervalLock()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateIntervalLock(intervalLockIsOn->get());
    else
        floatEngine.updateIntervalLock(intervalLockIsOn->get());
}

void ImogenAudioProcessor::updateLimiter()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateLimiter(limiterThresh->get(), limiterRelease->get(), limiterToggle->get());
    else
        floatEngine.updateLimiter(limiterThresh->get(), limiterRelease->get(), limiterToggle->get());
}


void ImogenAudioProcessor::updatePitchDetectionSettings()
{
    if (isUsingDoublePrecision())
        updatePitchDetectionWrapped (doubleEngine);
    else
        updatePitchDetectionWrapped (floatEngine);
}


template <typename SampleType>
void ImogenAudioProcessor::updatePitchDetectionWrapped (bav::ImogenEngine<SampleType>& activeEngine)
{
    activeEngine.updatePitchDetectionHzRange (minDetectedHz->get(), maxDetectedHz->get());
    
    activeEngine.updatePitchDetectionConfidenceThresh (pitchDetectionConfidenceUpperThresh->get(),
                                                       pitchDetectionConfidenceLowerThresh->get());
    
    if (latencySamples != activeEngine.reportLatency())
    {
        latencySamples = activeEngine.reportLatency();
        setLatencySamples (latencySamples);
    }
}


void ImogenAudioProcessor::updateAftertouchGainToggle()
{
    if (isUsingDoublePrecision())
        doubleEngine.updateAftertouchGainOnOff(aftertouchGainToggle->get());
    else
        floatEngine.updateAftertouchGainOnOff(aftertouchGainToggle->get());
}


void ImogenAudioProcessor::updatePlayingButRelesedGain()
{
    if (isUsingDoublePrecision())
        doubleEngine.updatePlayingButReleasedGain(playingButReleasedGain->get());
    else
        floatEngine.updatePlayingButReleasedGain(playingButReleasedGain->get());
}


void ImogenAudioProcessor::returnActivePitches (juce::Array<int>& outputArray) const
{
    if (isUsingDoublePrecision())
        doubleEngine.returnActivePitches(outputArray);
    else
        floatEngine.returnActivePitches(outputArray);
}


void ImogenAudioProcessor::updateNumVoices (const int newNumVoices)
{
    if (isUsingDoublePrecision())
        doubleEngine.updateNumVoices(newNumVoices);
    else
        floatEngine.updateNumVoices(newNumVoices);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// functions for custom preset management system ----------------------------------------------------------------------------------------------------

void ImogenAudioProcessor::savePreset (juce::String presetName)
{
    // this function can be used both to save new preset files or to update existing ones
    
    juce::File writingTo = getPresetsFolder().getChildFile(presetName);
    
    auto xml(tree.copyState().createXml());
   // xml->setAttribute("numberOfVoices", harmonizer.getNumVoices());
    xml->writeTo(writingTo);
    updateHostDisplay();
}


void ImogenAudioProcessor::loadPreset (juce::String presetName)
{
    juce::File presetToLoad = getPresetsFolder().getChildFile(presetName);
    
    if (! presetToLoad.existsAsFile())
        return;

    suspendProcessing (true);
    
    auto xmlElement = juce::parseXML(presetToLoad);
    
    if(xmlElement.get() != nullptr && xmlElement->hasTagName (tree.state.getType()))
    {
        tree.replaceState(juce::ValueTree::fromXml (*xmlElement));
        updateNumVoices( xmlElement->getIntAttribute("numberOfVoices", 4) ); // TO DO : send notif to GUI to update numVoices comboBox
        updateHostDisplay();
    }
    
    suspendProcessing (false);
}


void ImogenAudioProcessor::deletePreset (juce::String presetName)
{
    juce::File presetToDelete = getPresetsFolder().getChildFile(presetName);
    
    if(presetToDelete.existsAsFile())
        if(! presetToDelete.moveToTrash())
            presetToDelete.deleteFile();
    updateHostDisplay();
}


juce::File ImogenAudioProcessor::getPresetsFolder() const
{
    juce::File rootFolder;
    
#if JUCE_MAC
    rootFolder = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userApplicationDataDirectory)
                    .getChildFile ("Audio")
                    .getChildFile ("Presets")
                    .getChildFile ("Ben Vining Music Software")
                    .getChildFile ("Imogen");
    
#elif JUCE_LINUX
    rootFolder = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userApplicationDataDirectory)
                    .getChildFile ("Ben Vining Music Software")
                    .getChildFile ("Imogen");

#else    //  Windows
    rootFolder = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDocumentsDirectory)
                    .getChildFile ("Ben Vining Music Software")
                    .getChildFile ("Imogen");
#endif
    
    if (! rootFolder.isDirectory() && ! rootFolder.existsAsFile())
        rootFolder.createDirectory(); // creates the presets folder if it doesn't already exist
    
    return rootFolder;
}


void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto xml(tree.copyState().createXml());
 //   xml->setAttribute("numberOfVoices", harmonizer.getNumVoices());
    copyXmlToBinary (*xml, destData);
}


void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr && xmlState->hasTagName (tree.state.getType()))
    {
        tree.replaceState(juce::ValueTree::fromXml (*xmlState));
        const int newNumOfVoices = xmlState->getIntAttribute("numberOfVoices", 4);
        updateNumVoices(newNumOfVoices); // TO DO : send notif to GUI to update numVoices comboBox
    }
}


// standard and general-purpose functions -----------------------------------------------------------------------------------------------------------

juce::AudioProcessorValueTreeState::ParameterLayout ImogenAudioProcessor::createParameters() const
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    juce::NormalisableRange<float> gainRange (-60.0f, 0.0f, 0.01f);
    
    
    // general
    params.push_back(std::make_unique<juce::AudioParameterInt>	("dryPan", "Dry vox pan", 0, 127, 64));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("masterDryWet", "% wet", 0, 100, 100));
    
    // ADSR
    juce::NormalisableRange<float> msRange (0.001f, 1.0f, 0.001f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrAttack", "ADSR Attack", msRange, 0.035f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrDecay", "ADSR Decay", msRange, 0.06f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrSustain", "ADSR Sustain",
                                                                 juce::NormalisableRange<float> (0.01f, 1.0f, 0.01f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("adsrRelease", "ADSR Release", msRange, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterBool>	("adsrOnOff", "ADSR on/off", true));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("quickKillMs", "Quick kill ms", 1, 250, 15));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("quickAttackMs", "Quick attack ms", 1, 250, 15));
    
    // stereo width
    params.push_back(std::make_unique<juce::AudioParameterInt> 	("stereoWidth", "Stereo Width", 0, 100, 100));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("lowestPan", "Lowest panned midiPitch", 0, 127, 0));
    
    // midi settings
    params.push_back(std::make_unique<juce::AudioParameterInt> 	("midiVelocitySensitivity", "MIDI Velocity Sensitivity", 0, 100, 100));
    params.push_back(std::make_unique<juce::AudioParameterInt> 	("PitchBendUpRange", "Pitch bend range (up)", 0, 12, 2));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("PitchBendDownRange", "Pitch bend range (down)", 0, 12, 2));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("concertPitch", "Concert pitch (Hz)", 392, 494, 440));
    params.push_back(std::make_unique<juce::AudioParameterBool> ("voiceStealing", "Voice stealing", false));
    params.push_back(std::make_unique<juce::AudioParameterBool> ("latchIsOn", "MIDI latch on/off", false));
    params.push_back(std::make_unique<juce::AudioParameterBool> ("intervalLock", "MIDI interval lock", false));
    // pedal pitch
    params.push_back(std::make_unique<juce::AudioParameterBool>	("pedalPitchToggle", "Pedal pitch on/off", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("pedalPitchThresh", "Pedal pitch upper threshold", 0, 127, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("pedalPitchInterval", "Pedal pitch interval", 1, 12, 12));
    // descant
    params.push_back(std::make_unique<juce::AudioParameterBool>	("descantToggle", "Descant on/off", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("descantThresh", "Descant lower threshold", 0, 127, 127));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("descantInterval", "Descant interval", 1, 12, 12));
    
    // input & output gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>("inputGain", "Input gain",   gainRange, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("outputGain", "Output gain", gainRange, -4.0f));
    
    // dry & wet gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>("dryGain", "Dry gain", gainRange, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("wetGain", "Wet gain", gainRange, 0.0f));
    
    // output limiter
    params.push_back(std::make_unique<juce::AudioParameterBool>	("limiterIsOn", "Limiter on/off", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("limiterThresh", "Limiter threshold", gainRange, -2.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>	("limiterRelease", "limiter release (ms)", 1, 250, 10));
    
    // NEED GUI FOR THIS -- soft pedal gain multiplier
    params.push_back(std::make_unique<juce::AudioParameterFloat>("softPedalGain", "Soft pedal gain", gainRange, 0.0f));
    
    // aftertouch gain toggle on/off
    params.push_back(std::make_unique<juce::AudioParameterBool> ("aftertouchGainToggle", "Aftertouch gain on/off", true));
    
    // playing but released gain multiplier
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("playingButReleasedGain", "Released & ringing gain", gainRange, 0.5f));
    
    // NEED GUI -- PITCH DETECTION SETTINGS
    // Note that the minimum possible Hz value will impact the plugin's latency.
    juce::NormalisableRange<float> confidenceRange (0.0f, 1.0f, 0.01f);
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("pitchDetectionMinHz", "Min possible Hz", 40, 600, 80));
    params.push_back(std::make_unique<juce::AudioParameterInt>  ("pitchDetectionMaxHz", "Max possible Hz", 1000, 10000, 2600));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pitchDetectionConfidenceUpperThresh", "Confidence upper thresh",
                                                                confidenceRange, 0.15f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pitchDetectionConfidenceLowerThresh", "Confidence lower thresh",
                                                                 confidenceRange, 0.05f));
    
    // MAKE PARAM FOR INPUT MODE !!
    
    return { params.begin(), params.end() };
}


double ImogenAudioProcessor::getTailLengthSeconds() const
{
    if (adsrToggle->get())
        return static_cast<double> (adsrRelease->get()); // ADSR release time in seconds
    
    return static_cast<double> (quickKillMs->get() * 1000.0f); // "quick kill" time in seconds
}

int ImogenAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs, so this should be at least 1, even if you're not really implementing programs.
}

int ImogenAudioProcessor::getCurrentProgram() {
    return 0;
}

void ImogenAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String ImogenAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void ImogenAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    ignoreUnused (index, newName);
}


juce::AudioProcessor::BusesProperties ImogenAudioProcessor::makeBusProperties() const
{
    if (! juce::JUCEApplicationBase::isStandaloneApp())
        if (host.isLogic() || host.isGarageBand())
            return BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                    .withInput ("Sidechain", juce::AudioChannelSet::mono(),   true)
                                    .withOutput("Output",    juce::AudioChannelSet::stereo(), true);

    return BusesProperties().withInput ("Input",     juce::AudioChannelSet::stereo(), true)
                            .withOutput("Output",    juce::AudioChannelSet::stereo(), true);
}


bool ImogenAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled())
    {
        if (juce::JUCEApplicationBase::isStandaloneApp())
            return false;
        else if ((! (host.isLogic() || host.isGarageBand())))
                return false;
    }
    
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
}


bool ImogenAudioProcessor::canAddBus (bool isInput) const
{
    if (juce::JUCEApplicationBase::isStandaloneApp())
        return false;
    
    if (host.isLogic() || host.isGarageBand())
        return isInput;
    
    return false;
}


juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor()
{
    return new ImogenAudioProcessorEditor(*this);
}


// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImogenAudioProcessor();
}
