
#if !IMOGEN_HEADLESS
#include "GUI/Holders/Plugin_Editor/PluginEditor.h"
#endif

#include "PluginProcessor.h"


ImogenAudioProcessor::ImogenAudioProcessor()
    : AudioProcessor (makeBusProperties())
#if !IMOGEN_HEADLESS
    , abletonLink (120.0) // constructed with the initial BPM
#endif
{
    parameters.addParametersTo (*this);
    meters.addParametersTo (*this);

    mainBypassPntr = parameters.mainBypass.get();

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
    if (!idleEngine.hasBeenReleased()) idleEngine.releaseResources();

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
    if (!doubleEngine.hasBeenReleased()) doubleEngine.releaseResources();

    if (!floatEngine.hasBeenReleased()) floatEngine.releaseResources();
}


/*===========================================================================================================
 ===========================================================================================================*/

void ImogenAudioProcessor::processBlock (juce::AudioBuffer< float >& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, floatEngine, mainBypassPntr->get());
}


void ImogenAudioProcessor::processBlock (juce::AudioBuffer< double >& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, doubleEngine, mainBypassPntr->get());
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer< float >& buffer, juce::MidiBuffer& midiMessages)
{
    if (! mainBypassPntr->get())
    {
        mainBypassPntr->set (true);
        updateHostDisplay();
    }
    
    processBlockWrapped (buffer, midiMessages, floatEngine, true);
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer< double >& buffer, juce::MidiBuffer& midiMessages)
{
    if (! mainBypassPntr->get())
    {
        mainBypassPntr->set (true);
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
    jassert (!engine.hasBeenReleased() && engine.hasBeenInitialized());

    juce::ScopedNoDenormals nodenorms;
    
    parameters.doAllActions();

    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0) return;

    auto inBus  = getBusBuffer (buffer, true, getBusesLayout().getMainInputChannelSet() == juce::AudioChannelSet::disabled());
    auto outBus = getBusBuffer (buffer, false, 0);

    engine.process (inBus, outBus, midiMessages, isBypassedThisCallback);

    updateMeters (engine.getLatestMeterData());
}

/*===========================================================================================================================
 ============================================================================================================================*/

void ImogenAudioProcessor::updateMeters (ImogenMeterData meterData)
{
    bool anyChanged = false;
    
    auto updateMeter = [&anyChanged] (bav::FloatParameter& meter, float newValue)
    {
        if (meter.get() != newValue)
        {
            meter.setValueNotifyingHost (newValue);
            anyChanged = true;
        }
    };
    
    updateMeter (meters.inputLevel, meterData.inputLevel);
    updateMeter (meters.outputLevelL, meterData.outputLevelL);
    updateMeter (meters.outputLevelR, meterData.outputLevelR);
    updateMeter (meters.gateRedux, meterData.noiseGateGainReduction);
    updateMeter (meters.compRedux, meterData.compressorGainReduction);
    updateMeter (meters.deEssRedux, meterData.deEsserGainReduction);
    updateMeter (meters.limRedux, meterData.limiterGainReduction);
    updateMeter (meters.reverbLevel, meterData.reverbLevel);
    updateMeter (meters.delayLevel, meterData.delayLevel);
    
    if (anyChanged)
        updateHostDisplay();
}

/*===========================================================================================================================
 ============================================================================================================================*/

void ImogenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream (destData, false);
    juce::ValueTree tree {"ImogenState"};
    parameters.serialize (tree).writeToStream (stream);
}

void ImogenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto newTree = juce::ValueTree::readFromData (data, static_cast< size_t > (sizeInBytes));
    
    if (! newTree.isValid()) return;
    
    suspendProcessing (true);
    
    parameters.deserialize (newTree);
    parameters.refreshAllDefaults();
    
    suspendProcessing (false);
    
    updateHostDisplay();
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
    return parameters.adsrRelease.get()->get();
}


inline juce::AudioProcessor::BusesProperties ImogenAudioProcessor::makeBusProperties() const
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

/*===========================================================================================================================
 ============================================================================================================================*/

// This function initializes the actions that will be performed each time a parameter is changed

