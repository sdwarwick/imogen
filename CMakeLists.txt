#  IMOGEN 

#  AUTHOR: BEN VINING

#  This CMake script will generate an IDE project containing all the Imogen source code files, as well as the following build targets: a static library for the shared Imogen codebase; VST & AU plugins, and a standalone application. 
#  If you do not already have JUCE on your computer, you can specify a "juceDir" that does not exist in your filesystem; this script will automatically download the JUCE code from GitHub.
#  If you do have JUCE already and would like to avoid redownloading it, you can either move or copy your JUCE folder into the Imogen "Source" folder (so that JUCE is a subdirectory of the Imogen Source folder), and then make sure that the "juceDir" specified below matches the path to your JUCE folder. 


# TO DO:
# auto executables for generators other than XCode?
# get rid of binary data target scheme?
# automatically build AudioPluginHost if not found
# better cross-platforming of finding executable paths 
# Fetch content - only download if not found locally in _deps folder 
# in tests folder in IDE, hide modules (only show test files)


cmake_minimum_required (VERSION 3.15) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX)

#

set (unitTesting TRUE)  # set this to 'true' to enable a unit testing executable target

set (formats "AU" "VST3" "Standalone")  # valid formats: Standalone Unity VST3 AU AUv3 AAX VST. For AAX or VST, the path to the respective SDK must be provided using juce_set_aax_sdk_path or juce_set_vst2_sdk_path

set (juceDir ${CMAKE_CURRENT_LIST_DIR}/Source/JUCE)  # if this subdirectory isn't found, this script will automatically download the JUCE library code from GitHub

set (compileFeatures cxx_std_17)

# XCODE ONLY!
set (launchAudioPluginHostOnBuild FALSE)   #  when true, this automatically launches the AudioPluginHost on build for all targets except the Standalone, which uses itself as its own executable
set (launchStandaloneOnBuild      FALSE)   #  if launchAudioPluginHostOnBuild is false and this argument is true, then the Standalone will launch automatically for the Standalone and All targets, and the individual plugin formats will have no executable
set (preferStandaloneForAllTarget FALSE)  #  if both launchAudioPluginHostOnBuild and launchStandaloneOnBuild are true, then if this flag is set to true, the All target will use the Standalone as its executable instead of the AudioPluginHost. If this flag is false, the All target would default to using the AudioPluginHost.

#

message (STATUS "Welcome! I'm going to generate the Imogen IDE project for you.")

#

set (sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)  # This could conceivably be in a different place, but the source tree must be intact 
set (dspModulesPath ${sourceDir}/DSP_modules)  # The location of the custom JUCE-style modules that make up the shared Imogen codebase. Again, these could conceivably be in a different place...  ¯\_(ツ)_/¯

set (pluginSourcesDir       ${sourceDir}/PluginSources)  # The rest of the source tree (child folders of the main sourceDir specified above)
set (guiSourcePath          ${sourceDir}/GUI)
set (HelpScreenSourcePath   ${guiSourcePath}/HelpScreen)
set (IOPanelSourcePath      ${guiSourcePath}/IOControlPanel)
set (MidiControlSourcePath  ${guiSourcePath}/MidiControlPanel)
set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (GraphicAssetsDir ${guiSourcePath}/GraphicAssets)  # The location of the graphical asset files (images, etc)

set (testFilesPath ${sourceDir}/Tests)  # The location of the source files in which unit tests are defined

#

# testing that all the directories we need exist (ie, the source tree is intact)

if (NOT EXISTS ${sourceDir})
    message (FATAL_ERROR "Source code folder not found")
elseif (NOT EXISTS ${dspModulesPath})
    message (FATAL_ERROR "DSP modules folder not found")
elseif (NOT EXISTS ${GraphicAssetsDir})
    message (FATAL_ERROR "Graphic assets folder not found")
endif()

foreach (path ${pluginSourcesDir} ${guiSourcePath} ${HelpScreenSourcePath} ${IOPanelSourcePath} ${MidiControlSourcePath} ${StaffDisplaySourcePath})
    if (NOT EXISTS ${path})
        message (FATAL_ERROR "Source tree not intact - one or more child folders missing")
    endif()
endforeach()

