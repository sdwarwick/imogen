
#pragma once

#include "ImogenGUI/ImogenGUI.h"


namespace Imogen
{

class Remote : public juce::Component,
               private SystemInitializer
{
public:
    Remote();
    ~Remote() override;

private:
    void paint (juce::Graphics&) final;
    void resized() final;

    State state;
    GUI   gui {state};

    network::SelfOwnedOscDataSynchronizer dataSync {state};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Remote)
};

}  // namespace
