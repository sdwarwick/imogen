/*
    This file defines Imogen's internal audio processor as a whole, when Imogen is built as a plugin target
    Parent file: PluginProcessor.h
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"


ImogenAudioProcessor::ImogenAudioProcessor():
    AudioProcessor(makeBusProperties()),
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    needsSidechain(false),
#endif
    tree(*this, nullptr, "IMOGEN_PARAMETERS", createParameters())
{
    initializeParameterPointers();
    updateParameterDefaults();
    
    if (isUsingDoublePrecision())
        initialize (doubleEngine);
    else
        initialize (floatEngine);
}

ImogenAudioProcessor::~ImogenAudioProcessor()
{ }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename SampleType>
inline void ImogenAudioProcessor::initialize (bav::ImogenEngine<SampleType>& activeEngine)
{
    double initSamplerate = getSampleRate();
    if (initSamplerate <= 0.0) initSamplerate = 44100.0;
    
    int initBlockSize = getBlockSize();
    if (initBlockSize <= 0) initBlockSize = 512;
    
    activeEngine.initialize (initSamplerate, initBlockSize);
    
    updateAllParameters (activeEngine);
    
    setLatencySamples (activeEngine.reportLatency());
}


void ImogenAudioProcessor::prepareToPlay (const double sampleRate, const int samplesPerBlock)
{
    if (isUsingDoublePrecision())
        prepareToPlayWrapped (sampleRate, doubleEngine, floatEngine);
    else
        prepareToPlayWrapped (sampleRate, floatEngine,  doubleEngine);
    
    paramChangesForProcessor.reserveSize (samplesPerBlock);
    paramChangesForEditor.reserveSize (samplesPerBlock);
    
    currentMessages.ensureStorageAllocated (samplesPerBlock);
}


template <typename SampleType1, typename SampleType2>
inline void ImogenAudioProcessor::prepareToPlayWrapped (const double sampleRate,
                                                        bav::ImogenEngine<SampleType1>& activeEngine,
                                                        bav::ImogenEngine<SampleType2>& idleEngine)
{
    if (! idleEngine.hasBeenReleased())
        idleEngine.releaseResources();
    
    updateAllParameters (activeEngine);
    
    jassert (activeEngine.getLatency() > 0);
    
    activeEngine.prepare (sampleRate);
    
    setLatencySamples (activeEngine.reportLatency());
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    needsSidechain = (host.isLogic() || host.isGarageBand());
#endif
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

/*
 These four functions represent the top-level callbacks made by the host during audio processing. Audio samples may be sent to us as float or double values; both of these functions redirect to the templated processBlockWrapped() function below.
 The buffers sent to this function by the host may be variable in size, so I have coded defensively around several edge cases & possible buggy host behavior and created several layers of checks that each callback passes through before individual chunks of audio are actually rendered.
*/

void ImogenAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, floatEngine, mainBypass->get());
}


void ImogenAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, doubleEngine, mainBypass->get());
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, floatEngine, true);
    
    if (! mainBypass->get())
        mainBypass->setValueNotifyingHost (1.0f);
}


void ImogenAudioProcessor::processBlockBypassed (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    processBlockWrapped (buffer, midiMessages, doubleEngine, true);
    
    if (! mainBypass->get())
        mainBypass->setValueNotifyingHost (1.0f);
}


// LAYER 2 ---------------------------------------------------------------------------------

template <typename SampleType>
inline void ImogenAudioProcessor::processBlockWrapped (juce::AudioBuffer<SampleType>& buffer,
                                                       juce::MidiBuffer& midiMessages,
                                                       bav::ImogenEngine<SampleType>& engine,
                                                       const bool isBypassedThisCallback)
{
    jassert (! engine.hasBeenReleased() && engine.hasBeenInitialized());
    
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    if (needsSidechain && (getBusesLayout().getChannelSet(true, 1) == juce::AudioChannelSet::disabled()))
        return;   // our audio input is disabled! can't do processing
#endif
    
    processQueuedParameterChanges (engine); // the host might use a 0-sample long audio buffer to tell us to update our state with new automation values, which is why the check for that is AFTER this call.
    
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
        return;
    
#if IMOGEN_ONLY_BUILDING_STANDALONE
    juce::AudioBuffer<SampleType> inBus = AudioProcessor::getBusBuffer (buffer, true, 0);
#else
    juce::AudioBuffer<SampleType> inBus = AudioProcessor::getBusBuffer (buffer, true, needsSidechain);
#endif
    
    juce::AudioBuffer<SampleType> outBus = AudioProcessor::getBusBuffer (buffer, false, 0);
    
    engine.process (inBus, outBus, midiMessages, isBypassedThisCallback);
}


/*===========================================================================================================================
 ============================================================================================================================*/


// standard and general-purpose functions -----------------------------------------------------------------------------------------------------------

double ImogenAudioProcessor::getTailLengthSeconds() const
{
    if (adsrToggle->get())
        return double(adsrRelease->get()); // ADSR release time in seconds
    
    return 0.005;  // "quick kill" time in seconds -- must be the same as the ms value defined in the macro in bv_Harmonizer.cpp !!
}


inline juce::AudioProcessor::BusesProperties ImogenAudioProcessor::makeBusProperties() const
{
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    if (host.isLogic() || host.isGarageBand())
        return BusesProperties().withInput ("Input",     juce::AudioChannelSet::stereo(), true)
                                .withInput ("Sidechain", juce::AudioChannelSet::mono(),   true)
                                .withOutput("Output",    juce::AudioChannelSet::stereo(), true);
#endif

    return BusesProperties().withInput ("Input",  juce::AudioChannelSet::stereo(), true)
                            .withOutput("Output", juce::AudioChannelSet::stereo(), true);
}


bool ImogenAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled())
    {
#if IMOGEN_ONLY_BUILDING_STANDALONE
        return false;
#else
        if (needsSidechain)
        {
            if (layouts.getChannelSet (true, 1) == juce::AudioChannelSet::disabled())
                return false;
        }
        else
        {
            return false;
        }
#endif
    }
    
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}


bool ImogenAudioProcessor::canAddBus (bool isInput) const
{
#if ! IMOGEN_ONLY_BUILDING_STANDALONE
    if (needsSidechain)
        return isInput;
#else
    juce::ignoreUnused (isInput);
#endif
    
    return false;
}


#if ! IMOGEN_ONLY_BUILDING_STANDALONE
bool ImogenAudioProcessor::shouldWarnUserToEnableSidechain() const
{
    return needsSidechain && (getBusesLayout().getChannelSet(true, 1) == juce::AudioChannelSet::disabled());
}
#endif


juce::AudioProcessorEditor* ImogenAudioProcessor::createEditor()
{
    return new ImogenAudioProcessorEditor(*this);
}


// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImogenAudioProcessor();
}
