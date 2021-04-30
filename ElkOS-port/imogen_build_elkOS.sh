#!/bin/bash

#            _             _   _                _                _                 _               _
#           /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _               
#           \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
#           /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
#          / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
#         / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/       _____  ______ __  __  ____ _______ ______ 
#        / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /       |  __ \|  ____|  \/  |/ __ \__   __|  ____|
#       / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /        | |__) | |__  | \  / | |  | | | |  | |__   
#   ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /         |  _  /|  __| | |\/| | |  | | | |  |  __|  
#  /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /          | | \ \| |____| |  | | |__| | | |  | |____ 
#  \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/           |_|  \_\______|_|  |_|\____/  |_|  |______|
 
 
#  This file is part of the Imogen codebase.
 
#  @2021 by Ben Vining. All rights reserved.

#  imogenRemote_build_desktop.sh :	  This file provides a simple, cross-platform build configuration for ImogenRemote. Simply pull the git repo and run this script to build ImogenRemote in its default Release configuration for your platform; no other configuration is necessary and this script will automatically resolve all dependencies for you.


SCRIPT_DIR="$(dirname $0)"; # save the directory of the script


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
 	printf "\n \t \v IMOGEN REMOTE BUILD SCRIPT \n USAGE: \n \n"
  	printf "Simply execute this script with no flags or arguments to build a default release confguration of the ImogenRemote desktop app for your current platform. \n"
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

cd "$SCRIPT_DIR/.." 

# first, make sure the local copy of the repo is up to date
printf "\n \t \v Checking for new commits to Imogen remote... \n \n"
git pull --recurse-submodules=yes


set -e;  # from this point forward, any errors trigger an exit signal

cd "$SCRIPT_DIR"

# configure CMake
printf "\n \t \v Configuring CMake... \n \n"
cmake -B Builds/headless_plugin .


# execute build
printf "\n \t \v Building ImogenRemote... \n \n"
cmake --build Builds/headless_plugin --config Release


printf "\n \t \v ImogenRemote built successfully!"
printf "\n \t \v Enjoy! \n"

exit 0;

###

