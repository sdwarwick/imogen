#pragma once

namespace Imogen
{

class CenterDial : public juce::Component, public juce::SettableTooltipClient
{
public:
    CenterDial (State& stateToUse);

    void paint (juce::Graphics& g) override final;

    void resized() override final;

    bool hitTest (int x, int y) override final;

    bool keyPressed (const juce::KeyPress& key) override final;
    bool keyStateChanged (bool isKeyDown) override final;
    void modifierKeysChanged (const juce::ModifierKeys& modifiers) override final;
    void focusLost (FocusChangeType cause) override final;
    
private:
    void showParameter (Parameter& param);
    void showPitchCorrection();
    
    State& state;
    
    gui::Label mainText;
    gui::Label description;
    gui::Label leftEnd;
    gui::Label rightEnd;
    
    ParameterList::Listener l {state.parameters,
                               [&](Parameter& param) { showParameter (param); },
                               [&](Parameter&, bool starting) { if (! starting) showPitchCorrection(); }};
};

}
