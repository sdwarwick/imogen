
namespace Imogen
{
OutputLevelMeter::OutputLevelMeter (Meters& metersToUse)
    : meters (metersToUse)
{
}

void OutputLevelMeter::paint (juce::Graphics&)
{
}

void OutputLevelMeter::resized()
{
}


OutputLevelMeter::Bar::Bar (plugin::GainMeterParameter& meter)
    : level (meter)
{
    juce::ignoreUnused (level);
}

void OutputLevelMeter::Bar::paint (juce::Graphics&)
{
}

void OutputLevelMeter::Bar::resized()
{
}

}  // namespace Imogen
