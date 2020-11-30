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

### Author
**Ben Vining**: ben.the.vining@gmail.com | [Facebook](http://www.facebook.com/benviningofficial/) | [Twitter](http://twitter.com/benthevining) | [Gumroad](http://gumroad.com/benvining)

I'm also an electronic musician! Check out my music on [Soundcloud](http://soundcloud.com/benvining), [Bandcamp](http://benvining.bandcamp.com/releases), or [Spotify](http://open.spotify.com/artist/2UA73qR4E3nNPjjf8CphX8?si=RRm5taiETwi8L42-cHQwDw)
  
## Works cited & resources 

I created Imogen (and learned how to code while doing it) by gathering a wealth of resources, tutorials, examples, and research papers. Some specific items warrant explicit recognition:

### Tutorials & video series 
* [Free C++ course for beginners](http://www.youtube.com/playlist?list=PLmpc3xvYSk4wDCP5zjt2QQXe8-JGHa4Kt) by [caveofprogramming.com](http://www.caveofprogramming.com) (John Purcell)
* [The Audio Programmer YouTube channel:](http://www.youtube.com/channel/UCpKb02FsH4WH4X_2xhIoJ1A) a wealth of knowledge & some wonderful introductory tutorials on JUCE

### Research papers & scholarly articles 
* ["YIN, a fundamental frequency estimator for speech and music", by Alain de Cheveigne´ & Hideki Kawahara, 2002](http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf)
* ["Pitch-Synchronous Waveform Processing Techniques for Text-to-Speech Synthesis Using Diphones", by Eric Moulines & Francis Charpentier, 1990](http://courses.engr.illinois.edu/ece420/sp2017/PSOLA.pdf)
* ["Epoch-Synchronous Overlap-Add (ESOLA) for Time- and Pitch-Scale Modification of Speech Signals", by Sunil Rudresh et al, 2018](http://arxiv.org/pdf/1801.06492.pdf)
* ["Epoch Extraction From Speech Signals", by K. Sri Rama Murty & B. Yegnanarayana, 2008](http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=6D94C490DA889017DE4362D322E1A23C?doi=10.1.1.586.7214&rep=rep1&type=pdf)
* ["Time-Scale Modification Using Fuzzy Epoch-Synchronous Overlap-Add (FESOLA)", by Timothy Roberts & Kuldip K. Paliwal, 2019](http://maxwell.ict.griffith.edu.au/spl/publications/papers/iwaspaa19_roberts.pdf)
* ["Harmonic-Plus-Noise Model for Concatenative Speech Synthesis", by D. Vandromme, 2005](http://infoscience.epfl.ch/record/83295/files/vandromme_2005.pdf)

### Example & open-source code 
* [TD-PSOLA implementation in C++ by Terry Kong](http://www.github.com/terrykong/Phase-Vocoder/blob/master/PSOLA/PSOLA.cpp)
* [PSOLA in Python by Sanna Wager](http://www.github.com/sannawag/TD-PSOLA/blob/master/td_psola.py)
* [ESOLA in C++ by Arjun Variar](http://www.github.com/viig99/esolafast/blob/master/src/esola.cpp)
* [ESOLA in Python by BaronVladziu](http://www.github.com/BaronVladziu/ESOLA-Implementation/blob/master/ESOLA.py)
* [FESOLA in MATLAB by Tim Roberts](http://www.github.com/zygurt/TSM/blob/master/Batch/FESOLA_batch.m)
* [Implementation of the YIN pitch-tracking algorithm in Java by Joren Six, Matthias Mauch & Paul Brossier](http://github.com/JorenSix/TarsosDSP/blob/master/src/core/be/tarsos/dsp/pitch/FastYin.java)
* [YIN in Python by Patrice Guyot](http://www.github.com/patriceguyot/Yin/blob/master/yin.py)
* [PYIN ("probabilistic YIN") in C++, by Matthias Mauch et al](http://code.soundsoftware.ac.uk/projects/pyin)
* [Dynamic pitch-tracking library written in C by Antoine Schmitt](http://www.github.com/antoineschmitt/dywapitchtrack)
