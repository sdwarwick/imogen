#  IMOGEN 

#  AUTHOR: BEN VINING

#  This CMake script will generate an IDE project containing all the Imogen source code files, as well as the following build targets: a static library for the shared Imogen codebase; VST & AU plugins, and a standalone application.

#  The Imogen source code consists of dependencies on a few JUCE modules; a couple of custom JUCE-style modules I created; and the PluginProcessor & PluginEditor source files specific to the plugin build. The bv_ImogenEngine module is a completely contained and self-sufficient audio processor with all of Imogen's features; other source files could replace the PluginProcessor/PluginEditor to adapt Imogen for other build targets.


cmake_minimum_required (VERSION 3.15) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX)

#

set (formats "AU" "VST3" "Standalone")  # valid formats are: 

set (CMAKE_OSX_ARCHITECTURES arm64 x86_64)

# Minimum MacOS target
set (CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version" FORCE)

# static linking on Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set (compileFeatures cxx_std_17)

set (unitTesting FALSE)  # set this to 'true' to enable a unit testing executable target

set (launchPluginHostOnBuild TRUE)

set (sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)

set (juceDir ${sourceDir}/JUCE)

#

set (CMAKE_XCODE_GENERATE_SCHEME OFF) # schemes are maually generated for each target to avoid clutter from modules getting schemes, etc

set (CMAKE_XCODE_SCHEME_THREAD_SANITIZER ON)
set (CMAKE_XCODE_SCHEME_UNDEFINED_BEHAVIOR_SANITIZER ON)

set (CMAKE_SUPPRESS_REGENERATION TRUE) # no "zero-check" target

set_property (GLOBAL PROPERTY USE_FOLDERS YES)

set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Targets")

option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

#

add_subdirectory (${juceDir})  # Juce can also be included using find_package, but that is more complex and I couldn't get it to work, so I just copied the library code into a subdirectory of my Source folder. Works for now.

set_directory_properties (PROPERTIES 
    JUCE_COMPANY_NAME             BenViningMusicSoftware
    JUCE_COMPANY_WEBSITE          www.benvining.com
    JUCE_COMPANY_EMAIL            ben.the.vining@gmail.com
    JUCE_COPY_PLUGIN_AFTER_BUILD  TRUE
    JUCE_VST3_COPY_DIR            /Users/benvining/Library/Audio/Plug-Ins/VST/MyPlugins
    JUCE_AU_COPY_DIR              /Users/benvining/Library/Audio/Plug-Ins/Components
    JUCE_COMPANY_COPYRIGHT        This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License.
    )

#

set (dspModulesPath ${sourceDir}/DSP_modules)

juce_add_modules (
    ${dspModulesPath}/bv_GeneralUtils
    ${dspModulesPath}/bv_PitchDetector
    ${dspModulesPath}/bv_Harmonizer
    ${dspModulesPath}/bv_ImogenEngine)

#

# this command adds a static library target with the shared plugin code that can be linked to for the testing target, as well as build targets for VST & AU plugins and a standalone:

juce_add_plugin (${CMAKE_PROJECT_NAME} 
    PRODUCT_NAME ${CMAKE_PROJECT_NAME}
    VERSION ${CMAKE_PROJECT_VERSION}
    BUNDLE_ID com.BenViningMusicSoftware.Imogen
    PLUGIN_NAME ${CMAKE_PROJECT_NAME}
    PLUGIN_MANUFACTURER_CODE Benv
    DESCRIPTION Real-time vocal harmonizer instrument
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    PLUGIN_CODE Imgn
    FORMATS ${formats}
    VST3_CATEGORIES Pitch Shift
    AU_MAIN_TYPE kAudioUnitType_MusicEffect
    ICON_BIG ${CMAKE_CURRENT_LIST_DIR}/imogen_icon.png
    )

target_compile_features (${CMAKE_PROJECT_NAME} PRIVATE ${compileFeatures})

target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC 
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0)

