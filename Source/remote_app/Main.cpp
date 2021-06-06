
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "MainComponent.h"


//==============================================================================
class ImogenRemoteApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    ImogenRemoteApplication() { juce::PluginHostType::jucePlugInClientCurrentWrapperType = juce::AudioProcessor::wrapperType_Standalone; }

    const juce::String getApplicationName() final { return juce::String ("Imogen ") + TRANS ("Remote"); }
    const juce::String getApplicationVersion() final { return {"0.0.1"}; }
    bool               moreThanOneInstanceAllowed() final { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) final
    {
        juce::ignoreUnused (commandLine);
        mainWindow.reset (new MainWindow (getApplicationName(), getBackgroundColor()));
    }

    void shutdown() final { mainWindow = nullptr; }

    //==============================================================================
    void systemRequestedQuit() final
    {
        if (juce::ModalComponentManager::getInstance()->cancelAllModalComponents())
            juce::Timer::callAfterDelay (100, [&]()
                                         { requestQuit(); });
        else
            quit();
    }

    void requestQuit() const
    {
        if (auto app = getInstance()) app->systemRequestedQuit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) final { juce::ignoreUnused (commandLine); }

    //==============================================================================

    bool backButtonPressed() final { return false; }

    //==============================================================================

    void suspended() final { }

    void resumed() final { }

    //==============================================================================

    static constexpr bool isDesktopStandaloneApp()
    {
#if JUCE_IOS || JUCE_ANDROID
        return false;
#else
        return true;
#endif
    }

    static constexpr bool isMobileApp() { return ! isDesktopStandaloneApp(); }

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
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name, juce::Colour backgroundColour)
            : DocumentWindow (name, backgroundColour, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (false);
            setTitleBarTextCentred (true);

            setContentOwned (new Imogen::Remote(), true);

#if JUCE_IOS || JUCE_ANDROID
            setDropShadowEnabled (false);
            setFullScreen (true);
#else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
            setDropShadowEnabled (true);
#endif

            setIcon (juce::ImageCache::getFromMemory (BinaryData::imogen_icon_png, BinaryData::imogen_icon_pngSize));

            setVisible (true);
        }

        void closeButtonPressed() final { JUCEApplication::getInstance()->systemRequestedQuit(); }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr< MainWindow > mainWindow;
};


//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (ImogenRemoteApplication)
