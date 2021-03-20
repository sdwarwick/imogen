#!/bin/bash

# IMOGEN TESTING SCRIPT

# This script can automatically run various tests on the built Imogen executables. This script must be run after the build is completed. 
# Note that this script is not meant to be as robust as the build script or the included CMake modules. This script must be edited to provide the path to various tester apps on your system, and does not attempt to install them if they cannot be found. Imogen's CMake scripts can automatically download pluginval for you, so use CMake to configure your build system if you want that functionality.


IMOGEN_DIR="$(dirname $0)"; # save the directory of the script


CONFIG_TO_TEST="Debug"; # the build configuration to test

FORMATS_TO_TEST="VST3 AU"; # formats to test, separated by whitespace 


ARTEFACTS_DIR="$IMOGEN_DIR/Builds/Imogen_artefacts/$CONFIG_TO_TEST";



# PLUGINVAL

PLUGINVAL_PATH="/Users/benvining/Documents/pluginval/Builds/MacOSX/build/Debug/pluginval.app/Contents/MacOS/pluginval"; # the path to the pluginval executable

PLUGINVAL_INTENSITY_LEVEL="10";

printf "\n \v \t Starting pluginval tests... \n"

printf "\n \v Testing VST3 version... \n"

"$PLUGINVAL_PATH" --strictness-level "$PLUGINVAL_INTENSITY_LEVEL" --validate "$ARTEFACTS_DIR/VST3/Imogen.vst3"

printf "\n \v Testing AU version... \n"

"$PLUGINVAL_PATH" --strictness-level "$PLUGINVAL_INTENSITY_LEVEL" --validate "$ARTEFACTS_DIR/AU/Imogen.component"


# AUVAL


# STEINBERG VALIDATOR

