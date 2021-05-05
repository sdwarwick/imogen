
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
  : bav::TranslationInitializer (findAppropriateTranslationFile()),
    AudioProcessor (makeBusProperties()),
    state (imogenValueTreeType()),
    treeSync (state, *this)
#if IMOGEN_USE_ABLETON_LINK
    , abletonLink (120.0) // constructed with the initial BPM
#endif
{
#if BV_USE_NE10
    ne10_init();
#endif
    
    addParameterGroup (createParameterTree());
    
    createValueTree (state, getParameterTree());
    
    jassert (getParameters().size() == numParams);
    
    if (isUsingDoublePrecision())
        initialize (doubleEngine);
    else
        initialize (floatEngine);
    
    initializeParameterPointers();
    
    updateEditorSizeFromAPVTS();
    
    treeSync.sendFullSyncCallback();
    
#if IMOGEN_PROCESSOR_TIMER
    constexpr int timerFramerate = 30;
    Timer::startTimerHz (timerFramerate);
#endif
}

ImogenAudioProcessor::~ImogenAudioProcessor()
{
#if IMOGEN_PROCESSOR_TIMER
    Timer::stopTimer();
#endif
}

/*===========================================================================================================
===========================================================================================================*/

#if IMOGEN_PROCESSOR_TIMER
void ImogenAudioProcessor::timerCallback()
{

}
#endif

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
    
    initializeParameterFunctionPointers (activeEngine);
}


void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock)
{
    if (isUsingDoublePrecision())
        prepareToPlayWrapped (sampleRate, doubleEngine, floatEngine);
    else
        prepareToPlayWrapped (sampleRate, floatEngine,  doubleEngine);
    
    currentMessages.ensureStorageAllocated (samplesPerBlock);
}


template <typename SampleType1, typename SampleType2>
inline void ImogenAudioProcessor::prepareToPlayWrapped (const double sampleRate,
                                                        bav::ImogenEngine<SampleType1>& activeEngine,
                                                        bav::ImogenEngine<SampleType2>& idleEngine)
{
    if (! idleEngine.hasBeenReleased())
        idleEngine.releaseResources();
    
    initializeParameterFunctionPointers (activeEngine);
    
    processQueuedNonParamEvents (activeEngine);
    
    jassert (activeEngine.getLatency() > 0);
    
    activeEngine.prepare (sampleRate);
    
    setLatencySamples (activeEngine.reportLatency());
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
                         mainBypassPntr->getCurrentNormalizedValue() >= 0.5f);
}


void ImogenAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, doubleEngine,
                         mainBypassPntr->getCurrentNormalizedValue() >= 0.5f);
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (! (mainBypassPntr->getCurrentNormalizedValue() >= 0.5f))
        mainBypassPntr->orig()->setValueNotifyingHost (1.0f);
    
    processBlockWrapped (buffer, midiMessages, floatEngine, true);
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    if (! (mainBypassPntr->getCurrentNormalizedValue() >= 0.5f))
        mainBypassPntr->orig()->setValueNotifyingHost (1.0f);
    
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
    
    processQueuedNonParamEvents (engine);
    
    /* update all parameters... */
    for (auto* pntr : parameterPointers)
        pntr->doAction();
    
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
        return;
    
    auto inBus  = getBusBuffer (buffer, true, getBusesLayout().getMainInputChannelSet() == juce::AudioChannelSet::disabled());
    auto outBus = getBusBuffer (buffer, false, 0);
    
    engine.process (inBus, outBus, midiMessages, isBypassedThisCallback);
}

/*===========================================================================================================================
 ============================================================================================================================*/

void ImogenAudioProcessor::changeMidiLatchState (bool isNowLatched)
{
    if (isUsingDoublePrecision())
        doubleEngine.updateMidiLatch (isNowLatched);
    else
        floatEngine.updateMidiLatch (isNowLatched);
    
#if IMOGEN_USE_OSC
    oscSender.sendMidiLatch (isNowLatched);
#endif
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
    return abletonLink.isEnabled() ? (int)abletonLink.numPeers() : 0;
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
    return double(getParameterPntr(adsrReleaseID)->getCurrentNormalizedValue());
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
    return new ImogenAudioProcessorEditor(*this);
#endif
}

ImogenGuiHolder* ImogenAudioProcessor::getActiveGui() const
{
#if IMOGEN_HEADLESS
    return nullptr;
#else
    if (auto* editor = getActiveEditor())
        return dynamic_cast<ImogenGuiHolder*> (editor);
    
    return nullptr;
#endif
}

void ImogenAudioProcessor::saveEditorSize (int width, int height)
{
    savedEditorSize.setX (width);
    savedEditorSize.setY (height);
}

void ImogenAudioProcessor::updateEditorSizeFromAPVTS()
{
    auto editor = state.getChildWithName ("editorSize");
    
    if (editor.isValid())
    {
        savedEditorSize.setX (editor.getProperty ("editorSize_X", 900));
        savedEditorSize.setY (editor.getProperty ("editorSize_Y", 500));
        
        if (auto* activeEditor = getActiveEditor())
            activeEditor->setSize (savedEditorSize.x, savedEditorSize.y);
    }
}

/*===========================================================================================================================
 ============================================================================================================================*/

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImogenAudioProcessor();
}
