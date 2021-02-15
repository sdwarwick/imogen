#  IMOGEN 

#  AUTHOR: BEN VINING

#  This CMake script will generate an IDE project containing all the Imogen source code files, as well as the following build targets: a static library for the shared Imogen codebase; VST & AU plugins, and a standalone application. 
#  If they cannot be located automatically, JUCE [and, if unit testing is enabled, Catch2] will be downloaded from GitHub.


# TO DO:
# auto executables for generators other than XCode?
# get rid of binary data target scheme?
# better cross-platforming of finding executable paths 
# in tests folder in IDE, hide modules (only show test files)


cmake_minimum_required (VERSION 3.15) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX)

#

set (imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source CACHE FILEPATH "The location of the Imogen source code folder")  # This could conceivably be in a different place, but the source tree must be intact 

include (${imogen_sourceDir}/CMake/cmakeDefinitions.cmake REQUIRED)
include (${imogen_sourceDir}/CMake/cmakeUtilityFunctions.cmake REQUIRED)

#  initializing all the cache variables & preferences:

set (IMOGEN_unitTesting ON CACHE BOOL "${ds_testing}")  # set this to 'true' to enable a unit testing executable target

set (IMOGEN_launchAudioPluginHostOnBuild OFF CACHE BOOL "${ds_launchAPH}")        #  when true, this automatically launches the AudioPluginHost on build for all targets except the Standalone, which uses itself as its own executable
set (IMOGEN_launchStandaloneOnBuild      OFF CACHE BOOL "${ds_launchSAL}")        #  if launchAudioPluginHostOnBuild is false and this argument is true, then the Standalone will launch automatically for the Standalone and All targets, and the individual plugin formats will have no executable
set (IMOGEN_preferStandaloneForAllTarget OFF CACHE BOOL "${ds_preferSALforAll}")  #  if both launchAudioPluginHostOnBuild and launchStandaloneOnBuild are true, then if this flag is set to true, the All target will use the Standalone as its executable instead of the AudioPluginHost. If this flag is false, the All target would default to using the AudioPluginHost.

set (IMOGEN_buildStandalone ON  CACHE BOOL "Builds a standalone application version of Imogen")
set (IMOGEN_buildVST3       ON  CACHE BOOL "Builds a VST3 version of Imogen")
set (IMOGEN_buildAU         ON  CACHE BOOL "Builds an AudioUnit version of Imogen")
set (IMOGEN_buildUnity      OFF CACHE BOOL "Builds a Unity plugin version of Imogen {UNTESTED}")
set (IMOGEN_buildAUv3       OFF CACHE BOOL "Builds an Auv3 version of Imogen")
set (IMOGEN_buildAAX        OFF CACHE BOOL "Builds an AAX version of Imogen. NOTE: for this to work, you must modify the main Imogen CMakeLists.txt to provide the path to the AAX SDK by calling juce_set_aax_sdk_path before the call to juce_add_plugin")
set (IMOGEN_buildVST        OFF CACHE BOOL "Builds a VST2 version of Imogen. NOTE: for this to work, you must modify the main Imogen CMakeLists.txt to provide the path to the VST2 SDK by calling juce_set_vst2_sdk_path before the call to juce_add_plugin")

set (imogen_juceDir  ${CMAKE_CURRENT_SOURCE_DIR}/Source/JUCE             CACHE FILEPATH "${ds_juceDir}")  # if this subdirectory isn't found, this script will automatically download the JUCE library code from GitHub
set (imogen_catchDir ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/catch2-src CACHE FILEPATH "${ds_catchDir}") # if this subdirectory isn't found and unit testing is enabled, this script will automatically download Catch2 from GitHub

set (imogen_GraphicAssetsDir ${guiSourcePath}/GraphicAssets CACHE FILEPATH "location of the Imogen grapic asset files")  # The location of the graphical asset files (images, etc)

set (imogen_standalone_exec_path "" CACHE FILEPATH "${ds_SALpath}")  # path to the Imogen standalone executable, if it exists 
set (imogen_AudioPluginHost_Path "" CACHE FILEPATH "${ds_APHpath}")  # path to the JUCE AudioPluginHost executable, if it exists

set (imogen_compileFeatures "cxx_std_17" CACHE STRING "Build compile features")

