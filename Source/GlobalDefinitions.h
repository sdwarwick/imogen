/*
  ==============================================================================

    GlobalDefinitions.h
    Created: 23 Nov 2020 9:56:38pm
    Author:  Ben Vining
 
 	This file just defines some global macros used across Imogen

  ==============================================================================
*/

#pragma once


#define MAX_POSSIBLE_NUMBER_OF_VOICES 12
// used to define some arbitrary global maximums for memory allocation, etc...


#define MAX_BUFFERSIZE 256
// an arbitrary maximum size, in samples, for the input audio buffer. If an audio vector is recieved from the host that is larger than this size, it will be sliced into a series of smaller vectors that are MAX_BUFFERSIZE or smaller.


#define FRAMERATE 60
// the framerate, in ms, of the graphics processing
