
#pragma once
#if ! IMOGEN_HEADLESS

#    include "../PluginProcessor/PluginProcessor.h"
#    include "ImogenGUI/ImogenGUI.h"


namespace Imogen
{
class Editor : public gui::EditorBase
{
public:
    Editor (Processor& p);
    ~Editor() override;

private:
    void paint (juce::Graphics&) override final;
    void resizeTriggered() override final;

    GUI gui;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Editor)
};

}  // namespace Imogen

#endif /* if ! IMOGEN_HEADLESS */
