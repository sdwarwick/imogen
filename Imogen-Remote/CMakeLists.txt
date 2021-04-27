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

include (${Imogen_sourceDir}/imogenConfig.cmake)

set (ImogenRemote_sourceFiles
    ${ImogenCore_sourceFiles}
	${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.h
    ${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.cpp
    ${Imogen_sourceDir}/Remote/Main.cpp
    ${Imogen_sourceDir}/Remote/OSC_parser.h)

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
    ICON_BIG                    ${CMAKE_CURRENT_LIST_DIR}/../assets/imogen_icon.png
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
juce_add_binary_data (ImogenRemoteGraphicAssets SOURCES ${Imogen_assetFiles})
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
