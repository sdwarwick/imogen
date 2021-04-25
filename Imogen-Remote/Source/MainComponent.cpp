#include "MainComponent.h"


MainComponent::MainComponent()
    : oscParser(this)
{
    this->setBufferedToImage (true);
    
    addAndMakeVisible (gui());
    
    setSize (800, 2990);
    
    oscReceiver.addListener (&oscParser);
    
#if JUCE_OPENGL
    openGLContext.attachTo (*getTopLevelComponent());
#endif
}

MainComponent::~MainComponent()
{
    oscReceiver.removeListener (&oscParser);
    
#if JUCE_OPENGL
    openGLContext.detach();
#endif
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    gui()->setBounds (0, 0, getWidth(), getHeight());
}


//==============================================================================

void MainComponent::sendParameterChange (int paramID, float newValue) 
{
    juce::ignoreUnused (paramID, newValue);
}

void MainComponent::startParameterChangeGesture (int paramID)
{
    juce::ignoreUnused (paramID);
}

void MainComponent::endParameterChangeGesture (int paramID)
{
    juce::ignoreUnused (paramID);
}


void MainComponent::sendEditorPitchbend (int wheelValue) 
{
    juce::ignoreUnused (wheelValue);
}


void MainComponent::sendMidiLatch (bool shouldBeLatched) 
{
    juce::ignoreUnused (shouldBeLatched);
}


void MainComponent::loadPreset (const juce::String& presetName) 
{
    juce::ignoreUnused (presetName);
}

void MainComponent::savePreset (const juce::String& presetName) 
{
    juce::ignoreUnused (presetName);
}

void MainComponent::deletePreset (const juce::String& presetName) 
{
    juce::ignoreUnused (presetName);
}


void MainComponent::enableAbletonLink (bool shouldBeEnabled)
{
    juce::ignoreUnused (shouldBeEnabled);
}
