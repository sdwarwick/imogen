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
 
 Main.cpp :     This file defines the application base for the Imogen Remote app.
 
================================================================================================================================*/


#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "imogen/Source/GUI/Holders/Remote_MainComponent/MainComponent.h"


//==============================================================================
class ImogenRemoteApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    ImogenRemoteApplication()
    {
        juce::PluginHostType::jucePlugInClientCurrentWrapperType = juce::AudioProcessor::wrapperType_Standalone;
    }

    const juce::String getApplicationName() override       { return juce::String("Imogen ") + TRANS("Remote"); }
    const juce::String getApplicationVersion() override    { return { "0.0.1" }; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        juce::ignoreUnused (commandLine);
        mainWindow.reset (new MainWindow (getApplicationName(), getBackgroundColor()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        if (juce::ModalComponentManager::getInstance()->cancelAllModalComponents())
            juce::Timer::callAfterDelay(100, [&]() { requestQuit(); });
        else
            quit();
    }
    
    void requestQuit() const
    {
        if (auto app = getInstance())
            app->systemRequestedQuit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        juce::ignoreUnused (commandLine);
    }
    
    //==============================================================================
    
    bool backButtonPressed() override
    {
        return false;
    }
    
    //==============================================================================
    
    void suspended() override
    {
        
    }
    
    void resumed() override
    {
        
    }
    
    //==============================================================================
    
    static bool isDesktopStandaloneApp()
    {
#if JUCE_IOS || JUCE_ANDROID
        return false;
#else
        return true;
#endif
    }
    
    static bool isMobileApp()
    {
        return ! isDesktopStandaloneApp();
    }

    //==============================================================================
    
    juce::Colour getBackgroundColor() const
    {
        return juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    }
    
    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name, juce::Colour backgroundColour)
            : DocumentWindow (name, backgroundColour, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (false);
            setTitleBarTextCentred (true);
            
            setContentOwned (new MainComponent(), true);

          #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
          #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
            setDropShadowEnabled (true);
          #endif
            
            setIcon (juce::ImageCache::getFromMemory (BinaryData::imogen_icon_png,
                                                      BinaryData::imogen_icon_pngSize));

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};


//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (ImogenRemoteApplication)
