
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

#if IMOGEN_USE_OSC
  #include "../../../OSC/OSC.h"
#endif


using namespace Imogen;


class MainComponent  : public juce::Component,
                       public ImogenGuiHolder
{
public:
    MainComponent();
    
    ~MainComponent() override;

    /*=========================================================================================*/
    /* ProcessorStateChangeSender functions */
    
    void sendParameterChange  (ParameterID param, float newValue) override final;
    void sendParameterGesture (ParameterID param, bool gestureStart) override final;
    
    /*=========================================================================================*/
    /* juce::Component functions */
    
    void paint (juce::Graphics&) override final;
    void resized() override final;
    
    /*=========================================================================================*/

private:
#if IMOGEN_USE_OSC
    juce::OSCReceiver oscReceiver;
    ImogenOSCReciever<juce::OSCReceiver::MessageLoopCallback> oscParser;
    
    ImogenOSCSender   oscSender;
#endif
    
#if JUCE_OPENGL
    OpenGLContext openGLContext;
#endif
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
