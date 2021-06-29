
namespace Imogen
{

LogoButton::LogoButton()
{
    gui::addAndMakeVisible (this, button, aboutWindow);
}

void LogoButton::resized()
{
    // button, aboutWindow
}

void LogoButton::createAboutWindow()
{
    aboutWindow.create ([&]{ aboutWindow.destroy(); });
    resized();
}

}
