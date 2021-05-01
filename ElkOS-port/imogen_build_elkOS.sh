#!/bin/bash


SCRIPT_DIR="$(dirname $0)"; # save the directory of the script


# function to check if a requested command is valid
command_exists () {
	loc="$(type "$1" > /dev/null)" || [[ -z "$loc" ]] ;
}

###  CMAKE INSTALLATION  ###

# if CMake can't be found, install it
if ! command_exists cmake ; then
	printf "\n \t \v Installing Cmake... \n \n" && sudo apt-get -y install cmake ;;  # Linux
fi


# if CMake still can't be found, there must be some error
if ! command_exists cmake ; then
	printf "\n \t \v CMake could not be found, and could not be automatically installed. \n Please manually install CMake and then re-run this script. \n"
	exit 1
fi


###  THE BUILD SCRIPT  ###

cd "$SCRIPT_DIR/.." 

set -e;  # from this point forward, any errors trigger an exit signal

# need to download the ElkOS cross-compiling toolchain from https://github.com/elk-audio/elkpi-sdk/releases/download/0.9.0/elk-glibc-x86_64-elkpi-audio-os-image-aarch64-raspberrypi4-64-toolchain-1.0.sh

# unset LD_LIBRARY_PATH
# source [path-to-extracted-sdk]/environment-setup-aarch64-elk-linux

cd "$SCRIPT_DIR"

# configure CMake
printf "\n \t \v Configuring CMake... \n \n"
cmake -B Builds/headless_plugin -GMakefile .


# execute build
printf "\n \t \v Building ImogenRemote... \n \n"
cmake --build Builds/headless_plugin --config Release


printf "\n \t \v ImogenRemote built successfully!"
printf "\n \t \v Enjoy! \n"

exit 0;

###

