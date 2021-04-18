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

set (Imogen_graphicAssetsDir ${guiSourcePath}/GraphicAssets)  # The location of the graphical asset files (images, etc)


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
    ${guiSourcePath}/ImogenGUI.h
    ${guiSourcePath}/ImogenGUI.cpp
    ${guiSourcePath}/LookAndFeel/ImogenLookAndFeel.h
    ${guiSourcePath}/LookAndFeel/ImogenLookAndFeel.cpp
    ${Imogen_sourceDir}/StandaloneWrapper/StandaloneWrapper.h
    ${Imogen_sourceDir}/StandaloneWrapper/StandaloneWrapper.cpp)

set (Imogen_graphicAssetFiles
    ${CMAKE_CURRENT_SOURCE_DIR}/imogen_icon.png)

#

set (bv_useSharedCodeModule TRUE)

set (bv_useSynthBaseModule TRUE)
set (bv_SynthBaseModule_Path ${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP_modules/SynthBase)

#

if (NOT DEFINED bv_juceGitRepoToUse OR "${bv_juceGitRepoToUse}" STREQUAL "https://github.com/juce-framework/JUCE.git")
        set (bv_juceGitRepo_TagToUse origin/develop)
endif()

#

include (JUCE-CmakeWrapper/jcmw_Step1.cmake)

#

juce_add_plugin (${CMAKE_PROJECT_NAME}  
    PRODUCT_NAME                Imogen
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
    PLUGIN_CODE                 Imgn
    FORMATS                     ${formats}
    VST3_CATEGORIES             "Pitch Shift"
    AU_MAIN_TYPE                "kAudioUnitType_MusicEffect"
    ICON_BIG                    ${CMAKE_CURRENT_SOURCE_DIR}/imogen_icon.png
    COMPANY_NAME                BenViningMusicSoftware
    COMPANY_WEBSITE             www.benvining.com
    COMPANY_EMAIL               ben.the.vining@gmail.com
    COPY_PLUGIN_AFTER_BUILD     TRUE
    UNITY_COPY_DIR              ${CMAKE_CURRENT_SOURCE_DIR}/Builds/${CMAKE_PROJECT_NAME}_artefacts  # unlike the other copy locations, Unity doesn't have a default copy location, so if you select Unity as a format and enable copy plugin on build, you must provide a Unity copy dir here
    COMPANY_COPYRIGHT           "This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License."
    )

#

include (JUCE-CmakeWrapper/jcmw_Step2.cmake)

#

target_link_libraries (${CMAKE_PROJECT_NAME} PUBLIC
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
