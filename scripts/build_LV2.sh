#!/bin/bash

if [[ "$OSTYPE" != "linux-gnu"* ]]; then
	printf ("LV2 is only available on Linux :(");
	exit 0;
fi

sudo apt-get install lv2-dev

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

bash $SCRIPT_DIR/install_juce_linux_deps.sh

if [[ $# > 0 ]]; then
	REPO_DIR=$1
else
	REPO_DIR=$SCRIPT_DIR/..
fi

cd $REPO_DIR/Builds

git clone -b lv2 https://github.com/lv2-porting-project/JUCE.git

cd JUCE

JUCE_DIR=$PWD
git pull

cd $REPO_DIR

cmake -B Builds -DFormats=LV2 -DCPM_JUCE_SOURCE=$JUCE_DIR

cmake --build Builds