foreach (target ${formats} "All")  # This cleans up folder organization, putting each individual plugin format target into a "Targets" folder
    if (TARGET ${CMAKE_PROJECT_NAME}_${target})
        set_target_properties (${CMAKE_PROJECT_NAME}_${target} PROPERTIES
            FOLDER "Targets" 
            XCODE_GENERATE_SCHEME ON)

        if (launchPluginHostOnBuild)
            set_target_properties (${CMAKE_PROJECT_NAME}_${target} PROPERTIES
                XCODE_SCHEME_EXECUTABLE "${juceDir}/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app")  # Note that for this to work, the AudioPluginHost executable must first be built from the JUCE source code, and if you are not on MacOSX, this path must be changed to the correct executable for your OS
        endif()
    endif()
endforeach()

#######

set (pluginSourcesDir ${sourceDir}/PluginSources)
set (guiSourcePath ${sourceDir}/GUI)
set (HelpScreenSourcePath ${guiSourcePath}/HelpScreen)
set (IOPanelSourcePath ${guiSourcePath}/IOControlPanel)
set (MidiControlSourcePath ${guiSourcePath}/MidiControlPanel)
set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (sourceFiles
    ${pluginSourcesDir}/PluginProcessor.cpp
    ${pluginSourcesDir}/PluginProcessor.h
    ${pluginSourcesDir}/PluginEditor.cpp
    ${pluginSourcesDir}/PluginEditor.h
    ${guiSourcePath}/LookAndFeel.h
    ${guiSourcePath}/EnableSidechainWarning.h
    ${HelpScreenSourcePath}/HelpScreen.cpp
    ${HelpScreenSourcePath}/HelpScreen.h
    ${IOPanelSourcePath}/IOControlPanel.cpp  
    ${IOPanelSourcePath}/IOControlPanel.h
    ${IOPanelSourcePath}/LimiterControlPanel.h
    ${MidiControlSourcePath}/MidiControlPanel.cpp
    ${MidiControlSourcePath}/MidiControlPanel.h
    ${StaffDisplaySourcePath}/StaffDisplay.cpp
    ${StaffDisplaySourcePath}/StaffDisplay.h)

target_sources (${CMAKE_PROJECT_NAME} PRIVATE ${sourceFiles})

target_include_directories (${CMAKE_PROJECT_NAME} PRIVATE ${pluginSourcesDir} ${guiSourcePath})

set_target_properties ("${CMAKE_PROJECT_NAME}" PROPERTIES FOLDER "")  # prevets IDE from generating an extra nested folder

source_group (TREE ${sourceDir} PREFIX "" FILES ${sourceFiles})

#####

set (GraphicAssetsDir ${guiSourcePath}/GraphicAssets)

juce_add_binary_data (ImogenGraphicAssets SOURCES # this creates a static library target which JUCE will populate with Imogen's graphic assets as binary files, so it can then be linked to
    ${GraphicAssetsDir}/1-1_note_semibreve.svg 
    ${GraphicAssetsDir}/closeIcon.png 
    ${GraphicAssetsDir}/grandStaff.png)

#####

target_link_libraries (${CMAKE_PROJECT_NAME}
    PRIVATE
        ImogenGraphicAssets
        bv_ImogenEngine
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

#######

if (NOT unitTesting)
    return()
endif()

#

# Test executable for unit testing

enable_testing()

set (testFilesPath ${sourceDir}/Tests) 

set (testFiles 
    ${testFilesPath}/tests.cpp 
    ${testFilesPath}/tests.h) 

# Download the tagged version of Catch2

include (FetchContent)

FetchContent_Declare (Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.3)

FetchContent_MakeAvailable (Catch2)

#

add_executable (tests ${testFiles})

target_include_directories (tests PRIVATE ${testFilesPath})

target_compile_features (tests PRIVATE ${compileFeatures})

target_link_libraries (tests
    PRIVATE
        Catch2::Catch2
        bv_ImogenEngine
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

set_target_properties (tests PROPERTIES XCODE_GENERATE_SCHEME ON)

source_group(TREE ${testFilesPath} PREFIX "" FILES ${testFiles})

include (${Catch2_SOURCE_DIR}/contrib/Catch.cmake)

catch_discover_tests (tests)

#
