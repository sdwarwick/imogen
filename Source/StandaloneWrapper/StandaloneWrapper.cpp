#if JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP

#include "StandaloneWrapper.h"

StandaloneFilterApp::StandaloneFilterApp()
{
    juce::PluginHostType::jucePlugInClientCurrentWrapperType = juce::AudioProcessor::wrapperType_Standalone;

    appProperties.setStorageParameters (PropertiesFileOptions());
}

juce::StandaloneFilterWindow* StandaloneFilterApp::createWindow()
{
    return new juce:: StandaloneFilterWindow (getApplicationName(),
                                              juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId),
                                              appProperties.getUserSettings(),
                                              false,
                                              {},
                                              nullptr,
                                              {},
                                              true);
}

void StandaloneFilterApp::initialise (const juce::String&)
{
    mainWindow.reset (createWindow());
    mainWindow->setVisible (true);
}

void StandaloneFilterApp::shutdown()
{
    mainWindow = nullptr;
    appProperties.saveIfNeeded();
}

void StandaloneFilterApp::systemRequestedQuit()
{
    if (mainWindow != nullptr)
        mainWindow->pluginHolder->savePluginState();

    if (juce::ModalComponentManager::getInstance()->cancelAllModalComponents())
        juce::Timer::callAfterDelay(100, [&]() { requestQuit(); });
    else
        quit();
}

void StandaloneFilterApp::requestQuit() const
{
    if (auto app = getInstance())
        app->systemRequestedQuit();
}


juce::JUCEApplicationBase* juce_CreateApplication()
{
    return new StandaloneFilterApp();
}


#if JucePlugin_Build_Standalone && JUCE_IOS

bool JUCE_CALLTYPE juce_isInterAppAudioConnected()
{
    if (auto holder = juce::StandalonePluginHolder::getInstance())
        return holder->isInterAppAudioConnected();

    return false;
}

void JUCE_CALLTYPE juce_switchToHostApplication()
{
    if (auto holder = juce::StandalonePluginHolder::getInstance())
        holder->switchToHostApplication();
}

juce::Image JUCE_CALLTYPE juce_getIAAHostIcon (int size)
{
    if (auto holder = juce::StandalonePluginHolder::getInstance())
        return holder->getIAAHostIcon (size);

    return juce::Image();
}

#endif  /* JucePlugin_Build_Standalone && JUCE_IOS */


#endif /* JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP */
