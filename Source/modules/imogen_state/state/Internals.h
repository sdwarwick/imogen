
#pragma once

namespace Imogen
{
struct Internals : ParameterList
{
    Internals();

    ToggleParam abletonLinkEnabled {"Ableton link Toggle", "Ableton link toggle", false};

    IntParam abletonLinkSessionPeers {"Link Num peers", "Ableton link num session peers", 0, 50, 0,
                                      [] (int value, int maximumStringLength)
                                      { return juce::String (value).substring (0, maximumStringLength); }};

    ToggleParam mtsEspIsConnected {"Is connected", "MTS-ESP is connected", false};

    StringProperty mtsEspScaleName {"Scale name"};

    IntParam lastMovedMidiController {"Controller Number", "Last moved MIDI controller number", 0, 127, 0};

    IntParam lastMovedCCValue {"Controller Value", "Last moved MIDI controller value", 0, 127, 0};

    BoolParam guiDarkMode {"Dark mode", "GUI Dark mode", true,
                           [] (bool val, int maxLength)
                           {
                               if (val) return TRANS ("Dark mode is on").substring (0, maxLength);

                               return TRANS ("Dark mode is off").substring (0, maxLength);
                           }};

    IntParam currentInputNote {"Current note", "Current input note", -1, 127, -1,
                               [] (int note, int maxLength)
                               {
                                   if (note == -1) return juce::String ("Unpitched");
                                   return pitchToString (note).substring (0, maxLength);
                               }};

    IntParam currentCentsSharp {"Cents sharp", "Current input cents sharp", -100, 100, 0,
                                [] (int cents, int maxLength)
                                {
                                    if (cents == 0) return TRANS ("Perfect!");

                                    if (cents > 0) return (juce::String (cents) + TRANS (" cents sharp")).substring (0, maxLength);

                                    return (juce::String (abs (cents)) + TRANS (" cents flat")).substring (0, maxLength);
                                },
                                nullptr,
                                TRANS ("cents")};

private:
    ParamUpdater linkPeersUpdater {abletonLinkEnabled, [&]
                                   {
                                       if (! abletonLinkEnabled->get())
                                           abletonLinkSessionPeers->set (0);
                                   }};

    ParamUpdater scaleNameUpdater {mtsEspIsConnected, [&]
                                   {
                                       if (mtsEspIsConnected->get())
                                       {
                                           mtsEspScaleName->setDefault ("UnnamedScale");
                                       }
                                       else
                                       {
                                           mtsEspScaleName->setDefault ("Not connected");
                                           mtsEspScaleName->set ("Not connected");
                                       }
                                   }};
};

}  // namespace Imogen


// auto scaleName = std::make_unique< StringNode > ("Scale name", "MTS-ESP scale name", "No active scale");
