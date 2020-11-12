# imogen
Ever since I was introduced to music like [Imogen Heap](http://www.youtube.com/watch?v=dHk2lLaDzlM), [Bon Iver](http://www.youtube.com/watch?v=CaYgMdq6NDg), and in particular [Jacob Collier's famous "Harmonizer"](http://www.youtube.com/watch?v=ZXIApugIuqk), I became obsessed with finding a way to replicate the sound of this incredible instrument, the vocal harmonizer.

Of course, vocoders are practically a dime a dozen these days (in fact, [here is a great free VST vocoder from TAL audio!](http://tal-software.com/products/tal-vocoder)); both hardware and software options are very affordable and provide good results -- for what a vocoder IS, which is an inherently more robotic replication of the voice at each output harmony pitch. But to actually achieve a sound as incredibly HUMAN as [Jacob Collier does](http://www.youtube.com/watch?v=m7_1HUEvieE) is another thing entirely, and quickly proved to be a tall order as soon as I started searching.

Commercial plugins offering this effect [do exist](http://www.izotope.com/en/products/nectar/features/harmony.html), but most are limited to four harmony voices, come with a hefty price tag, and are clunky & difficult to use in actual live performance. The closest thing to Jacob Collier's Harmonizer currently available on the market seems to be [Antares Harmony Engine](http://www.antarestech.com/product/harmony-engine/), which retails for $249 and only has four harmony voices. ([Here's](https://www.youtube.com/watch?v=4hgeVqTNVIw) what it sounds like.)

As far as I know, there is no piece of software currently commercially available that offers the features of Jacob Collier's Harmonizer: 16 real-time, incredibly human-sounding harmony voices. 

So I decided to [create it myself](http://www.youtube.com/watch?v=0lJxbwp_Sdg). 

The first version of my Imogen vocal harmonizer was created with [Max/MSP](http://cycling74.com/products/max). I successfully [implemented](http://www.youtube.com/watch?v=wRZxLcK6Ar4) 12 low-latency high quality PSOLA harmony voices; however, this implementation suffers from two major drawbacks: it runs only as a stand-alone application, and is not useable as a plugin inside a DAW, and it also only runs on Mac computers. 

The Max-built standalone app version saw some pretty great response (in fact, you can still download it [here](http://gumroad.com/benvining#PAkNo) if you like), which inspired me to continue my development journey and begin venturing into VST plugin coding: this led me to [JUCE](http://juce.com/).

I am currently working on implementing the Imogen vocal harmonizer in JUCE code; when this project is completed, Imogen will be a VST plugin useable on (hypothetically) any operating system. 
