
namespace Imogen
{
Processor::Processor()
    : dsp::Processor< State, Engine > (BusesProperties()
                                           .withInput (TRANS ("Input"), juce::AudioChannelSet::stereo(), true)
                                           .withInput (TRANS ("Sidechain"), juce::AudioChannelSet::mono(), false)
                                           .withOutput (TRANS ("Output"), juce::AudioChannelSet::stereo(), true))
{
    parameters.addDataChild (dataSync);
    dataSync.connect ("host");
}

double Processor::getTailLengthSeconds() const
{
    return parameters.midiState.adsrRelease->get();
}

bool Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet().isDisabled() && layouts.getChannelSet (true, 1).isDisabled()) return false;

    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}


}  // namespace Imogen
