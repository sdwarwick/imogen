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

void CenterDial::showParameter (Parameter& param)
{
    const auto n = juce::NotificationType::dontSendNotification;
    
    mainText.setText (param.rap.getCurrentValueAsText(), n);
    description.setText (param.parameterNameVerbose, n);
    
//    const auto range = param.getRange();
//
//    leftEnd.setText (range.start, n);
//    rightEnd.setText (range.end, n);
    
    setTooltip (param.parameterNameVerbose);
}

void CenterDial::showPitchCorrection()
{
    const auto n = juce::NotificationType::dontSendNotification;
    
    mainText.setText (state.internals.currentInputNote->getCurrentValueAsText(), n);
    description.setText (TRANS ("Pitch correction"), n);
    leftEnd.setText (TRANS ("Flat"), n);
    rightEnd.setText (TRANS ("Sharp"), n);
    
    setTooltip (TRANS ("Pitch correction"));
}

}
