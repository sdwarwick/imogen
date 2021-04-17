#if JucePlugin_Build_Standalone && JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP

#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

#include "bv_SharedCode/BinaryDataHelpers.h"


class StandaloneFilterApp : public juce::JUCEApplication
{
public:
    StandaloneFilterApp();

    const juce::String getApplicationName() override { return JucePlugin_Name; }
    const juce::String getApplicationVersion() override { return JucePlugin_VersionString; }
    
    const juce::String getDefaultAudioDeviceName() const { return juce::String(); }
    juce::AudioDeviceManager::AudioDeviceSetup* getDefaultAudioDeviceSetup() const { return nullptr; }
    
    juce::Colour getBackgroundColor() const { return juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId); } 
    
    bool moreThanOneInstanceAllowed() override { return true; }
    void anotherInstanceStarted (const juce::String&) override { }

    virtual juce::StandaloneFilterWindow* createWindow();

    void initialise (const juce::String&) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    
    virtual bool backButtonPressed() override;
    
    
    static bool isDesktopStandaloneApp() 
    {
#if JUCE_IOS || JUCE_ANDROID
        return false;
#else  
        return true;
#endif
    }
    
    static bool isMobileApp() { return ! isDesktopStandaloneApp(); }

private:
    juce::ApplicationProperties appProperties;
    std::unique_ptr<juce::StandaloneFilterWindow> mainWindow;
    void requestQuit() const;
};


struct PropertiesFileOptions : public juce::PropertiesFile::Options
{
    PropertiesFileOptions()
    {
        applicationName = JucePlugin_Name;
        filenameSuffix = ".settings";
        osxLibrarySubFolder = "Application Support";
        folderName = getOptionsFolderName();
    }

    static juce::String getOptionsFolderName()
    {
#if JUCE_LINUX
        return "~/.config";
#else
        return "";
#endif
    }
};


#endif /* JucePlugin_Build_Standalone && JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP */
