namespace Imogen
{
CenterDial::CenterDial (State& stateToUse)
    : state (stateToUse)
{
    setOpaque (true);
    setInterceptsMouseClicks (true, true);

    showPitchCorrection();
}


void CenterDial::paint (juce::Graphics& g)
{
    juce::Graphics::ScopedSaveState graphicsState (g);

    juce::ignoreUnused (g);
}

void CenterDial::resized()
{
}


bool CenterDial::hitTest (int x, int y)
{
    juce::ignoreUnused (x, y);
    return false;
}


bool CenterDial::keyPressed (const juce::KeyPress& key)
{
    juce::ignoreUnused (key);
    return false;
}

bool CenterDial::keyStateChanged (bool isKeyDown)
{
    juce::ignoreUnused (isKeyDown);
    return false;
}

void CenterDial::modifierKeysChanged (const juce::ModifierKeys& modifiers)
{
    juce::ignoreUnused (modifiers);
}

void CenterDial::focusLost (FocusChangeType cause)
{
    juce::ignoreUnused (cause);
}

void CenterDial::showParameter (plugin::Parameter& param)
{
    mainText.set (param.getCurrentValueAsText());
    description.set (param.getName());

    leftEnd.set (param.getTextForMin());
    rightEnd.set (param.getTextForMax());

    setTooltip (param.getName());
}

void CenterDial::showPitchCorrection()
{
    mainText.set (state.internals.currentInputNote->getCurrentValueAsText());
    description.set (TRANS ("Pitch correction"));
    leftEnd.set (TRANS ("Flat"));
    rightEnd.set (TRANS ("Sharp"));

    setTooltip (TRANS ("Pitch correction"));
}

}  // namespace Imogen
