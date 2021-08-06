#pragma once

namespace Imogen
{
class Remote : public juce::Component
{
public:
    Remote();
    ~Remote() override;

private:
    void paint (juce::Graphics&) final;
    void resized() final;

    State                state;
    plugin::StateToggler toggler {state};
    UndoManager          undo {state};

    GUI gui {state, toggler, undo};

    // network::OscDataSynchronizer dataSync {state};
};

}  // namespace Imogen
