
cmake_minimum_required (VERSION 3.18) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX) 

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)  # This could conceivably be in a different place, but the source tree must be intact 

#

if (DEFINED bv_ignoreAllThirdPartyLibs)
	set (bv_use_AbletonLink FALSE)
	set (bv_use_MTS_ESP FALSE)
else()
	set (bv_use_AbletonLink TRUE)
	set (bv_use_MTS_ESP TRUE)
endif()

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

include (${Imogen_sourceDir}/imogenConfig.cmake)

#

set (Imogen_sourceFiles
    ${Imogen_Common_SourceFiles}
    ${Imogen_Processor_SourceFiles}
    ${Imogen_GUI_SourceFiles}
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.cpp
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_StandaloneWrapper/StandaloneWrapper.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_StandaloneWrapper/StandaloneWrapper.cpp)

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
    ${Imogen_Common_Flags}
    ${Imogen_Plugin_Flags}
    PLUGIN_CODE                 Imgn
    DOCUMENT_BROWSER_ENABLED    TRUE
    BACKGROUND_AUDIO_ENABLED	FALSE # for iOS
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
# Add binary data

file (REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/Builds/juce_binarydata_ImogenGraphicAssets)
juce_add_binary_data (ImogenGraphicAssets SOURCES ${Imogen_assetFiles})
set_target_properties (ImogenGraphicAssets PROPERTIES FOLDER "Imogen")
list (APPEND thingsToLinkTo ImogenGraphicAssets)

#

target_link_libraries (Imogen PRIVATE ${thingsToLinkTo})

#

if (bv_use_AbletonLink)
	include (${CMAKE_CURRENT_LIST_DIR}/third-party/ableton-link/AbletonLinkConfig.cmake)
	target_link_libraries (Imogen PRIVATE Ableton::Link)
	target_compile_definitions (Imogen PUBLIC IMOGEN_USE_ABLETON_LINK=1)
else()
	target_compile_definitions (Imogen PUBLIC IMOGEN_USE_ABLETON_LINK=0)
endif()

#

if (bv_use_MTS_ESP)
	target_include_directories (Imogen PUBLIC "third-party/MTS-ESP/Client" "MTS-ESP")
	target_compile_definitions (Imogen PUBLIC bvsb_USE_MTS_ESP=1)
else()
	target_compile_definitions (Imogen PUBLIC bvsb_USE_MTS_ESP=0)
endif()

#

imogenConfig_step2()

#

if (NOT DEFINED bv_use_OSC)
	set (bv_use_OSC FALSE)
endif()

if (bv_use_OSC)
	target_compile_definitions (Imogen PUBLIC IMOGEN_USE_OSC=1)
else()
	target_compile_definitions (Imogen PUBLIC IMOGEN_USE_OSC=0)
endif()

#

target_compile_definitions (Imogen PUBLIC 
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    IMOGEN_REMOTE_APP=0
    IMOGEN_HEADLESS=0
    )

#
