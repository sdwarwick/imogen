#if JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP

#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#include <juce_data_structures/app_properties/juce_PropertiesFile.h>


extern juce::JUCEApplicationBase* juce_CreateApplication();

class StandaloneFilterApp : public juce::JUCEApplication
{
public:
    StandaloneFilterApp();

    const juce::String getApplicationName() override { return JucePlugin_Name; }
    const juce::String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override { return true; }
    void anotherInstanceStarted(const juce::String&) override {}

    virtual juce::StandaloneFilterWindow* createWindow();

    void initialise(const juce::String&) override;
    void shutdown() override;
    void systemRequestedQuit() override;

private:
    juce::ApplicationProperties appProperties;
    std::unique_ptr<juce::StandaloneFilterWindow> mainWindow;
    void requestQuit() const;
};


struct PropertiesFileOptions : public PropertiesFile::Options
{
    PropertiesFileOptions()
    {
        applicationName = JucePlugin_Name;
        filenameSuffix = ".settings";
        osxLibrarySubFolder = "Application Support";
        folderName = getOptionsFolderName();
    }

    static String getOptionsFolderName()
    {
#if JUCE_LINUX
        return "~/.config";
#else
        return "";
#endif
    }
};


#endif /* JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP */
