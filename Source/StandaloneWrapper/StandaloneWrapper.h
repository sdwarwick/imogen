#pragma once

#include <juce_audio_plugin_client/utility/juce_PluginHostType.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

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

    void initialise(const String&) override;
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