#  uncomment this 'set formats' call to override the boolean GUI cache variables' settings for build formats, to easily & quickly type a new list of formats as text strings. Strings entered here MUST be valid JUCE build formats, separated by whitespace. Any non-valid strings will be removed from the list.
#  valid formats are: Standalone VST3 AU Unity AUv3 AAX VST
#  for AAX or VST [vst2], the API paths must be provided manually by calling juce_set_aax_sdk_path and/or juce_set_vst2_sdk_path, respectively. These calls are provided below, simply fill in the absolute path to the SDK on your filesystem and uncomment the applicable line.
#
# set (formats AU VST3 Standalone)

#

message (STATUS "Welcome! I'm going to generate the Imogen IDE project for you.")

imogen_checkAllDirectories()

imogen_makeFormatsList()

if (IMOGEN_launchAudioPluginHostOnBuild OR IMOGEN_launchStandaloneOnBuild)
    imogen_checkIfCanUseExecutables()
endif()

if (IMOGEN_unitTesting)
    message (STATUS "JUCE and Catch2 will be downloaded from GitHub, only if the most recent versions can't be found locally.")
else()
    message (STATUS "JUCE will be downloaded from GitHub, only if the most recent version can't be found locally.")
endif()

#

imogen_addJuce()

juce_add_modules (${customModulesNeeded})

#  To build AAX or VST2 plugins, fill in the absolute paths to the SDKs on your filesystem and uncomment these function calls

# juce_set_aax_sdk_path (path)
# juce_set_vst2_sdk_path (path)

#

if (IMOGEN_launchAudioPluginHostOnBuild OR IMOGEN_launchStandaloneOnBuild)

    imogen_configureAudioPluginHostExecutable()

    imogen_configureStandaloneExecutable()

endif()

#

message (STATUS "Configuring Imogen...")

juce_add_plugin (Imogen   # this command creates a static library for the shared plugin code, and adds targets for each of the specified plugin formats, as well as an "All" target, linked to the shared static lib
    PRODUCT_NAME                Imogen
    VERSION                     ${CMAKE_PROJECT_VERSION}
    BUNDLE_ID                   com.BenViningMusicSoftware.Imogen
    PLUGIN_NAME                 Imogen
    PLUGIN_MANUFACTURER_CODE    Benv
    DESCRIPTION                 Real-time vocal harmonizer instrument
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    PLUGIN_CODE                 Imgn
    FORMATS                     ${formats}
    VST3_CATEGORIES             Pitch Shift
    AU_MAIN_TYPE                kAudioUnitType_MusicEffect
    ICON_BIG                    ${CMAKE_CURRENT_LIST_DIR}/imogen_icon.png
    COMPANY_NAME                BenViningMusicSoftware
    COMPANY_WEBSITE             www.benvining.com
    COMPANY_EMAIL               ben.the.vining@gmail.com
    COPY_PLUGIN_AFTER_BUILD     TRUE
    VST3_COPY_DIR               /Users/benvining/Library/Audio/Plug-Ins/VST/MyPlugins
    AU_COPY_DIR                 /Users/benvining/Library/Audio/Plug-Ins/Components
    UNITY_COPY_DIR              ${imogen_sourceDir}/Builds/Imogen_artefacts  # unlike the other copy locations, Unity doesn't have a default copy location, so if you select Unity as a format and enable copy plugin on build, you must provide a Unity copy dir here
    COMPANY_COPYRIGHT           This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License.)

target_compile_features (Imogen PRIVATE ${imogen_compileFeatures})

target_compile_definitions (Imogen PUBLIC 
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0)

imogen_configureIndividualBuildTargets()

#

target_sources (Imogen PRIVATE ${sourceFiles})

source_group (TREE ${imogen_sourceDir} PREFIX "" FILES ${sourceFiles})

target_include_directories (Imogen PRIVATE ${pluginSourcesDir} ${guiSourcePath})

set_target_properties (Imogen PROPERTIES FOLDER "")  # prevets IDE from generating an extra nested folder

#

juce_add_binary_data (ImogenGraphicAssets SOURCES ${graphicAssetFiles})

set_target_properties (ImogenGraphicAssets PROPERTIES FOLDER "Imogen")

#

target_link_libraries (Imogen
    PRIVATE
        ImogenGraphicAssets
        bv_ImogenEngine
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

#

if (IMOGEN_unitTesting)
    imogen_configureUnitTesting()
endif()

#

_imogen_unset_all_variables()

#

