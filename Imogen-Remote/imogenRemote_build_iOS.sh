#!/bin/bash

#  imogenRemote_build_iOS.sh :	  This script builds an iOS version of ImogenRemote. When cross compiling for iOS, using MacOS is recommended, as is using CMake's XCode generator.


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

cd "$SCRIPT_DIR/.." # assume that the directory of the script is also the location of the local Imogen git clone

# first, make sure the local copy of the repo is up to date
printf "\n \t \v Checking for new commits to Imogen remote... \n \n"
git pull --recurse-submodules=yes

cd "$SCRIPT_DIR"

set -e;  # from this point forward, any errors trigger an exit signal


# configure CMake
printf "\n \t \v Configuring CMake... \n \n"
cmake -B Builds/ios_Build -GXcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=9.3 .


# execute build
printf "\n \t \v Building ImogenRemote... \n \n"
cmake --build Builds/ios_Build --target ImogenRemote --config Release -- -sdk iphonesimulator -allowProvisioningUpdates


printf "\n \t \v ImogenRemote built successfully!"
printf "\n \t \v Enjoy! \n"

exit 0;

###
