#if JucePlugin_Build_Standalone && JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP

#include "StandaloneWrapper.h"

StandaloneFilterApp::StandaloneFilterApp()
{
    juce::PluginHostType::jucePlugInClientCurrentWrapperType = juce::AudioProcessor::wrapperType_Standalone;

    appProperties.setStorageParameters (PropertiesFileOptions());
}

juce::StandaloneFilterWindow* StandaloneFilterApp::createWindow()
{
    auto window = new juce::StandaloneFilterWindow (getApplicationName(),
                                                    getBackgroundColor(),
                                                    appProperties.getUserSettings(),
                                                    false,  // take ownership of settings
                                                    getDefaultAudioDeviceName(),  
                                                    getDefaultAudioDeviceSetup(),
                                                    {},
                                                    true); // auto open midi devices
    
    window->setTitleBarTextCentred (true);
    window->setUsingNativeTitleBar (false);
    
    window->setIcon (juce::ImageCache::getFromMemory (BinaryData::imogen_icon_png, BinaryData::imogen_icon_pngSize));
    
    window->setVisible (true);
    
    if (isMobileApp())
    {
        window->setTitleBarHeight(0);
        window->setFullScreen (true);
        window->setDropShadowEnabled (false);
    }
    else
    {
        window->setDropShadowEnabled (true);
    }
    
    return window;
}

void StandaloneFilterApp::initialise (const juce::String&)
{
    mainWindow.reset (createWindow());
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

bool StandaloneFilterApp::backButtonPressed()
{
    return false;
}


JUCE_CREATE_APPLICATION_DEFINE(StandaloneFilterApp)


#if JUCE_IOS

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

#endif  /* JUCE_IOS */

#endif /* JucePlugin_Build_Standalone && JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP */
