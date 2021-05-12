
/*======================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 PluginProcessor.cpp: This file contains the guts of Imogen's AudioProcessor code.
 
======================================================================================================================================================*/


#include "GUI/Holders/Plugin_Editor/PluginEditor.h"
#include "PluginProcessor.h"

#include "PluginProcessorParameters.cpp"
#include "PluginProcessorState.cpp"


ImogenAudioProcessor::ImogenAudioProcessor()
  : AudioProcessor (makeBusProperties()),
    state (ValueTreeIDs::Imogen),
    treeSync (state, *this),
    nonAutomatableProperties (Imogen::createNonAutomatableParametersTree())
#if IMOGEN_USE_ABLETON_LINK
    , abletonLink (120.0) // constructed with the initial BPM
#endif
{
#if BV_USE_NE10
    ne10_init();
#endif
    
    bav::initializeTranslations (findAppropriateTranslationFile());
    
    addParameterGroup (Imogen::createParameterTree());
    
    Imogen::buildImogenMainValueTree (state, getParameterTree(), nonAutomatableProperties);
    
    Imogen::initializeParameterPointers (parameterPointers, meterParameterPointers, getParameterTree());
    
    auto* tempMB = getParameterPntr (mainBypassID);
    jassert (tempMB != nullptr);
    mainBypassPntr = tempMB->orig();
    
    bav::createTwoWayParameterValueTreeAttachments (parameterTreeAttachments,
                                                    state.getChildWithName (ValueTreeIDs::Parameters), numParams,
                                                    [this](int param) { return getParameterPntr (static_cast<ParameterID>(param)); });
    
    bav::createWriteOnlyParameterValueTreeAttachments (meterTreeAttachments,
                                                       state.getChildWithName (ValueTreeIDs::Meters), numMeters,
                                                       [this](int param) { return getMeterParamPntr (static_cast<MeterID>(param)); });
    
    if (isUsingDoublePrecision())
        initialize (doubleEngine);
    else
        initialize (floatEngine);
    
    treeSync.sendFullSyncCallback();
    resetParameterDefaultsToCurrentValues();
}


ImogenAudioProcessor::~ImogenAudioProcessor()
{

}


/*===========================================================================================================
 ===========================================================================================================*/


template <typename SampleType>
inline void ImogenAudioProcessor::initialize (bav::ImogenEngine<SampleType>& activeEngine)
{
    auto initSamplerate = getSampleRate();
    if (initSamplerate <= 0.0) initSamplerate = 44100.0;
    
    auto initBlockSize = getBlockSize();
    if (initBlockSize <= 0) initBlockSize = 512;
    
    activeEngine.initialize (initSamplerate, initBlockSize);
    
    setLatencySamples (activeEngine.reportLatency());
    ChangeDetails change;
    updateHostDisplay (change.withLatencyChanged (true));
    
    initializeParameterFunctionPointers (activeEngine);
}


void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock)
{
    if (isUsingDoublePrecision())
        prepareToPlayWrapped (sampleRate, doubleEngine, floatEngine);
    else
        prepareToPlayWrapped (sampleRate, floatEngine,  doubleEngine);
}


template <typename SampleType1, typename SampleType2>
inline void ImogenAudioProcessor::prepareToPlayWrapped (const double sampleRate,
                                                        bav::ImogenEngine<SampleType1>& activeEngine,
                                                        bav::ImogenEngine<SampleType2>& idleEngine)
{
    if (! idleEngine.hasBeenReleased())
        idleEngine.releaseResources();
    
    initializeParameterFunctionPointers (activeEngine);
    
    jassert (activeEngine.getLatency() > 0);
    
    activeEngine.prepare (sampleRate);
    
    setLatencySamples (activeEngine.reportLatency());
    ChangeDetails change;
    updateHostDisplay (change.withLatencyChanged (true));
}

/*===========================================================================================================
 ===========================================================================================================*/

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

/*===========================================================================================================
 ===========================================================================================================*/

void ImogenAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, floatEngine,
                         mainBypassPntr->getValue() >= 0.5f);
}


void ImogenAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, doubleEngine,
                         mainBypassPntr->getValue() >= 0.5f);
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (! (mainBypassPntr->getValue() >= 0.5f))
    {
        mainBypassPntr->setValueNotifyingHost (1.0f);

        ChangeDetails change;
        updateHostDisplay (change.withParameterInfoChanged (true));
    }
    
    processBlockWrapped (buffer, midiMessages, floatEngine, true);
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    if (! (mainBypassPntr->getValue() >= 0.5f))
    {
        mainBypassPntr->setValueNotifyingHost (1.0f);
        
        ChangeDetails change;
        updateHostDisplay (change.withParameterInfoChanged (true));
    }
    
    processBlockWrapped (buffer, midiMessages, doubleEngine, true);
}


