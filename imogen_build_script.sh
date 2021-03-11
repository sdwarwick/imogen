#!/bin/bash


# IMOGEN BUILD SCRIPT

# This shell script will build a default release configuration of Imogen. 
# Imogen's build options can be customized using CMake. This script is essentially a wrapper around Imogen's CMake scripts that selects the appropriate default options for you.

# When this script is run, if CMake cannot be found, it will be downloaded and installed.
# Running this script will create a directory "Builds" and execute the entire CMake configuration and build process. 
# The JUCE library code will be downloaded from GitHub into /imogen/Builds/_deps. If you are on Linux, this script will also use apt to download/update JUCE's Linux dependencies, as necessary.

# Before invoking this script, you should use the cd command to navigate to the directory containing the Imogen git repo (and, presumably, this script).
# An example invocation of this script looks like this:
# cd /Desktop/imogen
# bash imogen_build_script.sh

# After executing this script, the built Imogen executables will be located at /imogen/Builds/Imogen_artefacts/Release/.


IMOGEN_DIR="$(dirname $0)" # save the directory of the script


# set up execute permissions
chmod 755 "$IMOGEN_DIR"/imogen_build_script.sh


###  UTILITY FUNCTIONS  ###

# function to check if a requested command is valid
command_exists () {
	loc="$(type "$1" > /dev/null)" || [[ -z $loc ]] ;
}

# no cmake, and you're on Windows :(
windows_no_cmake () {
	printf "\n \t \v CMake cannot be found, and I've detected you're on Windows. \n"
	printf "\t Installing software from the command line on Windows is the realm of gods and demons. \n"
	printf "Please manually install CMake and re-run this script. \n"
	exit 1
}


###

# check to see if the script was invoked with the --help or -h flags
if [ ${#@} -ne 0 ] && ( [ "${@#"--help"}" = "" ] || [ "${@#"-h"}" = "" ] ); then
  printf "\n \t \v IMOGEN BUILD SCRIPT \n USAGE: \n \n Simply execute this script with no flags or arguments to build a default release confguration of Imogen in VST, AU and Standalone formats. \n"
  exit 0
fi


###  CMAKE INSTALLATION  ###

# if CMake can't be found, install it
if ! command_exists cmake ; then
	case "$OSTYPE" in
		darwin*) printf "\n \t \v Installing CMake... \n \n" && brew install cmake ;;  # MacOS
		msys*)   windows_no_cmake ;;
		*)       printf "\n \t \v Installing Cmake... \n \n" && sudo apt-get -y install cmake ;;  # Linux
	esac
fi


# if CMake still can't be found, there must be some error
if ! command_exists cmake ; then
	printf "\n \t \v CMake could not be found, and could not be automatically installed. \n Please manually install CMake and then re-run this script. \n"
	exit 1
fi


###  THE BUILD SCRIPT  ###

cd "$IMOGEN_DIR" # assume that the directory of the script is also the location of the local Imogen git clone

# first, make sure the local copy of the repo is up to date
printf "\n \t \v Checking for new commits to Imogen remote... \n \n"
git pull --recurse-submodules=yes


set -e;  # from this point forward, any errors trigger an exit signal


# configure CMake
printf "\n \t \v Configuring CMake... \n \n"
cmake -B Builds --config Release -DImogen_unitTesting=FALSE -Dbv_alwaysForceCacheInits=TRUE .


# execute build
printf "\n \t \v Building Imogen... \n \n"
cmake --build Builds --config Release


set +e;  # an error in zipping shouldn't be fatal


# zip artifacts 
printf "\n \t \v Zipping artifacts... \n \n"
cd Builds
cmake -E tar cfv Imogen_build_artefacts.zip --format=zip Imogen_artefacts


printf "\n \t \v Imogen built successfully!"
printf "\n \t \v Enjoy! \n"

exit 0

###

