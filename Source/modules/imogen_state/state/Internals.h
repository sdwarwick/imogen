
#pragma once

namespace Imogen
{
struct Internals
{
	void addToList (plugin::ParameterList& list);

	ToggleParam abletonLinkEnabled { "Ableton link toggle", false };

	IntParam abletonLinkSessionPeers { 0, 50, 0, "Ableton link num session peers",
									   [] (int value, int maximumStringLength)
									   { return juce::String (value).substring (0, maximumStringLength); } };

	ToggleParam mtsEspIsConnected { "MTS-ESP is connected", false };

	//    plugin::StringProperty mtsEspScaleName {"Scale name"};

	IntParam lastMovedMidiController { 0, 127, 0, "Last moved MIDI controller number" };

	IntParam lastMovedCCValue { 0, 127, 0, "Last moved MIDI controller value" };

	BoolParam guiDarkMode { true, "GUI Dark mode" };

	IntParam currentInputNote { -1, 127, -1, "Current input note",
								[] (int note, int maxLength)
								{
									if (note == -1) return juce::String ("Unpitched");
									return pitchToString (note).substring (0, maxLength);
								} };

	IntParam currentCentsSharp { -100, 100, 0, "Current input cents sharp",
								 [] (int cents, int maxLength)
								 {
									 if (cents == 0) return TRANS ("Perfect!");

									 if (cents > 0) return (juce::String (cents) + TRANS (" cents sharp")).substring (0, maxLength);

									 return (juce::String (abs (cents)) + TRANS (" cents flat")).substring (0, maxLength);
								 },
								 nullptr,
								 TRANS ("cents") };

private:

	plugin::ParamUpdater linkPeersUpdater { abletonLinkEnabled, [&]
											{
												if (! abletonLinkEnabled->get())
													abletonLinkSessionPeers->set (0);
											} };

	//    plugin::ParamUpdater scaleNameUpdater {mtsEspIsConnected, [&]
	//                                           {
	//                                               if (mtsEspIsConnected->get())
	//                                               {
	//                                                   mtsEspScaleName->setDefault ("UnnamedScale");
	//                                               }
	//                                               else
	//                                               {
	//                                                   mtsEspScaleName->setDefault ("Not connected");
	//                                                   mtsEspScaleName->set ("Not connected");
	//                                               }
	//                                           }};
};

}  // namespace Imogen


// auto scaleName = std::make_unique< StringNode > ("Scale name", "MTS-ESP scale name", "No active scale");
