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

#  CMakeLists.txt :	This file defines the build configuration for Imogen.
 

cmake_minimum_required (VERSION 3.18) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX) 

#

message (STATUS "Welcome! I'm going to generate the Imogen IDE project for you.")

#

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)  # This could conceivably be in a different place, but the source tree must be intact 

#

include (${Imogen_sourceDir}/imogenConfig.cmake)

#

set (Imogen_sourceFiles
    ${ImogenCore_sourceFiles}
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessor.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessorParameters.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessorState.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessorNetworking.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessor.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.cpp
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_StandaloneWrapper/StandaloneWrapper.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_StandaloneWrapper/StandaloneWrapper.cpp)
#

if (DEFINED bv_formats)
    set (lv2detector -1)
    list (FIND bv_formats LV2 lv2detector)

    if (NOT ${lv2detector} EQUAL -1)
        set_source_files_properties (${bv_juceDir}/extras/Build/lv2_ttl_generator/lv2_ttl_generator.c PROPERTIES LANGUAGE CXX)
    endif()
else()
    set (bv_formats AU VST3 Standalone Unity)
endif()

#

set (thingsToLinkTo "")

juce_add_module ("${Imogen_sourceDir}/DSP_modules/bv_Harmonizer")
list (APPEND thingsToLinkTo bv_Harmonizer)

juce_add_module ("${Imogen_sourceDir}/DSP_modules/bv_ImogenEngine")
list (APPEND thingsToLinkTo bv_ImogenEngine)

juce_add_module ("${Imogen_sourceDir}/Shared-code/bv_SharedCode")
list (APPEND thingsToLinkTo "bv_SharedCode")

juce_add_module ("${Imogen_sourceDir}/DSP_modules/SynthBase/bv_SynthBase")
list (APPEND thingsToLinkTo "bv_SynthBase")

#

juce_add_plugin (Imogen
    PRODUCT_NAME                Imogen
    PLUGIN_CODE                 Imgn
    VERSION                     ${CMAKE_PROJECT_VERSION}
    BUNDLE_ID                   com.BenViningMusicSoftware.Imogen
    PLUGIN_NAME                 Imogen
    PLUGIN_MANUFACTURER_CODE    Benv
    DESCRIPTION                 "Real-time vocal harmonizer instrument"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    FORMATS                     ${bv_formats}
    VST3_CATEGORIES             "Pitch Shift"
    AU_MAIN_TYPE                "kAudioUnitType_MusicEffect"
    ICON_BIG                    ${CMAKE_CURRENT_LIST_DIR}/assets/imogen_icon.png
    COMPANY_NAME                BenViningMusicSoftware
    COMPANY_WEBSITE             www.benvining.com
    COMPANY_EMAIL               ben.the.vining@gmail.com
    COPY_PLUGIN_AFTER_BUILD     TRUE
    UNITY_COPY_DIR              ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Imogen_artefacts  # unlike the other copy locations, Unity doesn't have a default copy location, so if you select Unity as a format and enable copy plugin on build, you must provide a Unity copy dir here
    COMPANY_COPYRIGHT           "This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License."
    MICROPHONE_PERMISSION_ENABLED        TRUE
    MICROPHONE_PERMISSION_TEXT           "Imogen requires audio input to be able to produce its output. Please enable the microphone, or you won't hear anything when you press the keys."
    SEND_APPLE_EVENTS_PERMISSION_ENABLED FALSE
    DOCUMENT_BROWSER_ENABLED             TRUE
    STATUS_BAR_HIDDEN                    TRUE  # for iOS
    REQUIRES_FULL_SCREEN                 TRUE  # for iOS
    IPAD_SCREEN_ORIENTATIONS             UIInterfaceOrientationUnknown, UIInterfaceOrientationLandscapeLeft, UIInterfaceOrientationLandscapeRight
    TARGETED_DEVICE_FAMILY               2     # target iPad only
    VST_NUM_MIDI_INS 1
    VST_NUM_MIDI_OUTS 1
    )

#

if (TARGET Imogen_LV2)
    set_target_properties (Imogen PROPERTIES JUCE_LV2_URI https://github.com/benthevining/imogen)
endif()

#

target_sources (Imogen PRIVATE ${Imogen_sourceFiles})

source_group (TREE ${Imogen_sourceDir} PREFIX "" FILES ${Imogen_sourceFiles})

target_include_directories (Imogen PRIVATE ${Imogen_sourceDir})

set_target_properties (Imogen PROPERTIES FOLDER "")  # prevets IDE from generating an extra nested folder

#

list (APPEND bv_formats "All")

foreach (target ${bv_formats}) 
    set (thisTargetName "Imogen_${target}")  # this is how JUCE automatically names the build targets created for each format

    if (NOT TARGET ${thisTargetName})
        continue()
    endif()
	
	message (STATUS "Configuring ${thisTargetName}...")
    set_target_properties (${thisTargetName} PROPERTIES FOLDER "Build Targets" XCODE_GENERATE_SCHEME ON)
endforeach()

#

file (REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/Builds/juce_binarydata_ImogenGraphicAssets)
juce_add_binary_data (ImogenGraphicAssets SOURCES ${Imogen_assetFiles})
set_target_properties (ImogenGraphicAssets PROPERTIES FOLDER "Imogen")
list (APPEND thingsToLinkTo ImogenGraphicAssets)

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

    target_link_libraries (Imogen PUBLIC ${log} ${android} ${glesv2} ${egl} ${cpufeatures_lib} ${oboe_lib})
endif()

#

if (NOT "${thingsToLinkTo}" STREQUAL "")
	target_link_libraries (Imogen PRIVATE ${thingsToLinkTo})
endif()

#

include (${CMAKE_CURRENT_LIST_DIR}/third-party/ableton-link/AbletonLinkConfig.cmake)
target_link_libraries (Imogen PRIVATE Ableton::Link)

#

target_include_directories (Imogen PUBLIC "third-party/MTS-ESP/Client" "MTS-ESP")
target_compile_definitions (Imogen PUBLIC bvsb_USE_MTS_ESP=1)

#

if (DEFINED BV_IGNORE_VDSP OR NOT APPLE)

    target_compile_definitions (Imogen PUBLIC JUCE_USE_VDSP_FRAMEWORK=0 BV_USE_VDSP=0)
    
    if (DEFINED BV_IGNORE_MIPP)
        target_compile_definitions (Imogen PUBLIC BV_USE_MIPP=0)
    else()
        message (STATUS "Configuring MIPP...")
	    target_compile_definitions (Imogen PUBLIC MIPP_ENABLE_BACKTRACE BV_USE_MIPP=1)
        target_include_directories (Imogen PUBLIC "third-party/MIPP/src" "MIPP")
    endif()
else()
    target_compile_definitions (Imogen PUBLIC BV_USE_VDSP=1 JUCE_USE_VDSP_FRAMEWORK=1)
endif()

#

target_link_libraries (Imogen PUBLIC
	    juce::juce_recommended_config_flags
	    juce::juce_recommended_lto_flags
	    juce::juce_recommended_warning_flags
)

target_compile_features (Imogen PUBLIC cxx_std_17)

target_compile_definitions (Imogen PUBLIC 
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_MODAL_LOOPS_PERMITTED=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    IMOGEN_REMOTE_APP=0
    )

#
