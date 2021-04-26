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

#  CMakeLists.txt :	This file defines the build configuration for the Imogen Remote standalone app.
 

cmake_minimum_required (VERSION 3.18) 

project (ImogenRemote VERSION 0.0.1 LANGUAGES CXX) 

#

message (STATUS "Welcome! I'm going to generate the Imogen Remote IDE project for you.")

#

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/../Source)

set (guiSourcePath ${Imogen_sourceDir}/GUI)

set (Imogen_graphicAssetsDir ${CMAKE_CURRENT_LIST_DIR}/../assets)  # The location of the graphical asset files (images, etc)

#

set (ImogenRemote_sourceFiles
	${Imogen_sourceDir}/ImogenCommon.h
    ${guiSourcePath}/ImogenGUI.h
    ${guiSourcePath}/ImogenGUI.cpp
    ${guiSourcePath}/Holders/ImogenGuiHolder.h
	${guiSourcePath}/Holders/Remote_MainComponent/MainComponent.h
    ${guiSourcePath}/Holders/Remote_MainComponent/MainComponent.cpp
    ${guiSourcePath}/MainDialComponent/MainDialComponent.h
    ${guiSourcePath}/MainDialComponent/MainDialComponent.cpp
    ${guiSourcePath}/LookAndFeel/ImogenLookAndFeel.h
    ${guiSourcePath}/LookAndFeel/ImogenLookAndFeel.cpp
    ${Imogen_sourceDir}/Remote/Main.cpp
    ${Imogen_sourceDir}/Remote/OSC_parser.h)

set (ImogenRemote_graphicAssetFiles
    ${Imogen_graphicAssetsDir}/imogen_icon.png)

#

if (UNIX AND NOT (APPLE OR WIN32))  # install JUCE's Linux dependencies 

    message (STATUS "Updating JUCE's Linux dependencies...")

    execute_process (COMMAND "sudo" "apt-get" "update" OUTPUT_QUIET)

    foreach (pkg libasound2-dev libcurl4-openssl-dev libx11-dev libxinerama-dev libxext-dev libfreetype6-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev)
        execute_process (COMMAND "sudo" "apt" "install" "${pkg}" OUTPUT_QUIET)
    endforeach()
endif()

#

if (ANDROID)
    add_definitions (
    "-DJUCE_ANDROID=1" 
    "-DJUCE_PUSH_NOTIFICATIONS=1" 
    "-DJUCE_PUSH_NOTIFICATIONS_ACTIVITY=\"com/rmsl/juce/JuceActivity\"" 
    )
endif()

#

set_property (GLOBAL PROPERTY USE_FOLDERS YES)
set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Build Targets")

