
#include "PluginProcessor.h"

#if ! IMOGEN_HEADLESS
#    include <ImogenGUI/ImogenGUI.h>
#endif


namespace Imogen
{
Processor::Processor()
    : ProcessorBase (state.parameters,
                     floatEngine, doubleEngine,
                     BusesProperties()
                         .withInput (TRANS ("Input"), juce::AudioChannelSet::stereo(), true)
                         .withInput (TRANS ("Sidechain"), juce::AudioChannelSet::mono(), false)
                         .withOutput (TRANS ("Output"), juce::AudioChannelSet::stereo(), true))
{
    state.addTo (this);
    parameters.addDataChild (dataSync);
    dataSync.connect ("host");
}

BoolParameter& Processor::getMainBypass() const
{
    return *parameters.mainBypass.get();
}

double Processor::getTailLengthSeconds() const
{
    return parameters.adsrRelease->get();
}

bool Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet().isDisabled() && layouts.getChannelSet (true, 1).isDisabled()) return false;

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
    return new gui::PluginEditor<GUI> (*this, {450, 300}, state);
#endif
}


}  // namespace Imogen

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Imogen::Processor();
}
