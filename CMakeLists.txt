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

set (pluginSourcesDir       ${Imogen_sourceDir}/PluginSources)  # The rest of the source tree (child folders of the main sourceDir specified above)
set (guiSourcePath          ${Imogen_sourceDir}/GUI)

set (Imogen_graphicAssetsDir ${CMAKE_CURRENT_LIST_DIR}/assets)  # The location of the graphical asset files (images, etc)

#

set (Imogen_sourceFiles
    ${pluginSourcesDir}/PluginProcessor.cpp
    ${pluginSourcesDir}/PluginProcessorParameters.cpp
    ${pluginSourcesDir}/PluginProcessor.h
    ${pluginSourcesDir}/PluginEditor.cpp
    ${pluginSourcesDir}/PluginEditor.h
    ${guiSourcePath}/ImogenGUI.h
    ${guiSourcePath}/ImogenGUI.cpp
    ${guiSourcePath}/LookAndFeel/ImogenLookAndFeel.h
    ${guiSourcePath}/LookAndFeel/ImogenLookAndFeel.cpp
    ${Imogen_sourceDir}/StandaloneWrapper/StandaloneWrapper.h
    ${Imogen_sourceDir}/StandaloneWrapper/StandaloneWrapper.cpp)

set (Imogen_graphicAssetFiles
    ${Imogen_graphicAssetsDir}/imogen_icon.png)

#

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

#

if (NOT DEFINED bv_formats)
    set (bv_formats AU VST3 Standalone Unity)
endif()

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

message (STATUS "Fetching the latest version of JUCE from GitHub repo: ${bv_juceGitRepoToUse}")

include (FetchContent)

FetchContent_Declare (juce
GIT_REPOSITORY ${bv_juceGitRepoToUse}
GIT_TAG        ${bv_juceGitRepo_TagToUse})

FetchContent_MakeAvailable (juce)

set (bv_juceDir ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/juce-src CACHE FILEPATH "${ds_juceDir}" FORCE)

#

if (ANDROID)
    set (OBOE_DIR "../JUCE/modules/juce_audio_devices/native/oboe")
    add_subdirectory (${OBOE_DIR} ./oboe)

    add_library ("cpufeatures" STATIC "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
    set_source_files_properties ("${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c" PROPERTIES COMPILE_FLAGS "-Wno-sign-conversion -Wno-gnu-statement-expression")

    enable_language (ASM)
endif()

#

set (thingsToLinkTo "")

#

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
    ICON_BIG                    ${Imogen_graphicAssetsDir}/imogen_icon.png
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

if (TARGET ${CMAKE_PROJECT_NAME}_LV2)
    set_source_files_properties (${bv_juceDir}/extras/Build/lv2_ttl_generator/lv2_ttl_generator.c PROPERTIES LANGUAGE CXX)
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
juce_add_binary_data (ImogenGraphicAssets SOURCES ${Imogen_graphicAssetFiles})
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
endif()

#

if (NOT "${thingsToLinkTo}" STREQUAL "")
	target_link_libraries (Imogen PRIVATE ${thingsToLinkTo})
endif()

if (ANDROID)
        target_link_libraries (Imogen PUBLIC ${log} ${android} ${glesv2} ${egl} ${cpufeatures_lib} ${oboe_lib})
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
    )
    
if (TARGET Imogen_LV2)
    set_target_properties (Imogen PROPERTIES JUCE_LV2_URI https://github.com/benthevining/imogen)
endif()

#
