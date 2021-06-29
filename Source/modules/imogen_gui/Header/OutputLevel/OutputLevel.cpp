
namespace Imogen
{

OutputLevel::OutputLevel (State& stateToUse)
: state (stateToUse)
{
    gui::addAndMakeVisible (this, thumb, meter);
}

void OutputLevel::paint (juce::Graphics&)
{
    
}

void OutputLevel::resized()
{
    // thumb, meter
}

}