set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "")  # universal macOS binaries 
set (CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum OS X deployment version" FORCE)  # minimum MacOS version to build for

set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # static linking on Windows
set (CMAKE_WIN32_EXECUTABLE TRUE)

set (CMAKE_XCODE_GENERATE_SCHEME OFF)  # schemes are maually generated for each target to avoid clutter from modules getting schemes, etc

set (CMAKE_SUPPRESS_REGENERATION TRUE)  # no "zero-check" target
set (CMAKE_OPTIMIZE_DEPENDENCIES TRUE)
set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option (JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)
option (JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)

#

# ADD JUCE #

if (DEFINED bv_juceDir)
	add_subdirectory (${bv_juceDir} ${CMAKE_CURRENT_SOURCE_DIR}/Builds/JUCE)
else()
	if (NOT DEFINED bv_juceGitRepoToUse)
        set (bv_juceGitRepoToUse https://github.com/juce-framework/JUCE.git)
	endif()

	if (NOT DEFINED bv_juceGitRepo_TagToUse)
        if ("${bv_juceGitRepoToUse}" STREQUAL "https://github.com/juce-framework/JUCE.git")
	        set (bv_juceGitRepo_TagToUse origin/develop)
		else()
	        set (bv_juceGitRepo_TagToUse origin)
		endif()
	endif()

	message (STATUS "Fetching the latest version of JUCE from GitHub repo: ${bv_juceGitRepoToUse} with tag: ${bv_juceGitRepo_TagToUse} ")

	include (FetchContent)

	FetchContent_Declare (juce
	GIT_REPOSITORY ${bv_juceGitRepoToUse}
	GIT_TAG        ${bv_juceGitRepo_TagToUse})

	FetchContent_MakeAvailable (juce)

	set (bv_juceDir ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/juce-src)
endif()

#

if (ANDROID)
    set (OBOE_DIR "../JUCE/modules/juce_audio_devices/native/oboe")
    add_subdirectory (${OBOE_DIR} ./oboe)

    add_library ("cpufeatures" STATIC "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
    set_source_files_properties ("${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c" PROPERTIES COMPILE_FLAGS "-Wno-sign-conversion -Wno-gnu-statement-expression")

    enable_language (ASM)
endif()

#

juce_add_module ("${Imogen_sourceDir}/Shared-code/bv_SharedCode")

#

juce_add_gui_app (ImogenRemote
    PRODUCT_NAME                ImogenRemote
    VERSION                     ${CMAKE_PROJECT_VERSION}
    BUNDLE_ID                   com.BenViningMusicSoftware.ImogenRemote
    DESCRIPTION                 "Remote control for the Imogen plugin"
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    ICON_BIG                    ${Imogen_graphicAssetsDir}/imogen_icon.png
    COMPANY_NAME                BenViningMusicSoftware
    COMPANY_WEBSITE             www.benvining.com
    COMPANY_EMAIL               ben.the.vining@gmail.com
    COMPANY_COPYRIGHT           "This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License."
    MICROPHONE_PERMISSION_ENABLED        FALSE
    SEND_APPLE_EVENTS_PERMISSION_ENABLED FALSE
    DOCUMENT_BROWSER_ENABLED             TRUE
    STATUS_BAR_HIDDEN                    TRUE  # for iOS
    REQUIRES_FULL_SCREEN                 TRUE  # for iOS
    IPAD_SCREEN_ORIENTATIONS             UIInterfaceOrientationUnknown, UIInterfaceOrientationLandscapeLeft, UIInterfaceOrientationLandscapeRight
    TARGETED_DEVICE_FAMILY               2     # target iPad only
    )

#

target_sources (ImogenRemote PRIVATE ${ImogenRemote_sourceFiles})

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR}/.. PREFIX "" FILES ${ImogenRemote_sourceFiles})

target_include_directories (ImogenRemote PRIVATE ${Imogen_sourceDir})

target_link_libraries (ImogenRemote PRIVATE bv_SharedCode)

set_target_properties (ImogenRemote PROPERTIES FOLDER "" XCODE_GENERATE_SCHEME ON)  # prevets IDE from generating an extra nested folder

#

file (REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/Builds/juce_binarydata_ImogenRemoteGraphicAssets)
juce_add_binary_data (ImogenRemoteGraphicAssets SOURCES ${ImogenRemote_graphicAssetFiles})
set_target_properties (ImogenRemoteGraphicAssets PROPERTIES FOLDER "ImogenRemote")

target_link_libraries (ImogenRemote PRIVATE ImogenRemoteGraphicAssets)

#

if (ANDROID)
    find_library (log "log")
    find_library (android "android")
    find_library (glesv2 "GLESv2")
    find_library (egl "EGL")
    set (cpufeatures_lib "cpufeatures")
    set (oboe_lib "oboe")

    target_include_directories (Imogen PRIVATE
        "${ANDROID_NDK}/sources/android/cpufeatures"
        "${OBOE_DIR}/include")

    target_link_libraries (ImogenRemote PUBLIC ${log} ${android} ${glesv2} ${egl} ${cpufeatures_lib} ${oboe_lib})
endif()

#

if (DEFINED BV_IGNORE_VDSP OR NOT APPLE)

    target_compile_definitions (ImogenRemote PUBLIC JUCE_USE_VDSP_FRAMEWORK=0 BV_USE_VDSP=0)
    
    if (DEFINED BV_IGNORE_MIPP)
        target_compile_definitions (ImogenRemote PUBLIC BV_USE_MIPP=0)
    else()
        message (STATUS "Configuring MIPP...")
		target_compile_definitions (ImogenRemote PUBLIC MIPP_ENABLE_BACKTRACE BV_USE_MIPP=1)
    	target_include_directories (ImogenRemote PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/MIPP/src" "MIPP")
    endif()
else()
    target_compile_definitions (ImogenRemote PUBLIC BV_USE_VDSP=1 JUCE_USE_VDSP_FRAMEWORK=1)
endif()

#

target_link_libraries (ImogenRemote PUBLIC
	    juce::juce_recommended_config_flags
	    juce::juce_recommended_lto_flags
	    juce::juce_recommended_warning_flags
)

target_compile_features (ImogenRemote PUBLIC cxx_std_17)

target_compile_definitions (ImogenRemote PUBLIC 
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_MODAL_LOOPS_PERMITTED=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    IMOGEN_REMOTE_APP=1
    )

#
