# Overview of source files 

Like almost all JUCE plugins, Imogen has two concurrent primary processing threads: the audio plugin editor, which controls the GUI elements (PluginEditor.h), and the audio plugin processor, which processes the audio at DSP sample rate (PluginProcessor.h). All subroutines and subcomponents of Imogen are hosted at the top-level by either PluginEditor.h or PluginProcessor.h.

Here is an overview of the various source files, and where they fit into the architecture of Imogen's code:

## hosted by the Editor
* **PluginEditor.h**
* **PluginEditor.cpp**

## hosted by the Processor
* **PluginProcessor.h**
* **PluginProcessor.cpp**
### MIDI processing
Imogen has no MIDI output; she only accepts MIDI input. The goal of Imogen's MIDI processing is to allow the user to dynamically assign and modulate expressive parameters for their *choir as a whole*, rather than needing to focus too exclusively on manipulating any individual harmony voice's settings. Imogen's controls have been specifically designed with live performance in mind; the user should be able to plug-and-play Imogen as a natural, organic instrument right out of the box without needing to set up any sophisticated settings or routings. In general, the inner workings (ie, *"voice #6 is playing pitch 42, and voice #9 is playing pitch 38"*) are hidden from the user; the player doesn't need to know *which* voices are on, just that *enough* voices are on, and that they're doing what they're supposed to. 
* **MidiProcessor.h :** MIDI wrapper class. Handles all realtime MIDI input; hosts & manages instances of the MIDI helper classes.
MIDI helper classes:
* **PolyphonyVoiceManager.h :** keeps track of which instances of HarmonyVoice are currently active, and what pitches are being played by the active voices. Essentially a wrapper class for an array of size NUMBER_OF_VOICES containing midiPitch values. -1 is an inactive voice.
* **VoiceStealingManager.h :** voice "stealing" is the concept that if all available instances of HarmonyVoice are already active with a previous note when a new note on comes in, the Harmonizer can intelligently choose which HarmonyVoice instance to assign the new note to -- "stealing" that voice from its old note. VoiceStealingManager keeps track of which voie # has been on the LONGEST, so that the stolen voice will always be the oldest pitch.
* **MidiPanningManager.h :** creates and stores a list of possible MIDIpanning values based on the user's selected "stereo width" parameter, and attempts to assign these values to newly activated voices from the "middle out".
* **MidiLatchManager.h :** allows MIDI latch to be turned off, by tracking any incoming note events recieved by the plugin while the MIDI LATCH option is ACTIVE : incoming note offs are collected into a list, so that if MIDI LATCH is turned off, the appropriate note offs can be sent to the Harmonizer and no notes will be "stuck on". As for incoming note ons: if the pitch was not already an active harmony pitch, it will be turned on without retriggering HarmonyVoices' ADSRs; if the pitch was previously on (ie, 'latched'), then that pitch will be REMOVED from LatchManager's held list of note offs to send upon MIDI LATCH being deactivated. The reason for this is because if the user re-presses a key while MIDI LATCH is still on, then releases MIDI LATCH but hasn't released that key, then of course that note should still be on. These notes are considered "retriggered".
### Input signal analysis
Nearly any pitch shifting algorithm of any kind will require some form of detection/estimation of the input signal's fundamental frequency. Knowing this value allows us to accurately resynthesize pitch grains of the input signal at our desired pitches; however, to increase the quality of the output, it is ideal if the center points in time of each analysis grain are the points of maximum energy of the input signal's fundamental frequency -- special points called *pitch marks*, *epochs*, or *peaks*. So Imogen needs to complete two major analysis tasks for each incoming signal vector: (1) estimate the input signal's fundamental frequency, and (2) identify these *pitch peaks* within the input signal. 

Luckily, both of these analysis tasks yield the same results regardless of our desired output shifted pitch -- the information we're concerned with in this step only has to do with the ***input*** frequency and pitch marks, which will be the same for all instances of the pitch shifting algorithm. Thus, for computational efficiency, I perform these two analysis steps only once on the input signal and feed their results into all 12 instances of the pitch shifting algorithm.
* **Yin.h :** Implements the YIN pitch tracking algorithm, using an FFT to compute the difference function.
* **EpochExtractor.h :** Detects the momentary "pitch marks" in the input signal -- the moments of maximum energy in the pitch periods of the input signal's fundamental frequency. My implementation passes the signal through two consecutive ZFRs and then detects zero crossings of the resulting signal
### The pitch shifting algorithm
As it is now, Imogen technically uses a time-domain ESOLA-based granular resynthesis process. Imogen takes the pitch epoch locations calculated in EpochExtractor.h and centers its analysis grains on these sample locations; in the resynthesis phase, the distance between the synthesis grains' pitch peaks is scaled to be consistent with the phase and periodicity of the input signal. 
* **HarmonyVoice.h :** wrapper class for the harmony algorithm. This class handles top-level functions, like MIDI note on/off, panning, an ADSR envelope, etc. The HarmonyVoice objects are technically hosted inside an OwnedArray<HarmonyVoice> within the PluginProcessor.
* **shifter.h :** this class houses the actual ESOLA algorithm (and handles the windowing function, which is a Hann).
### Utilities & miscellaneous
* **GlobalDeclarations.h** A helper file that just declares some global variables & parameters, for ease of includes.
