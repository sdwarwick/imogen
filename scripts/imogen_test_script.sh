#!/bin/bash

#            _             _   _                _                _                 _               _
#           /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
#           \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
#           /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
#          / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
#         / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
#        / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
#       / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
#   ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
#  /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
#  \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
#  This file is part of the Imogen codebase.
 
#  @2021 by Ben Vining. All rights reserved.

#  imogen_test_script.sh :	This script provides an automated way to run all tests on Imogen after the build has completed. Note that this script is not designed to be nearly as robust as the build script; this script will not automatically install pluginval, etc.


SCRIPT_DIR="$(dirname $0)"; # save the directory of the script
IMOGEN_DIR="$SCRIPT_DIR/..";


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

  #  to be added....



# STEINBERG VALIDATOR

  #  to be added....

