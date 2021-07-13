#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

cd $SCRIPT_DIR/..

mkdir JUCE

cd JUCE

JUCE_DIR=$PWD

git clone -b lv2 https://github.com/lv2-porting-project/JUCE.git
git pull

cd $SCRIPT_DIR/..

cmake -B Builds -DFormats=LV2 -DCPM_JUCE_SOURCE=$JUCE_DIR

cmake --build Builds --target Imogen_LV2
