#!/bin/bash

# IMOGEN BUILD SCRIPT

# This shell script will build a default release configuration of Imogen. 
# Imogen can also be built with CMake, using the CMakeLists.txt in the Imogen repo. This script is essentially a wrapper around Imogen's CMake scripts that selects the appropriate default options for you.

# Running this script for the first time will create a directory "Builds" and execute the entire CMake configuration and build process. The JUCE library code will be downloaded from GitHub into /Builds/_deps. If you are on Linux, this script will also use apt to download/update JUCE's Linux dependencies.

# Before invoking this script, you should use the cd command to navigate to the directory containing the Imogen git repo (and, presumably, this script).
# An example invocation of this script look like this:
# cd /Desktop/imogen
# bash imogen_build_script.sh

chmod 755 ${PWD}/imogen_build_script.sh

# configure CMake
cmake -B Builds --config Release -DCMAKE_BUILD_TYPE=Release -DImogen_unitTesting=FALSE -Dbv_alwaysForceCacheInits=TRUE .

# execute build
cmake --build Builds --config Release

