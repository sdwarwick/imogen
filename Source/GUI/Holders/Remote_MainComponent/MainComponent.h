
/*================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/       _____  ______ __  __  ____ _______ ______
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /       |  __ \|  ____|  \/  |/ __ \__   __|  ____|
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /        | |__) | |__  | \  / | |  | | | |  | |__
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /         |  _  /|  __| | |\/| | |  | | | |  |  __|
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /          | | \ \| |____| |  | | |__| | | |  | |____
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/           |_|  \_\______|_|  |_|\____/  |_|  |______|
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 MainComponent.h :     This file defines main content component for the Imogen Remote app, which contains the Imgogen GUI and wraps it with its networking capabilities.
 
 ================================================================================================================================*/


#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "../ImogenGuiHolder.h"

#include "../../../OSC/OSC_sender.h"
#include "../../../OSC/OSC_reciever.h"


using namespace Imogen;


class MainComponent  : public juce::Component,
                       public ImogenGuiHolder
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override final;
    void resized() override final;
    
    //==============================================================================
    void sendParameterChange (ParameterID paramID, float newValue) override final;
    void sendParameterChangeGestureStart (ParameterID paramID) override final;
    void sendParameterChangeGestureEnd   (ParameterID paramID) override final;
    
    void sendEditorPitchbend (int wheelValue) override final;
    void sendKillAllMidiEvent() override final;
    void sendMidiLatch (bool shouldBeLatched) override final;
    
    void sendLoadPreset   (const juce::String& presetName) override final;
    void sendSavePreset   (const juce::String& presetName) override final;
    void sendDeletePreset (const juce::String& presetName) override final;
    
    void sendEnableAbletonLink (bool shouldBeEnabled) override final;
    
    void sendErrorCode (ErrorCode) override final { }


private:
    juce::OSCReceiver oscReceiver;
    ImogenOSCReciever oscParser;
    
    ImogenOSCSender   oscSender;
    
#if JUCE_OPENGL
    OpenGLContext openGLContext;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
