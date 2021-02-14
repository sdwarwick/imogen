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

# uncomment this to override the boolean GUI cache variables' settings for build formats, to easily & quickly type a new list of formats:
# set (formats VST AU Standalone)

message (STATUS "Welcome! I'm going to generate the Imogen IDE project for you.")

#

set (sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)  # This could conceivably be in a different place, but the source tree must be intact 

include (${sourceDir}/CMake/cmakeDefinitions.cmake REQUIRED)
include (${sourceDir}/CMake/cmakeUtilityFunctions.cmake REQUIRED)

#

imogen_checkAllDirectories()

if (IMOGEN_unitTesting)
    message (STATUS "JUCE and Catch2 will be downloaded from GitHub, only if the most recent versions can't be found locally.")
else()
    message (STATUS "JUCE will be downloaded from GitHub, only if the most recent version can't be found locally.")
endif()

imogen_makeFormatsList()

if (IMOGEN_launchAudioPluginHostOnBuild OR IMOGEN_launchStandaloneOnBuild)
    imogen_checkIfCanUseExecutables()
endif()

#

set_property (GLOBAL PROPERTY USE_FOLDERS YES)
set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Build Targets")

set (CMAKE_OSX_ARCHITECTURES "arm64" "x86_64")  # universal macOS binaries 
set (CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version" FORCE)  # minimum MacOS version to build for

set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # static linking on Windows
set (CMAKE_WIN32_EXECUTABLE TRUE)

set (CMAKE_XCODE_GENERATE_SCHEME OFF)  # schemes are maually generated for each target to avoid clutter from modules getting schemes, etc
set (CMAKE_XCODE_SCHEME_THREAD_SANITIZER ON)
set (CMAKE_XCODE_SCHEME_UNDEFINED_BEHAVIOR_SANITIZER ON)

set (CMAKE_SUPPRESS_REGENERATION TRUE)  # no "zero-check" target
set (CMAKE_OPTIMIZE_DEPENDENCIES TRUE)
set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option (JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)
option (JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)

#

imogen_addJuce()

juce_add_modules (${customModulesNeeded})

#

if (IMOGEN_launchAudioPluginHostOnBuild)
    imogen_configureAudioPluginHostExecutable()
endif()

if (IMOGEN_launchAudioPluginHostOnBuild OR IMOGEN_launchStandaloneOnBuild)
    imogen_configureStandaloneExecutable()
endif()

#

message (STATUS "Configuring Imogen...")

juce_add_plugin (${CMAKE_PROJECT_NAME}   # this command creates a static library for the shared plugin code, and adds targets for each of the specified plugin formats, as well as an "All" target, linked to the shared static lib
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
    COMPANY_COPYRIGHT           This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License.)

target_compile_features (Imogen PRIVATE ${imogen_compileFeatures})

target_compile_definitions (Imogen PUBLIC 
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0)

imogen_configureIndividualBuildTargets()

#

target_sources (${CMAKE_PROJECT_NAME} PRIVATE ${sourceFiles})

source_group (TREE ${sourceDir} PREFIX "" FILES ${sourceFiles})

target_include_directories (${CMAKE_PROJECT_NAME} PRIVATE ${pluginSourcesDir} ${guiSourcePath})

set_target_properties (${CMAKE_PROJECT_NAME} PROPERTIES FOLDER "")  # prevets IDE from generating an extra nested folder

#

juce_add_binary_data (ImogenGraphicAssets SOURCES 
    ${GraphicAssetsDir}/1-1_note_semibreve.svg
    ${GraphicAssetsDir}/closeIcon.png
    ${GraphicAssetsDir}/grandStaff.png
    ${CMAKE_CURRENT_LIST_DIR}/imogen_icon.png)

set_target_properties (ImogenGraphicAssets PROPERTIES FOLDER "${CMAKE_PROJECT_NAME}")

#

target_link_libraries (${CMAKE_PROJECT_NAME}
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

