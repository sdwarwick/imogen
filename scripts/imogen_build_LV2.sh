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

#  imogen_build_LV2.sh :	  This script builds an LV2 version of Imogen using the DISTRHO fork of JUCE.


SCRIPT_DIR="$(dirname $0)"; # save the directory of the script
IMOGEN_DIR="$SCRIPT_DIR/..";


###  UTILITY FUNCTIONS  ###

# function to check if a requested command is valid
command_exists () {
	loc="$(type "$1" > /dev/null)" || [[ -z "$loc" ]] ;
}

# no cmake, and you're on Windows :(
windows_no_cmake () {
	printf "\n \t \v CMake cannot be found, and I've detected you're on Windows. \n"
	printf "\t Installing software from the command line on Windows is the realm of gods and demons. \n"
	printf "Please manually install CMake and re-run this script. \n"
	exit 1
}

unknown_argument () {
	printf "\v Unknown argument '$1'. For usage, run this script with --help or -h. \n"
	exit 0;
}


###  CHECK FOR FLAGS & ARGUMENTS  ###

# check to see if the script was invoked with the --help or -h flags
if [[ ${#@} -ne 0 ]] && ( [[ ${@#"--help"} = "" ]] || [[ ${@#"-h"} = "" ]] ); then
 	printf "\n \t \v IMOGEN BUILD SCRIPT \n USAGE: \n \n"
  	printf "Simply execute this script with no flags or arguments to build a default release confguration of Imogen in VST, AU and Standalone formats. \n"
  	exit 0;
fi


for flag in "$@" ; do
	case "$flag" in
		*) unknown_argument "$flag" ;;
	esac
done


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
cmake -DCMAKE_BUILD_TYPE=release -B Builds/LV2_build -DImogen_unitTesting=FALSE -Dbv_alwaysForceCacheInits=TRUE -Dbv_formats=LV2 -Dbv_juceGitRepoToUse="https://github.com/lv2-porting-project/JUCE/tree/lv2.git" .


# execute build
printf "\n \t \v Building Imogen... \n \n"
cmake --build Builds/LV2_build --target Imogen_LV2 --config Release


printf "\n \t \v Imogen built successfully!"
printf "\n \t \v Enjoy! \n"

exit 0;

###
