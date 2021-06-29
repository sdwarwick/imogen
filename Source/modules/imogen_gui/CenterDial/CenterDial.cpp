namespace Imogen
{

CenterDial::CenterDial (State& stateToUse)
: state (stateToUse)
{
    setOpaque (true);
    setInterceptsMouseClicks (true, true);
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

void CenterDial::showParameter (Parameter& /*param*/)
{
    
}

void CenterDial::showPitchCorrection()
{
    
}

}
