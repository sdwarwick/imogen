
#pragma once

namespace Imogen
{
class PresetBar : public juce::Component
{
public:
    PresetBar (State& stateToUse, UndoManager& undoToUse);

private:
    State&       state;
    UndoManager& undo;

    PresetManager presetManager {state, &undo};
};

}  // namespace Imogen
