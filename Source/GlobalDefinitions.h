/*
  ==============================================================================

    GlobalDefinitions.h
    Created: 23 Nov 2020 9:56:38pm
    Author:  Ben Vining
 
 	This file just defines some global variables. I put these all in 1 file for convenience of includes.

  ==============================================================================
*/

#pragma once


// the number of instances of the harmony algorithm running concurrently
#define NUMBER_OF_VOICES 12


// an arbitrary maximum size, in samples, for the input audio buffer. If an audio vector is recieved from the host that is larger than this size, it will be sliced into a series of smaller vectors that are MAX_BUFFERSIZE or smaller.
#define MAX_BUFFERSIZE 1024


// the framerate, in ms, of the graphics processing
#define FRAMERATE 60


#define NUMBER_OF_CHANNELS 2
// as of now, Imogen is set up to output a stereo signal to channels 0 and 1.

