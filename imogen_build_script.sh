#!/bin/bash


# IMOGEN BUILD SCRIPT

# This shell script will build a default release configuration of Imogen. 
# Imogen can also be built with CMake, using the CMakeLists.txt in the Imogen repo. This script is essentially a wrapper around Imogen's CMake scripts that selects the appropriate default options for you.

# Running this script for the first time will create a directory "Builds" and execute the entire CMake configuration and build process. 
# If CMake can't be found, this script will download it.
# The JUCE library code will be downloaded from GitHub into /imogen/Builds/_deps. If you are on Linux, this script will also use apt to download/update JUCE's Linux dependencies, as necessary.

# Before invoking this script, you should use the cd command to navigate to the directory containing the Imogen git repo (and, presumably, this script).
# An example invocation of this script look like this:
# cd /Desktop/imogen
# bash imogen_build_script.sh


# set up execute permissions
chmod 755 ${PWD}/imogen_build_script.sh


###

# simple function to check if a requested command is valid
command_exists () {
    type "$1" &> /dev/null ;
}


# installs cmake (for linux use only)
linux_get_cmake() {
	sudo apt-get -y install cmake
}


# installs cmake (for MaxOSX use only)
mac_get_cmake() {
	brew install cmake
}


# installs cmake (for windows use only)
windows_get_cmake () {
	echo -e "\t \v CMake cannot be found, and I've detected you're on Windows. \n"
	echo -e "\t \v Installing software from the command line on Windows is the realm of gods and demons. \n"
	echo -e "Please manually install CMake and re-run this script."
}

###


# if CMake can't be found, install it
if ! command_exists cmake ; then
	case "$OSTYPE" in
	  darwin*)	mac_get_cmake ;;
	  msys*)	windows_get_cmake ;;
	  *)		linux_get_cmake ;;
	esac
fi


# if CMake still can't be found, there must be some error
if ! command_exists cmake ; then
	echo -e "\t \v CMake could not be found, and could not be automatically installed. \n Please manually install CMake and then re-run this script."
	exit 1
fi


# configure CMake
echo -e "\t \v Configuring CMake..."
cmake -B Builds --config Release -DCMAKE_BUILD_TYPE=Release -DImogen_unitTesting=FALSE -Dbv_alwaysForceCacheInits=TRUE .


# execute build
echo -e "\t \v Building Imogen..."
cmake --build Builds --config Release


echo -e "\t \v Imogen built successfully \n Enjoy!"

