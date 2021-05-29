
#if ! IMOGEN_HEADLESS
#    include "GUI/Holders/Plugin_Editor/PluginEditor.h"
#endif

#include "PluginProcessor.h"


ImogenAudioProcessor::ImogenAudioProcessor()
{
    state.addTo (*this);

    if (isUsingDoublePrecision())
        initialize (doubleEngine);
    else
        initialize (floatEngine);

    parameters.resetAllToDefault();
}


ImogenAudioProcessor::~ImogenAudioProcessor()
{
}


/*===========================================================================================================
 ===========================================================================================================*/


template < typename SampleType >
inline void ImogenAudioProcessor::initialize (bav::ImogenEngine< SampleType >& activeEngine)
{
    auto initSamplerate = getSampleRate();
    if (initSamplerate <= 0.0) initSamplerate = 44100.0;

    auto initBlockSize = getBlockSize();
    if (initBlockSize <= 0) initBlockSize = 512;

    activeEngine.initialize (initSamplerate, initBlockSize);

    setLatencySamples (activeEngine.reportLatency());

    updateHostDisplay();

    prepareToPlay (initSamplerate, 512);
}


void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int)
{
    if (isUsingDoublePrecision())
        prepareToPlayWrapped (sampleRate, doubleEngine, floatEngine);
    else
        prepareToPlayWrapped (sampleRate, floatEngine, doubleEngine);
}


template < typename SampleType1, typename SampleType2 >
inline void ImogenAudioProcessor::prepareToPlayWrapped (const double                      sampleRate,
                                                        bav::ImogenEngine< SampleType1 >& activeEngine,
                                                        bav::ImogenEngine< SampleType2 >& idleEngine)
{
    if (! idleEngine.hasBeenReleased()) idleEngine.releaseResources();

    initializeParameterFunctionPointers (activeEngine);

    jassert (activeEngine.getLatency() > 0);

    activeEngine.prepare (sampleRate);

    parameters.doAllActions();

    setLatencySamples (activeEngine.reportLatency());

    updateHostDisplay();
}

/*===========================================================================================================
 ===========================================================================================================*/

void ImogenAudioProcessor::releaseResources()
{
    if (! doubleEngine.hasBeenReleased()) doubleEngine.releaseResources();

    if (! floatEngine.hasBeenReleased()) floatEngine.releaseResources();
}


/*===========================================================================================================
 ===========================================================================================================*/

void ImogenAudioProcessor::processBlock (juce::AudioBuffer< float >& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, floatEngine, parameters.mainBypass->get());
}


void ImogenAudioProcessor::processBlock (juce::AudioBuffer< double >& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, doubleEngine, parameters.mainBypass->get());
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer< float >& buffer, juce::MidiBuffer& midiMessages)
{
    if (! parameters.mainBypass->get())
    {
        parameters.mainBypass->set (true);
        updateHostDisplay();
    }

    processBlockWrapped (buffer, midiMessages, floatEngine, true);
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer< double >& buffer, juce::MidiBuffer& midiMessages)
{
    if (! parameters.mainBypass->get())
    {
        parameters.mainBypass->set (true);
        updateHostDisplay();
    }

    processBlockWrapped (buffer, midiMessages, doubleEngine, true);
}


template < typename SampleType >
inline void ImogenAudioProcessor::processBlockWrapped (juce::AudioBuffer< SampleType >& buffer,
                                                       juce::MidiBuffer&                midiMessages,
                                                       bav::ImogenEngine< SampleType >& engine,
                                                       const bool                       isBypassedThisCallback)
{
    jassert (! engine.hasBeenReleased() && engine.hasBeenInitialized());

    juce::ScopedNoDenormals nodenorms;

    parameters.doAllActions();

    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0) return;

    auto inBus  = getBusBuffer (buffer, true, getBusesLayout().getMainInputChannelSet() == juce::AudioChannelSet::disabled());
    auto outBus = getBusBuffer (buffer, false, 0);

    engine.process (inBus, outBus, midiMessages, isBypassedThisCallback);

    updateMeters (engine.getLatestMeterData());
    updateInternals (engine.getLatestInternalsData());
}

