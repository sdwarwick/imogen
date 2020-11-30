/*
  ==============================================================================

    GlobalDefinitions.h
    Created: 23 Nov 2020 9:56:38pm
    Author:  Ben Vining
 
 	This file just defines some global variables. I put these all in 1 file for convenience of includes.

  ==============================================================================
*/

#pragma once


#define NUMBER_OF_VOICES 12
// the number of instances of the harmony algorithm running concurrently


#define MAX_BUFFERSIZE 1024
// an arbitrary maximum size, in samples, for the input audio buffer. If an audio vector is recieved from the host that is larger than this size, it will be sliced into a series of smaller vectors that are MAX_BUFFERSIZE or smaller.


#define FRAMERATE 60
// the framerate, in ms, of the graphics processing


#define NUMBER_OF_CHANNELS 2
// as of now, Imogen is set up to output a stereo signal to channels 0 and 1.


#define CONCERT_PITCH_HZ 440.0f
// the pitch, in Hz, that Midi note 69 (the A above middle C) will be tuned to. Change this to essentially detune the entire Harmonizer.