if (unitTesting AND NOT EXISTS ${testFilesPath})
    set (unitTesting FALSE)
    message (WARNING "Test files directory not found, testing disabled")
endif()

#

if (unitTesting)
    message (STATUS "JUCE and Catch2 will be downloaded from GitHub, only if the most recent versions can't be found locally.")
else()
    message (STATUS "JUCE will be downloaded from GitHub, only if the most recent version can't be found locally.")
endif()

#

if (launchAudioPluginHostOnBuild OR launchStandaloneOnBuild)
    if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
        message (WARNING "Auto-launching executables are currently XCode only; these have been disabled because CMake has detected that you are not generating for XCode.")
        set (launchAudioPluginHostOnBuild FALSE)
        set (launchStandaloneOnBuild FALSE)
    elseif (NOT (APPLE OR UNIX OR WIN32))
        message (WARNING "Unrecognized operating system; auto-launching executables disabled")
        set (launchAudioPluginHostOnBuild FALSE)
        set (launchStandaloneOnBuild FALSE)
    endif()
endif()

#

set (isBuildingStandalone FALSE)

foreach (format ${formats})      
    if ("${format}" STREQUAL "Standalone")
        set (isBuildingStandalone TRUE)
        break()
    endif()
endforeach()

#

# attempt to locate standalone executable

if (launchAudioPluginHostOnBuild OR launchStandaloneOnBuild)

    set (standalone_exec_path ${CMAKE_CURRENT_LIST_DIR}/Builds/Imogen_artefacts/Debug/Standalone) 
    
    if (APPLE)
        set (standalone_exec_path ${standalone_exec_path}/Imogen.app)
    elseif (UNIX)
        set (AudioPluginHost_Path ${AudioPluginHost_Path}/Imogen.elf)
    elseif (WIN32)
        set (AudioPluginHost_Path ${AudioPluginHost_Path}/Imogen.exe)
    endif()

    if (isBuildingStandalone)
        set (canUseStandaloneExec TRUE)
    elseif (EXISTS ${standalone_exec_path})
        set (canUseStandaloneExec TRUE)
        message (WARNING "The Standalone executable was located and can be used, but you are not rebuilding the Standalone with this build, so its behavior may not reflect the most recent code changes.")
    else()
        message (WARNING "Standalone executable not found, and Standalone is not a current build target. Auto-launch feature disabled.")
        set (canUseStandaloneExec FALSE)
        set (launchStandaloneOnBuild FALSE)
    endif()

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

set (fetchcontentincluded FALSE)  # simple include guard for the 'FetchContent' package

#

# add JUCE 

if (EXISTS ${juceDir})
    add_subdirectory (${juceDir} REQUIRED)
else()
    include (FetchContent)
    set (fetchcontentincluded TRUE)  # simple include guard 

    message (STATUS "Fetching JUCE from GitHub...")
    
    FetchContent_Declare (juce   #  Downloads the latest version of JUCE if the indicated subdirectory cannot be found
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        origin/develop)

    FetchContent_MakeAvailable (juce)

    set (juceDir ${CMAKE_CURRENT_LIST_DIR}/Builds/_deps/juce-src)  # this is where FetchContent will download the JUCE library code to
endif()

#

# configure AudioPluginHost executable 

if (launchAudioPluginHostOnBuild)

    message (STATUS "Configuring AudioPluginHost...")

    set (AudioPluginHost_Path ${juceDir}/extras/AudioPluginHost/Builds)

    if (APPLE)
        set (AudioPluginHost_Path ${AudioPluginHost_Path}/MacOSX/build/Debug/AudioPluginHost.app)
    elseif (UNIX)
        set (AudioPluginHost_Path ${AudioPluginHost_Path}/LinuxMakefile/build/Debug/AudioPluginHost.elf)
    elseif (WIN32)
        set (AudioPluginHost_Path ${AudioPluginHost_Path}/VisualStudio2019/build/Debug/AudioPluginHost.exe)
    endif()

    if (NOT EXISTS ${AudioPluginHost_Path})
        message (WARNING "AudioPluginHost executable could not be found; auto-launch feature disabled")
        set (launchAudioPluginHostOnBuild FALSE)

        # TO DO: automatically build the APH here...?

    endif()

endif()

#

