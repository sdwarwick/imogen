
#pragma once

#include "ImogenGUI/ImogenGUI.h"


using namespace Imogen;


class MainComponent : public juce::Component,
                      private bav::SystemInitializer
{
public:
    MainComponent();

    ~MainComponent() override;

    /*=========================================================================================*/

    /*=========================================================================================*/
    /* juce::Component functions */

    void paint (juce::Graphics&) override final;
    void resized() override final;

    /*=========================================================================================*/

private:
    Imogen::State state;
    ImogenGUI gui {state};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
