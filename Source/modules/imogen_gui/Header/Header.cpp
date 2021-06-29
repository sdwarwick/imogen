
namespace Imogen
{

Header::Header (State& stateToUse, UndoManager& undoToUse)
: state (stateToUse), undo (undoToUse)
{
    gui::addAndMakeVisible (this, inputIcon, outputLevel, presetBar, scale);
}

void Header::paint (juce::Graphics&)
{
    
}

void Header::resized()
{
    // input icon, outputLevel, presetBar, scale
}

}
