# Imogen: a 12-voice, low-latency vocal harmonizer VST
Ever since I was introduced to music like [Imogen Heap](http://www.youtube.com/watch?v=dHk2lLaDzlM), [Bon Iver](http://www.youtube.com/watch?v=CaYgMdq6NDg), and in particular [Jacob Collier's famous "Harmonizer"](http://www.youtube.com/watch?v=ZXIApugIuqk), I became obsessed with finding a way to replicate the sound of this incredible instrument, the vocal harmonizer.

Of course, vocoders are practically a dime a dozen these days (in fact, [here is a great free VST vocoder from TAL audio!](http://tal-software.com/products/tal-vocoder)); both hardware and software options are very affordable and provide good results -- for what a vocoder IS, which is an inherently more robotic replication of the voice at each output harmony pitch. But to actually achieve a sound as incredibly HUMAN as [Jacob Collier does](http://www.youtube.com/watch?v=m7_1HUEvieE) is another thing entirely, and quickly proved to be a tall order as soon as I started searching.

Commercial plugins offering this effect [do exist](http://www.izotope.com/en/products/nectar/features/harmony.html), but most are limited to four harmony voices, come with a hefty price tag, and are clunky & difficult to use in actual live performance. The closest thing to Jacob Collier's Harmonizer currently available on the market seems to be [Antares Harmony Engine](http://www.antarestech.com/product/harmony-engine/), which retails for $249 and only has four harmony voices. ([Here's](https://www.youtube.com/watch?v=4hgeVqTNVIw) what it sounds like.)

As far as I know, there is no piece of software currently commercially available that offers the features of Jacob Collier's Harmonizer: 16 real-time, incredibly human-sounding harmony voices. 

So I decided to [create it myself](http://www.youtube.com/watch?v=0lJxbwp_Sdg). 

The first version of my Imogen vocal harmonizer was created with [Max/MSP](http://cycling74.com/products/max). I successfully [implemented](http://www.youtube.com/watch?v=wRZxLcK6Ar4) 12 low-latency high quality PSOLA harmony voices; however, this implementation suffers from two major drawbacks: it runs only as a stand-alone application, and is not useable as a plugin inside a DAW, and it also only runs on Mac computers. 

The Max-built standalone app version saw some pretty great response (in fact, you can still download it [here](http://gumroad.com/benvining#PAkNo) if you like), which inspired me to continue my development journey and begin venturing into VST plugin coding: this led me to [JUCE](http://juce.com/).

I am currently working on implementing the Imogen vocal harmonizer in JUCE code; when this project is completed, Imogen will be a VST plugin useable on (hypothetically) any operating system. 

[![ko-fi](https://www.ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/G2G32OKV9)

## Overview of source files 

Like almost all JUCE plugins, Imogen has two concurrent primary processing threads: the audio plugin editor, which controls the GUI elements (PluginEditor.h), and the audio plugin processor, which processes the audio at DSP sample rate (PluginProcessor.h). All subroutines and subcomponents of Imogen are hosted at the top-level by either PluginEditor.h or PluginProcessor.h.

Here is an overview of the various source files, and where they fit into the architecture of Imogen's code:

### hosted by the Editor

### hosted by the Processor
#### MIDI processing
Imogen has no MIDI output; she only accepts MIDI input. The goal of Imogen's MIDI processing is to allow the user to dynamically assign and modulate expressive parameters for their *choir as a whole*, rather than needing to focus too exclusively on manipulating any individual harmony voice's settings. Imogen's controls have been specifically designed with live performance in mind; the user should be able to plug-and-play Imogen as a natural, organic instrument right out of the box without needing to set up any sophisticated settings or routings. In general, the inner workings (ie, *"voice #6 is playing pitch 42, and voice #9 is playing pitch 38"*) are hidden from the user; the player doesn't need to know *which* voices are on, just that *enough* voices are on, and that they're doing what they're supposed to. 
* **MidiProcessor.h :** MIDI wrapper class. Handles all realtime MIDI input; hosts & manages instances of the MIDI helper classes.
MIDI helper classes:
* **PolyphonyVoiceManager.h :** keeps track of which instances of HarmonyVoice are currently active, and what pitches are being played by the active voices. Essentially a wrapper class for an array of size NUMBER_OF_VOICES containing midiPitch values. -1 is an inactive voice.
* **VoiceStealingManager.h :** voice "stealing" is the concept that if all available instances of HarmonyVoice are already active with a previous note when a new note on comes in, the Harmonizer can intelligently choose which HarmonyVoice instance to assign the new note to -- "stealing" that voice from its old note. VoiceStealingManager keeps track of which voie # has been on the LONGEST, so that the stolen voice will always be the oldest pitch.
* **MidiPanningManager.h :** creates and stores a list of possible MIDIpanning values based on the user's selected "stereo width" parameter, and attempts to assign these values to newly activated voices from the "middle out".
* **MidiLatchManager.h :** allows MIDI latch to be turned off, by tracking any incoming note events recieved by the plugin while the MIDI LATCH option is ACTIVE : incoming note offs are collected into a list, so that if MIDI LATCH is turned off, the appropriate note offs can be sent to the Harmonizer and no notes will be "stuck on". As for incoming note ons: if the pitch was not already an active harmony pitch, it will be turned on without retriggering HarmonyVoices' ADSRs; if the pitch was previously on (ie, 'latched'), then that pitch will be REMOVED from LatchManager's held list of note offs to send upon MIDI LATCH being deactivated. The reason for this is because if the user re-presses a key while MIDI LATCH is still on, then releases MIDI LATCH but hasn't released that key, then of course that note should still be on. These notes are considered "retriggered".
