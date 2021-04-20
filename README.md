# Imogen: the ultimate real-time vocal harmonizer

[![AutoBuild](https://github.com/benthevining/imogen/actions/workflows/autoBuild.yml/badge.svg?branch=master)](https://github.com/benthevining/imogen/actions/workflows/autoBuild.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/benthevining/imogen/badge)](https://www.codefactor.io/repository/github/benthevining/imogen)

Ever since I was introduced to music like [Imogen Heap](http://www.youtube.com/watch?v=dHk2lLaDzlM), [Bon Iver](http://www.youtube.com/watch?v=CaYgMdq6NDg), and in particular [Jacob Collier's famous "Harmonizer"](http://www.youtube.com/watch?v=ZXIApugIuqk), I became obsessed with finding a way to replicate the sound of this incredible instrument, the vocal harmonizer.
 
Commercial plugins offering this effect [do exist](http://www.izotope.com/en/products/nectar/features/harmony.html), but most are limited to four harmony voices, come with a hefty price tag, and are clunky & difficult to use in actual live performance. The closest thing to Jacob Collier's Harmonizer currently available on the market seems to be [Antares Harmony Engine](http://www.antarestech.com/product/harmony-engine/), which retails for $249 and only has four harmony voices. ([Here's](https://www.youtube.com/watch?v=4hgeVqTNVIw) what it sounds like.)

As far as I know, there is no piece of software currently commercially available that offers the features of Jacob Collier's Harmonizer: 16 real-time, incredibly human-sounding harmony voices. 

So I decided to create it myself.

Imogen is a low-latency pitch shifter designed to function as an instrument that is dynamic to play, and as a lead vocals mixing workstation: Imogen also includes pitch correction for the lead vocals, as well as a suite of built-in mixing effects. You can adjust the number of harmony voices running inside Imogen -- you can use only one for light CPU and guarunteed monophony, or you can add up to 12 (although theoretically infinite) polyphony voices.

Imogen features integrations with Ableton Link and MTS-ESP. On non-Apple platforms, the MIPP library is used for portable SIMD intrinsics.

<p align="center">
 <img src="https://github.com/benthevining/imogen/blob/master/assets/imogen_icon.png" alt="Imogen icon" width="150" height="150" />
 </p>
 
 
NOTE: Imogen is currently under development and will mostly likely not function as intended if you download it and try to build it, though you are free to do so. Imogen's official release is upcoming and will be announced.

## Author
**Ben Vining**: ben.the.vining@gmail.com | [Facebook](http://www.facebook.com/benviningofficial/) | [Twitter](http://twitter.com/benthevining) | [Gumroad](http://gumroad.com/benvining)
 
I'm also an electronic musician! Check out my music on [Soundcloud](http://soundcloud.com/benvining), [Bandcamp](http://benvining.bandcamp.com/releases), or [Spotify](http://open.spotify.com/artist/2UA73qR4E3nNPjjf8CphX8?si=RRm5taiETwi8L42-cHQwDw)

[![ko-fi](https://www.ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/G2G32OKV9)