/*===========================================================================================================================
 ============================================================================================================================*/

void ImogenAudioProcessor::updateMeters (ImogenMeterData meterData)
{
    meters.inputLevel->set (meterData.inputLevel);
    meters.outputLevelL->set (meterData.outputLevelL);
    meters.outputLevelR->set (meterData.outputLevelR);
    meters.gateRedux->set (meterData.noiseGateGainReduction);
    meters.compRedux->set (meterData.compressorGainReduction);
    meters.deEssRedux->set (meterData.deEsserGainReduction);
    meters.limRedux->set (meterData.limiterGainReduction);
    meters.reverbLevel->set (meterData.reverbLevel);
    meters.delayLevel->set (meterData.delayLevel);
}


void ImogenAudioProcessor::updateInternals (ImogenInternalsData internalsData)
{
    internals.abletonLinkEnabled->set (abletonLink.isEnabled());
    internals.abletonLinkSessionPeers->set (static_cast<int> (abletonLink.numPeers()));
    internals.mtsEspIsConnected->set (internalsData.mtsEspConnected);
    internals.currentCentsSharp->set (internalsData.currentCentsSharp);
    internals.currentInputNote->set (internalsData.currentPitch);
    internals.lastMovedMidiController->set (internalsData.lastMovedMidiController);
    internals.lastMovedCCValue->set (internalsData.lastMovedControllerValue);
}

/*===========================================================================================================================
 ============================================================================================================================*/

void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    bav::toBinary (state, destData);
}

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    bav::fromBinary (data, sizeInBytes, state);
}

/*===========================================================================================================================
 ============================================================================================================================*/

juce::String ImogenAudioProcessor::getScaleName() const
{
    return isUsingDoublePrecision() ? doubleEngine.getScaleName() : floatEngine.getScaleName();
}

/*===========================================================================================================================
 ============================================================================================================================*/

double ImogenAudioProcessor::getTailLengthSeconds() const
{
    return parameters.adsrRelease->get();
}

juce::AudioProcessor::BusesProperties ImogenAudioProcessor::createBusProperties() const
{
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto mono   = juce::AudioChannelSet::mono();

    return BusesProperties()
        .withInput (TRANS ("Input"), stereo, true)
        .withInput (TRANS ("Sidechain"), mono, false)
        .withOutput (TRANS ("Output"), stereo, true);
}


bool ImogenAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto disabled = juce::AudioChannelSet::disabled();

    if (layouts.getMainInputChannelSet() == disabled && layouts.getChannelSet (true, 1) == disabled) return false;

    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

juce::AudioProcessorParameter* ImogenAudioProcessor::getBypassParameter() const
{
    return parameters.mainBypass.get();
}

/*===========================================================================================================================
 ============================================================================================================================*/

// This function initializes the actions that will be performed each time a parameter is changed

