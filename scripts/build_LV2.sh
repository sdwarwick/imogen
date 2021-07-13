#!/bin/bash

sudo apt-get install lv2-dev

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

cd $SCRIPT_DIR/..

git clone -b lv2 https://github.com/lv2-porting-project/JUCE.git

cd JUCE

JUCE_DIR=$PWD

git pull

cd $SCRIPT_DIR/..

cmake -B Builds -DFormats=LV2 -DCPM_JUCE_SOURCE=$JUCE_DIR

cmake --build Builds --target Imogen_LV2