juce_add_modules (
    ${dspModulesPath}/bv_GeneralUtils
    ${dspModulesPath}/bv_PitchDetector
    ${dspModulesPath}/bv_Harmonizer
    ${dspModulesPath}/bv_ImogenEngine)

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

target_compile_features (Imogen PRIVATE ${compileFeatures})

target_compile_definitions (Imogen PUBLIC 
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0)

#

if (launchAudioPluginHostOnBuild OR launchStandaloneOnBuild)
    if (preferStandaloneForAllTarget OR NOT launchAudioPluginHostOnBuild)
        set (usingStandaloneForAllTarget TRUE)
    else()
        set (usingStandaloneForAllTarget FALSE)
    endif()
endif()

# This cleans up folder organization, putting each individual plugin format target generated by the juce_add_plugin call into a "Build Targets" folder, and also configures XCode scheme executables for each target

foreach (target ${formats} "All") 

    set (thisTargetName "${CMAKE_PROJECT_NAME}_${target}")  # this is how JUCE automatically names the build targets created for each format

    if (NOT TARGET ${thisTargetName})
        continue()
    endif()

    set_target_properties (${thisTargetName} PROPERTIES FOLDER "Build Targets" XCODE_GENERATE_SCHEME ON)

    if (NOT launchAudioPluginHostOnBuild AND NOT launchStandaloneOnBuild)
        continue()
    endif()

    if (canUseStandaloneExec)
        if ( "${target}" STREQUAL "Standalone" 
         OR ("${target}" STREQUAL "All" AND usingStandaloneForAllTarget) )
            set_target_properties (${thisTargetName} PROPERTIES XCODE_SCHEME_EXECUTABLE ${standalone_exec_path})
            message (STATUS "Executable for ${thisTargetName} set to 'Standalone'")
            continue()
        endif()
    elseif ("${target}" STREQUAL "Standalone")
        continue()
    endif()

    if (launchAudioPluginHostOnBuild)
        set_target_properties (${thisTargetName} PROPERTIES XCODE_SCHEME_EXECUTABLE ${AudioPluginHost_Path})
        message (STATUS "Executable for ${thisTargetName} set to 'AudioPluginHost'")
    endif()

endforeach()

unset (canUseStandaloneExec)
unset (usingStandaloneForAllTarget)

#

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

source_group (TREE ${sourceDir} PREFIX "" FILES ${sourceFiles})

target_include_directories (${CMAKE_PROJECT_NAME} PRIVATE ${pluginSourcesDir} ${guiSourcePath})

set_target_properties ("${CMAKE_PROJECT_NAME}" PROPERTIES FOLDER "")  # prevets IDE from generating an extra nested folder

#

juce_add_binary_data (ImogenGraphicAssets SOURCES # this creates a static library target which JUCE will populate with Imogen's graphic assets as binary files, so it can then be linked to
    ${GraphicAssetsDir}/1-1_note_semibreve.svg 
    ${GraphicAssetsDir}/closeIcon.png 
    ${GraphicAssetsDir}/grandStaff.png)

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

if (NOT unitTesting)
    return()
endif()

#

if (NOT fetchcontentincluded)  # simple include guard 
    include (FetchContent)
endif()

message (STATUS "Fetching Catch2 from GitHub...")

FetchContent_Declare (Catch2  # Downloads the latest version of Catch2 if not found 
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.3)

FetchContent_MakeAvailable (Catch2)

message (STATUS "Configuring unit testing...")

set (testFiles
    ${testFilesPath}/tests.cpp
    ${testFilesPath}/HarmonizerTests.cpp) 

source_group (TREE ${testFilesPath} PREFIX "" FILES ${testFiles})

add_executable (Tests ${testFiles})   # Test executable for unit testing

set_target_properties (Tests PROPERTIES FOLDER "${CMAKE_PROJECT_NAME}" XCODE_GENERATE_SCHEME ON)

target_include_directories (Tests PRIVATE ${testFilesPath})

target_compile_features (Tests PRIVATE ${compileFeatures})

target_link_libraries (Tests
    PRIVATE
        Catch2::Catch2
        bv_ImogenEngine
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

include (${Catch2_SOURCE_DIR}/contrib/Catch.cmake)

catch_discover_tests (Tests)

#