template <typename SampleType>
inline void ImogenAudioProcessor::processBlockWrapped (juce::AudioBuffer<SampleType>& buffer,
                                                       juce::MidiBuffer& midiMessages,
                                                       bav::ImogenEngine<SampleType>& engine,
                                                       const bool isBypassedThisCallback)
{
    jassert (! engine.hasBeenReleased() && engine.hasBeenInitialized());
    
    juce::ScopedNoDenormals nodenorms;
    
    updateAllParameters();
    
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
        return;
    
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
    
    auto getMeterValue = [&meterData](MeterID meter)
                         {
                             switch (meter)
                             {
                                 case (inputLevelID):        return meterData.inputLevel;
                                 case (outputLevelLID):      return meterData.outputLevelL;
                                 case (outputLevelRID):      return meterData.outputLevelR;
                                 case (gateReduxID):         return meterData.noiseGateGainReduction;
                                 case (compReduxID):         return meterData.compressorGainReduction;
                                 case (deEssGainReduxID):    return meterData.deEsserGainReduction;
                                 case (limiterGainReduxID):  return meterData.limiterGainReduction;
                                 case (reverbLevelID):       return meterData.reverbLevel;
                                 case (delayLevelID):        return meterData.delayLevel;
                             }
                         };
    
    auto updateMeter = [&anyChanged](RAP* meter, float newValue)
                       {
                           if (meter->getValue() != newValue)
                           {
                               meter->setValueNotifyingHost (newValue);
                               anyChanged = true;
                           }
                       };
    
    for (auto* param : meterParameterPointers)
    {
        updateMeter (param->orig(),
                     getMeterValue (static_cast<MeterID> (param->key())));
    }
    
    if (anyChanged)
    {
        ChangeDetails change;
        updateHostDisplay (change.withParameterInfoChanged (true));
    }
}

/*===========================================================================================================================
 ============================================================================================================================*/

void ImogenAudioProcessor::changeMidiLatchState (bool isNowLatched)
{
    if (isUsingDoublePrecision())
        doubleEngine.updateMidiLatch (isNowLatched);
    else
        floatEngine.updateMidiLatch (isNowLatched);
}

bool ImogenAudioProcessor::isMidiLatched() const
{
    return isUsingDoublePrecision() ? doubleEngine.isMidiLatched() : floatEngine.isMidiLatched();
}


bool ImogenAudioProcessor::isAbletonLinkEnabled() const
{
#if IMOGEN_USE_ABLETON_LINK
    return abletonLink.isEnabled();
#else
    return false;
#endif
}

int ImogenAudioProcessor::getNumAbletonLinkSessionPeers() const
{
#if IMOGEN_USE_ABLETON_LINK
    return abletonLink.isEnabled() ? static_cast<int>(abletonLink.numPeers()) : 0;
#else
    return 0;
#endif
}


bool ImogenAudioProcessor::isConnectedToMtsEsp() const noexcept
{
    return isUsingDoublePrecision() ? doubleEngine.isConnectedToMtsEsp() : floatEngine.isConnectedToMtsEsp();
}

juce::String ImogenAudioProcessor::getScaleName() const
{
    return isUsingDoublePrecision() ? doubleEngine.getScaleName() : floatEngine.getScaleName();
}

/*===========================================================================================================================
 ============================================================================================================================*/

double ImogenAudioProcessor::getTailLengthSeconds() const
{
    return static_cast<double> (getParameterPntr(adsrReleaseID)->getCurrentNormalizedValue());
}


inline juce::AudioProcessor::BusesProperties ImogenAudioProcessor::makeBusProperties() const
{
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto mono   = juce::AudioChannelSet::mono();

    return BusesProperties().withInput (TRANS ("Input"),     stereo, true)
                            .withInput (TRANS ("Sidechain"), mono,   false)
                            .withOutput(TRANS ("Output"),    stereo, true);
}


bool ImogenAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto disabled = juce::AudioChannelSet::disabled();
    
    if (layouts.getMainInputChannelSet() == disabled && layouts.getChannelSet(true, 1) == disabled)
        return false;
    
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
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
    juce::Timer::callAfterDelay (20, [&](){ treeSync.sendFullSyncCallback(); });
    return new ImogenAudioProcessorEditor(*this);
#endif
}

ImogenGUIUpdateReciever* ImogenAudioProcessor::getActiveGuiEventReciever() const
{
#if IMOGEN_HEADLESS
    return nullptr;
#else
    if (auto* editor = getActiveEditor())
        return dynamic_cast<ImogenGUIUpdateReciever*> (editor);
    
    return nullptr;
#endif
}

void ImogenAudioProcessor::saveEditorSize (int width, int height)
{
    savedEditorSize.setX (width);
    savedEditorSize.setY (height);
    
    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
        saveEditorSizeToValueTree();
}


void ImogenAudioProcessor::saveEditorSizeToValueTree()
{
    if (auto* editor = getActiveEditor())
    {
        savedEditorSize.x = editor->getWidth();
        savedEditorSize.y = editor->getHeight();
    }
    
    auto editorSize = state.getOrCreateChildWithName (ValueTreeIDs::SavedEditorSize, nullptr);
    
    editorSize.setProperty (ValueTreeIDs::SavedEditorSize_X, savedEditorSize.x, nullptr);
    editorSize.setProperty (ValueTreeIDs::SavedEditorSize_Y, savedEditorSize.y, nullptr);
}

void ImogenAudioProcessor::updateEditorSizeFromValueTree()
{
    auto editorSize = state.getChildWithName (ValueTreeIDs::SavedEditorSize);
    
    if (editorSize.isValid())
    {
        savedEditorSize.x = editorSize.getProperty (ValueTreeIDs::SavedEditorSize_X);
        savedEditorSize.y = editorSize.getProperty (ValueTreeIDs::SavedEditorSize_Y);
    }
    
    if (auto* editor = getActiveEditor())
        editor->setSize (savedEditorSize.x, savedEditorSize.y);
    
    updateHostDisplay();
}


/*===========================================================================================================================
 ============================================================================================================================*/

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImogenAudioProcessor();
}
