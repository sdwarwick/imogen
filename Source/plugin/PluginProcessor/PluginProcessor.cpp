
#if ! IMOGEN_HEADLESS
#    include "../PluginEditor/PluginEditor.h"
#endif

#include "PluginProcessor.h"


namespace Imogen
{
Processor::Processor()
    : ProcessorBase (state.parameters)
{
    state.addTo (*this);
    
    dataSync.connect ("host");

    if (isUsingDoublePrecision())
        initialize (doubleEngine);
    else
        initialize (floatEngine);
}

Processor::~Processor()
{
    dataSync.disconnect();
}

template < typename SampleType >
inline void Processor::initialize (Engine< SampleType >& activeEngine)
{
    auto initSamplerate = getSampleRate();
    if (initSamplerate <= 0.0) initSamplerate = 44100.0;

    auto initBlockSize = getBlockSize();
    if (initBlockSize <= 0) initBlockSize = 512;

    prepareToPlay (initSamplerate, 512);
    setLatencySamples (activeEngine.reportLatency());
}

void Processor::prepareToPlay (const double sampleRate, const int)
{
    if (isUsingDoublePrecision())
        prepareToPlayWrapped (sampleRate, doubleEngine, floatEngine);
    else
        prepareToPlayWrapped (sampleRate, floatEngine, doubleEngine);
}

template < typename SampleType1, typename SampleType2 >
inline void Processor::prepareToPlayWrapped (const double           sampleRate,
                                             Engine< SampleType1 >& activeEngine,
                                             Engine< SampleType2 >& idleEngine)
{
    if (idleEngine.isInitialized())
        idleEngine.releaseResources();

    jassert (activeEngine.reportLatency() > 0);

    activeEngine.prepare (sampleRate, activeEngine.reportLatency());

    setLatencySamples (activeEngine.reportLatency());
}

BoolParameter& Processor::getMainBypass() const
{
    return *parameters.mainBypass.get();
}

IntParameter& Processor::getPitchbendParam()
{
    return *parameters.editorPitchbend.get();
}

void Processor::releaseResources()
{
    if (doubleEngine.isInitialized())
        doubleEngine.releaseResources();

    if (floatEngine.isInitialized())
        floatEngine.releaseResources();
}

void Processor::renderChunk (juce::AudioBuffer< float >& audio, juce::MidiBuffer& midi)
{
    renderChunkInternal (floatEngine, audio, midi);
}

void Processor::renderChunk (juce::AudioBuffer< double >& audio, juce::MidiBuffer& midi)
{
    renderChunkInternal (doubleEngine, audio, midi);
}

template < typename SampleType >
void Processor::renderChunkInternal (Engine< SampleType >& engine, juce::AudioBuffer< SampleType >& audio, juce::MidiBuffer& midi)
{
    auto inBus  = getBusBuffer (audio, true, getBusesLayout().getMainInputChannelSet() == juce::AudioChannelSet::disabled());
    auto outBus = getBusBuffer (audio, false, 0);

    engine.process (inBus, outBus, midi, parameters.mainBypass->get());
}

double Processor::getTailLengthSeconds() const
{
    return parameters.adsrRelease->get();
}

juce::AudioProcessor::BusesProperties Processor::createBusProperties() const
{
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto mono   = juce::AudioChannelSet::mono();

    return BusesProperties()
        .withInput (TRANS ("Input"), stereo, true)
        .withInput (TRANS ("Sidechain"), mono, false)
        .withOutput (TRANS ("Output"), stereo, true);
}

bool Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto disabled = juce::AudioChannelSet::disabled();

    if (layouts.getMainInputChannelSet() == disabled && layouts.getChannelSet (true, 1) == disabled) return false;

    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

bool Processor::hasEditor() const
{
#if IMOGEN_HEADLESS
    return false;
#endif
    return true;
}

juce::AudioProcessorEditor* Processor::createEditor()
{
#if IMOGEN_HEADLESS
    return nullptr;
#else
    return new Editor (*this);
#endif
}


}  // namespace Imogen

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Imogen::Processor();
}
