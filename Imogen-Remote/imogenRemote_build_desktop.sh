#!/bin/bash

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
cmake -B Builds .


# execute build
printf "\n \t \v Building ImogenRemote... \n \n"
cmake --build Builds --target ImogenRemote --config Release


printf "\n \t \v ImogenRemote built successfully!"
printf "\n \t \v Enjoy! \n"

exit 0;

###

