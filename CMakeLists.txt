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

#  CMakeLists.txt :	This file defines the build configuration for Imogen. This CMake script takes care of finding or downloading JUCE. This CMake script uses my JUCE-CmakeWrapper package as its core API.
 

cmake_minimum_required (VERSION 3.18) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX) 

#

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)  # This could conceivably be in a different place, but the source tree must be intact 

set (pluginSourcesDir       ${Imogen_sourceDir}/PluginSources)  # The rest of the source tree (child folders of the main sourceDir specified above)
set (guiSourcePath          ${Imogen_sourceDir}/GUI)
set (HelpScreenSourcePath   ${guiSourcePath}/HelpScreen)
set (IOPanelSourcePath      ${guiSourcePath}/IOControlPanel)
set (MidiControlSourcePath  ${guiSourcePath}/MidiControlPanel)
set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (Imogen_graphicAssetsDir ${guiSourcePath}/GraphicAssets)  # The location of the graphical asset files (images, etc)

set (Imogen_testFilesPath ${Imogen_sourceDir}/Tests)  # The location of the source files in which unit tests are defined

#

set (Imogen_customModulesPath ${Imogen_sourceDir}/DSP_modules)

set (Imogen_customModulesNeeded
    bv_Harmonizer
    bv_ImogenEngine)

#

set (Imogen_sourceFiles
    ${pluginSourcesDir}/PluginProcessor.cpp
    ${pluginSourcesDir}/PluginProcessorParameters.cpp
    ${pluginSourcesDir}/PluginProcessor.h
    ${pluginSourcesDir}/PluginEditor.cpp
    ${pluginSourcesDir}/PluginEditor.h
    ${guiSourcePath}/LookAndFeel/ImogenLookAndFeel.h
    ${guiSourcePath}/LookAndFeel/ImogenLookAndFeel.cpp)

set (Imogen_graphicAssetFiles
    ${CMAKE_CURRENT_SOURCE_DIR}/imogen_icon.png)

set (Imogen_testFiles
    ${Imogen_testFilesPath}/tests.cpp
    ${Imogen_testFilesPath}/HarmonizerTests.cpp) 

#

set (Imogen_Bundle_ID com.BenViningMusicSoftware.Imogen)
set (Imogen_Manufacturer_Code Benv)
set (Imogen_Plugin_Code Imgn)
set (Imogen_Description "Real-time vocal harmonizer instrument")
set (Imogen_Is_Synth FALSE)
set (Imogen_Needs_MIDI_Input TRUE)
set (Imogen_Needs_MIDI_Output TRUE)
set (Imogen_Is_MIDI_Effect FALSE)
set (Imogen_Editor_Wants_Keyboard_Focus FALSE)
set (Imogen_VST3_Categories "Pitch Shift")
set (Imogen_AU_Main_Type "kAudioUnitType_MusicEffect")
set (Imogen_Icon_Path ${CMAKE_CURRENT_SOURCE_DIR}/imogen_icon.png)
set (Imogen_Company_Name BenViningMusicSoftware)
set (Imogen_Company_Website "www.benvining.com")
set (Imogen_Company_Email "ben.the.vining@gmail.com")
set (Imogen_Company_Copyright "This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License.")
set (Imogen_Copy_Plugin_After_Build TRUE)

set (Imogen_UseRecommendedWarningFlags TRUE)

set (bv_generatePreprocessorDefinitions TRUE)

#

include (JUCE-CmakeWrapper/CMakeLists.txt)

#

juce_add_modules (${Imogen_sourceDir}/Shared-code/bv_SharedCode 
                  ${Imogen_customModulesPath}/Pitch-detector/bv_PitchDetector
                  ${Imogen_customModulesPath}/SynthBase/bv_SynthBase)

target_link_libraries (Imogen PRIVATE bv_SharedCode bv_PitchDetector bv_SynthBase)

#

target_compile_features (Imogen PUBLIC cxx_std_17)

target_compile_definitions (Imogen PUBLIC 
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_MODAL_LOOPS_PERMITTED=0
    )

#