template < typename SampleType >
void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine< SampleType >& engine)
{
    parameters.adsrAttack->setAction ([&engine] (float value)
                                      { engine.updateAdsrAttack (value); });
    parameters.adsrDecay->setAction ([&engine] (float value)
                                     { engine.updateAdsrDecay (value); });
    parameters.adsrSustain->setAction ([&engine] (int value)
                                       { engine.updateAdsrSustain (value); });
    parameters.adsrRelease->setAction ([&engine] (float value)
                                       { engine.updateAdsrRelease (value); });
    parameters.inputGain->setAction ([&engine] (float value)
                                     { engine.updateInputGain (value); });
    parameters.outputGain->setAction ([&engine] (float value)
                                      { engine.updateOutputGain (value); });
    parameters.noiseGateThresh->setAction ([&engine] (float value)
                                           { engine.updateNoiseGateThresh (value); });
    parameters.compAmount->setAction ([&engine] (int value)
                                      { engine.updateCompressorAmount (value); });
    parameters.deEsserThresh->setAction ([&engine] (float value)
                                         { engine.updateDeEsserThresh (value); });
    parameters.deEsserAmount->setAction ([&engine] (int value)
                                         { engine.updateDeEsserAmount (value); });
    parameters.reverbDecay->setAction ([&engine] (int value)
                                       { engine.updateReverbDecay (value); });
    parameters.reverbDuck->setAction ([&engine] (int value)
                                      { engine.updateReverbDuck (value); });
    parameters.reverbLoCut->setAction ([&engine] (float value)
                                       { engine.updateReverbLoCut (value); });
    parameters.reverbHiCut->setAction ([&engine] (float value)
                                       { engine.updateReverbHiCut (value); });

    parameters.inputMode->setAction ([&engine] (int value)
                                     { engine.setModulatorSource (value); });
    parameters.leadPan->setAction ([&engine] (int value)
                                   { engine.updateDryVoxPan (value); });
    parameters.stereoWidth->setAction ([&engine] (int value)
                                       { engine.updateStereoWidth (value); });
    parameters.lowestPanned->setAction ([&engine] (int value)
                                        { engine.updateLowestPannedNote (value); });
    parameters.velocitySens->setAction ([&engine] (int value)
                                        { engine.updateMidiVelocitySensitivity (value); });
    parameters.pitchbendRange->setAction ([&engine] (int value)
                                          { engine.updatePitchbendRange (value); });
    parameters.pedalThresh->setAction ([&engine] (int value)
                                       { engine.updatePedalThresh (value); });
    parameters.pedalInterval->setAction ([&engine] (int value)
                                         { engine.updatePedalInterval (value); });
    parameters.descantThresh->setAction ([&engine] (int value)
                                         { engine.updateDescantThresh (value); });
    parameters.descantInterval->setAction ([&engine] (int value)
                                           { engine.updateDescantInterval (value); });
    parameters.reverbDryWet->setAction ([&engine] (int value)
                                        { engine.updateReverbDryWet (value); });
    parameters.delayDryWet->setAction ([&engine] (int value)
                                       { engine.updateDelayDryWet (value); });
    parameters.editorPitchbend->setAction ([&engine] (int value)
                                           { engine.recieveExternalPitchbend (value); });

    parameters.midiLatch->setAction ([&engine] (bool value)
                                     { engine.updateMidiLatch (value); });
    parameters.leadBypass->setAction ([&engine] (bool value)
                                      { engine.updateLeadBypass (value); });
    parameters.harmonyBypass->setAction ([&engine] (bool value)
                                         { engine.updateHarmonyBypass (value); });
    parameters.pedalToggle->setAction ([&engine] (bool value)
                                       { engine.updatePedalToggle (value); });
    parameters.descantToggle->setAction ([&engine] (bool value)
                                         { engine.updateDescantToggle (value); });
    parameters.voiceStealing->setAction ([&engine] (bool value)
                                         { engine.updateNoteStealing (value); });
    parameters.limiterToggle->setAction ([&engine] (bool value)
                                         { engine.updateLimiter (value); });
    parameters.noiseGateToggle->setAction ([&engine] (bool value)
                                           { engine.updateNoiseGateToggle (value); });
    parameters.compToggle->setAction ([&engine] (bool value)
                                      { engine.updateCompressorToggle (value); });
    parameters.aftertouchToggle->setAction ([&engine] (bool value)
                                            { engine.updateAftertouchGainOnOff (value); });
    parameters.deEsserToggle->setAction ([&engine] (bool value)
                                         { engine.updateDeEsserToggle (value); });
    parameters.reverbToggle->setAction ([&engine] (bool value)
                                        { engine.updateReverbToggle (value); });
    parameters.delayToggle->setAction ([&engine] (bool value)
                                       { engine.updateDelayToggle (value); });
}

/*===========================================================================================================================
 ============================================================================================================================*/

bool ImogenAudioProcessor::hasEditor() const
{
#if IMOGEN_HEADLESS
    return false;
#endif
    return true;
}

juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor()
{
#if IMOGEN_HEADLESS
    return nullptr;
#else
    return new ImogenAudioProcessorEditor (*this);
#endif
}


/*===========================================================================================================================
 ============================================================================================================================*/

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImogenAudioProcessor();
}