template < typename SampleType >
void ImogenAudioProcessor::initializeParameterFunctionPointers (bav::ImogenEngine< SampleType >& engine)
{
    parameters.adsrAttack.get()->setAction ([&engine] (float value) { engine.updateAdsrAttack (value); });
    parameters.adsrDecay.get()->setAction ([&engine] (float value) { engine.updateAdsrDecay (value); });
    parameters.adsrSustain.get()->setAction ([&engine] (float value) { engine.updateAdsrSustain (value); });
    parameters.adsrRelease.get()->setAction ([&engine] (float value) { engine.updateAdsrRelease (value); });
    parameters.inputGain.get()->setAction ([&engine] (float value) { engine.updateInputGain (value); });
    parameters.outputGain.get()->setAction ([&engine] (float value) { engine.updateOutputGain (value); });
    parameters.noiseGateThresh.get()->setAction ([&engine] (float value) { engine.updateNoiseGateThresh (value); });
    parameters.compAmount.get()->setAction ([&engine] (float value) { engine.updateCompressorAmount (value); });
    parameters.deEsserThresh.get()->setAction ([&engine] (float value) { engine.updateDeEsserThresh (value); });
    parameters.deEsserAmount.get()->setAction ([&engine] (float value) { engine.updateDeEsserAmount (value); });
    parameters.reverbDecay.get()->setAction ([&engine] (float value) { engine.updateReverbDecay (value); });
    parameters.reverbDuck.get()->setAction ([&engine] (float value) { engine.updateReverbDuck (value); });
    parameters.reverbLoCut.get()->setAction ([&engine] (float value) { engine.updateReverbLoCut (value); });
    parameters.reverbHiCut.get()->setAction ([&engine] (float value) { engine.updateReverbHiCut (value); });
    
    parameters.inputMode.get()->setAction ([&engine] (int value) { engine.setModulatorSource (value); });
    parameters.leadPan.get()->setAction ([&engine] (int value) { engine.updateDryVoxPan (value); });
    parameters.stereoWidth.get()->setAction ([&engine] (int value) { engine.updateStereoWidth (value); });
    parameters.lowestPanned.get()->setAction ([&engine] (int value) { engine.updateLowestPannedNote (value); });
    parameters.velocitySens.get()->setAction ([&engine] (int value) { engine.updateMidiVelocitySensitivity (value); });
    parameters.pitchbendRange.get()->setAction ([&engine] (int value) { engine.updatePitchbendRange (value); });
    parameters.pedalThresh.get()->setAction ([&engine] (int value) { engine.updatePedalThresh (value); });
    parameters.pedalInterval.get()->setAction ([&engine] (int value) { engine.updatePedalInterval (value); });
    parameters.descantThresh.get()->setAction ([&engine] (int value) { engine.updateDescantThresh (value); });
    parameters.descantInterval.get()->setAction ([&engine] (int value) { engine.updateDescantInterval (value); });
    parameters.reverbDryWet.get()->setAction ([&engine] (int value) { engine.updateReverbDryWet (value); });
    parameters.delayDryWet.get()->setAction ([&engine] (int value) { engine.updateDelayDryWet (value); });
    parameters.editorPitchbend.get()->setAction ([&engine] (int value) { engine.recieveExternalPitchbend (value); });
    
    parameters.midiLatch.get()->setAction([&engine] (bool value) { engine.updateMidiLatch (value); });
    parameters.leadBypass.get()->setAction ([&engine] (bool value) { engine.updateLeadBypass (value); });
    parameters.harmonyBypass.get()->setAction([&engine] (bool value) { engine.updateHarmonyBypass (value); });
    parameters.pedalToggle.get()->setAction ([&engine] (bool value) { engine.updatePedalToggle (value); });
    parameters.descantToggle.get()->setAction ([&engine] (bool value) { engine.updateDescantToggle (value); });
    parameters.voiceStealing.get()->setAction ([&engine] (bool value) { engine.updateNoteStealing (value); });
    parameters.limiterToggle.get()->setAction ([&engine] (bool value) { engine.updateLimiter (value); });
    parameters.noiseGateToggle.get()->setAction ([&engine] (bool value) { engine.updateNoiseGateToggle (value); });
    parameters.compToggle.get()->setAction ([&engine] (bool value) { engine.updateCompressorToggle (value); });
    parameters.aftertouchToggle.get()->setAction ([&engine] (bool value) { engine.updateAftertouchGainOnOff (value); });
    parameters.deEsserToggle.get()->setAction ([&engine] (bool value) { engine.updateDeEsserToggle (value); });
    parameters.reverbToggle.get()->setAction ([&engine] (bool value) { engine.updateReverbToggle (value); });
    parameters.delayToggle.get()->setAction ([&engine] (bool value) { engine.updateDelayToggle (value); });
}

/*===========================================================================================================================
 ============================================================================================================================*/

bool ImogenAudioProcessor::hasEditor() const
{
#if IMOGEN_HEADLESS
    return false;
#else
    return true;
#endif
}

juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor()
{
#if IMOGEN_HEADLESS
    return nullptr;
#else
    return new ImogenAudioProcessorEditor (*this);
#endif
}

void ImogenAudioProcessor::saveEditorSize (int width, int height)
{
    parameters.editorSizeX.get()->set (width);
    parameters.editorSizeY.get()->set (height);
}

juce::Point< int > ImogenAudioProcessor::getSavedEditorSize() const
{
    return { parameters.editorSizeX.get()->get(),
             parameters.editorSizeY.get()->get() };
}


/*===========================================================================================================================
 ============================================================================================================================*/

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImogenAudioProcessor();
}
