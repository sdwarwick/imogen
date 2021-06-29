
namespace Imogen
{

Header::Header (State& stateToUse, UndoManager& undoToUse)
: state (stateToUse), undo (undoToUse)
{
    gui::addAndMakeVisible (this, logo, inputIcon, outputLevel, presetBar, scale);
}

void Header::paint (juce::Graphics&)
{
    
}

void Header::resized()
{
    // logo, input icon, outputLevel, presetBar, scale
}

}
