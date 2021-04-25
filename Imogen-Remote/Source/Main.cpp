
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "BinaryData.h"

#include "MainComponent.h"


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
        
        auto window = new MainWindow (getApplicationName());
        
        window->setTitleBarTextCentred (true);
        window->setUsingNativeTitleBar (false);
        
        window->setIcon (juce::ImageCache::getFromMemory (BinaryData::imogen_icon_png, BinaryData::imogen_icon_pngSize));
        
        if (! isMobileApp())
            window->setDropShadowEnabled (true);

        mainWindow.reset (window);
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
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

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
