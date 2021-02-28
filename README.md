<p align="center">
 <img src="https://github.com/benthevining/imogen/blob/master/imogen_icon.png" alt="Imogen icon" width="150" height="150" />
 </p>

# Imogen: a 12-voice, low-latency vocal harmonizer VST

[![AutoBuild](https://github.com/benthevining/imogen/actions/workflows/autoBuild.yml/badge.svg?branch=master&event=push)](https://github.com/benthevining/imogen/actions/workflows/autoBuild.yml)

[![CodeFactor](https://www.codefactor.io/repository/github/benthevining/imogen/badge)](https://www.codefactor.io/repository/github/benthevining/imogen)

Ever since I was introduced to music like [Imogen Heap](http://www.youtube.com/watch?v=dHk2lLaDzlM), [Bon Iver](http://www.youtube.com/watch?v=CaYgMdq6NDg), and in particular [Jacob Collier's famous "Harmonizer"](http://www.youtube.com/watch?v=ZXIApugIuqk), I became obsessed with finding a way to replicate the sound of this incredible instrument, the vocal harmonizer.

Commercial plugins offering this effect [do exist](http://www.izotope.com/en/products/nectar/features/harmony.html), but most are limited to four harmony voices, come with a hefty price tag, and are clunky & difficult to use in actual live performance. The closest thing to Jacob Collier's Harmonizer currently available on the market seems to be [Antares Harmony Engine](http://www.antarestech.com/product/harmony-engine/), which retails for $249 and only has four harmony voices. ([Here's](https://www.youtube.com/watch?v=4hgeVqTNVIw) what it sounds like.)

As far as I know, there is no piece of software currently commercially available that offers the features of Jacob Collier's Harmonizer: 16 real-time, incredibly human-sounding harmony voices. 

So I decided to create it myself.  

The first version of my vocal harmonizer was created with [Max/MSP](http://cycling74.com/products/max). I successfully [implemented](http://www.youtube.com/watch?v=wRZxLcK6Ar4) 12 low-latency high quality PSOLA harmony voices; however, this implementation suffers from two major drawbacks: it runs only as a stand-alone application, and is not useable as a plugin inside a DAW, and it also only runs on Mac computers. 

The Max-built standalone app version saw some pretty great response (in fact, you can still download it [here](http://gumroad.com/benvining#PAkNo) if you like), which inspired me to continue my development journey and begin venturing into VST plugin coding: this led me to [JUCE](http://juce.com/).

I am currently working on implementing the Imogen vocal harmonizer in JUCE code; when this project is completed, Imogen will be a VST plugin useable on (hypothetically) any operating system. 

## Author
**Ben Vining**: ben.the.vining@gmail.com | [Facebook](http://www.facebook.com/benviningofficial/) | [Twitter](http://twitter.com/benthevining) | [Gumroad](http://gumroad.com/benvining)
 
I'm also an electronic musician! Check out my music on [Soundcloud](http://soundcloud.com/benvining), [Bandcamp](http://benvining.bandcamp.com/releases), or [Spotify](http://open.spotify.com/artist/2UA73qR4E3nNPjjf8CphX8?si=RRm5taiETwi8L42-cHQwDw)

[![ko-fi](https://www.ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/G2G32OKV9)
