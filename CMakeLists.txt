#  IMOGEN 

#  AUTHOR: BEN VINING

#  This CMake script will generate an IDE project containing all the Imogen source code files, as well as the following build targets: a static library for the shared Imogen codebase; VST & AU plugins, and a standalone application.

#  The Imogen source code consists of dependencies on a few JUCE modules; a couple of custom JUCE-style modules I created; and the PluginProcessor & PluginEditor source files specific to the plugin build. The bv_ImogenEngine module is a completely contained and self-sufficient audio processor with all of Imogen's features; other source files could replace the PluginProcessor/PluginEditor to adapt Imogen for other build targets.


cmake_minimum_required (VERSION 3.15) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX)

#

set (formats "AU" "VST3" "Standalone")  # valid formats are: 

set (CMAKE_OSX_ARCHITECTURES arm64 x86_64)

set (compileFeatures cxx_std_17)

set (unitTesting FALSE)  # set this to 'true' to enable a unit testing executable target

set (lauchAudioPluginHostOnBuild FALSE)  # set this to 'true' to automatically lauch JUCE's AudioPluginHost when the project builds. The AudioPluginHost must be built before building Imogen if this is enabled!

set (sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)

set (juceDir ${sourceDir}/JUCE)

# set catch2 dir 

# param for permission to install juce/catch if needed?

#

set (CMAKE_XCODE_GENERATE_SCHEME OFF) # schemes are maually generated for each target to avoid clutter from modules getting schemes, etc

set (CMAKE_SUPPRESS_REGENERATION true) # no "zero-check" target

set_property (GLOBAL PROPERTY USE_FOLDERS YES)

set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Targets")

option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

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

#######

# PLUGIN SOURCE FILES

set (pluginSourcesDir ${sourceDir}/pluginSources)

set (pluginSources
    ${pluginSourcesDir}/PluginProcessor.cpp
    ${pluginSourcesDir}/PluginProcessor.h
    ${pluginSourcesDir}/PluginEditor.cpp
    ${pluginSourcesDir}/PluginEditor.h)

source_group (
    TREE ${pluginSourcesDir}
    PREFIX "Plugin Sources"
    FILES ${pluginSources})

#

# GUI SOURCE FILES

set (guiSourcePath ${sourceDir}/GUI)

set (guiSources
    ${guiSourcePath}/LookAndFeel.h
    ${guiSourcePath}/EnableSidechainWarning.h)

source_group (
    TREE ${guiSourcePath}
    PREFIX "GUI"
    FILES ${guiSources})


set (HelpScreenSourcePath ${guiSourcePath}/HelpScreen)

set (HelpScreenSources
    ${HelpScreenSourcePath}/HelpScreen.cpp
    ${HelpScreenSourcePath}/HelpScreen.h)

source_group (
    TREE ${HelpScreenSourcePath}
    PREFIX "GUI/HelpScreen"
    FILES ${HelpScreenSources})


set (IOPanelSourcePath ${guiSourcePath}/IOControlPanel)

set (IOControlSources
    ${IOPanelSourcePath}/IOControlPanel.cpp
    ${IOPanelSourcePath}/IOControlPanel.h
    ${IOPanelSourcePath}/LimiterControlPanel.h)

source_group (
    TREE ${IOPanelSourcePath}
    PREFIX "GUI/IOControlPanel"
    FILES ${IOControlSources})


set (MidiControlSourcePath ${guiSourcePath}/MidiControlPanel)

set (MidiControlSources
    ${MidiControlSourcePath}/MidiControlPanel.cpp
    ${MidiControlSourcePath}/MidiControlPanel.h)

source_group (
    TREE ${MidiControlSourcePath}
    PREFIX "GUI/MidiControlPanel"
    FILES ${MidiControlSources})


set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (StaffDisplaySources
    ${StaffDisplaySourcePath}/StaffDisplay.cpp
    ${StaffDisplaySourcePath}/StaffDisplay.h)

source_group (
    TREE ${StaffDisplaySourcePath}
    PREFIX "GUI/StaffDisplay"
    FILES ${StaffDisplaySources})

#####

target_sources (${CMAKE_PROJECT_NAME} PRIVATE 
    ${pluginSources} 
    ${guiSources}
    ${HelpScreenSources}
    ${IOControlSources}
    ${MidiControlSources}
    ${StaffDisplaySources})

target_include_directories (${CMAKE_PROJECT_NAME} PRIVATE ${pluginSourcesDir} ${guiSourcePath})

set_target_properties ("${CMAKE_PROJECT_NAME}" PROPERTIES FOLDER "")  # prevets IDE from generating an extra nested folder

#####

set (GraphicAssetsDir ${guiSourcePath}/GraphicAssets)

juce_add_binary_data (ImogenGraphicAssets SOURCES # this creates a static library target which JUCE will populate with Imogen's graphic assets as binary files, so it can then be linked to
    ${GraphicAssetsDir}/1-1_note_semibreve.svg 
    ${GraphicAssetsDir}/closeIcon.png 
    ${GraphicAssetsDir}/grandStaff.png)

#####

# This cleans up folder organization, putting each individual plugin format build target into its own folder 

set (targetList "ImogenGraphicAssets") 

foreach (format ${formats})
    string (CONCAT newTargetName "${CMAKE_PROJECT_NAME}_" "${format}")
    list (APPEND targetList ${newTargetName})
endforeach()

list (APPEND targetList "${CMAKE_PROJECT_NAME}_All")

foreach (target ${targetList})
    if (TARGET ${target})
        set_target_properties (${target} PROPERTIES
            FOLDER "Targets" 
            XCODE_GENERATE_SCHEME ON)

        if (lauchAudioPluginHostOnBuild)
            set_target_properties (${target} PROPERTIES
                XCODE_SCHEME_EXECUTABLE "${juceDir}/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app")  # change this path to use the widows or linux AudioPluginHost executable...
        endif()
    endif()
endforeach()

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

source_group (
    TREE ${testFilesPath}
    PREFIX "Tests"
    FILES ${testFiles})

add_executable (tests ${testFiles})

target_include_directories (tests PRIVATE ${testFilesPath})

target_compile_features (tests PRIVATE ${compileFeatures})

target_link_libraries (tests
    PRIVATE
        bv_ImogenEngine
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

add_test (NAME tests COMMAND tests)

enable_testing()

#
