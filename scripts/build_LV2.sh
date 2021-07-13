#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

cd $SCRIPT_DIR/..

mkdir Builds

cd Builds

mkdir JUCE

cd JUCE

git clone https://github.com/lv2-porting-project/JUCE.git

cd $SCRIPT_DIR/..

cmake -B Builds -DFormats=LV2 -DCPM_JUCE_SOURCE=$SCRIPT_DIR/Builds/JUCE

cmake --build Builds --target Imogen_LV2