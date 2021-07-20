
namespace Imogen
{
Header::Header (State& stateToUse, UndoManager& undoToUse)
    : state (stateToUse), undo (undoToUse)
{
    gui::addAndMakeVisible (this, logo, inputIcon, outputLevel, presetBar, scale, keyboardButton);
}

void Header::paint (juce::Graphics&)
{
}

void Header::resized()
{
    // logo, keyboardButton, input icon, outputLevel, presetBar, scale
}

}  // namespace Imogen
