#!/bin/bash

#  imogen_build_script.sh :	  This file provides a simple, cross-platform build configuration for Imogen. Simply pull the git repo and run this script to build Imogen in its default Release configuration for your platform; no other configuration is necessary and this script will automatically resolve all dependencies for you.


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

set -e;  # from this point forward, any errors trigger an exit signal


# configure CMake
printf "\n \t \v Configuring CMake... \n \n"
cmake -B Builds .


# execute build
printf "\n \t \v Building Imogen... \n \n"
cmake --build Builds --target Imogen_All --config Release


printf "\n \t \v Imogen built successfully!"
printf "\n \t \v Enjoy! \n"

exit 0;

###

