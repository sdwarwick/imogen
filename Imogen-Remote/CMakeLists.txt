
cmake_minimum_required (VERSION 3.18) 

project (ImogenRemote VERSION 0.0.1 LANGUAGES CXX) 

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/../Source)

#

set (bv_use_AbletonLink FALSE)
set (bv_use_MTS_ESP FALSE)

#

include (${Imogen_sourceDir}/imogenConfig.cmake)

#

set (ImogenRemote_sourceFiles
    ${Imogen_Common_SourceFiles}
    ${Imogen_GUI_SourceFiles}
	${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.h
    ${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.cpp
    ${Imogen_sourceDir}/Remote/Main.cpp)

juce_add_module ("${Imogen_sourceDir}/Shared-code/bv_SharedCode")

#

juce_add_gui_app (ImogenRemote
    ${Imogen_Common_Flags}
    DESCRIPTION                   "Remote control for the Imogen plugin"
    DOCUMENT_BROWSER_ENABLED      TRUE
    MICROPHONE_PERMISSION_ENABLED FALSE
    BACKGROUND_AUDIO_ENABLED      TRUE # for iOS
    )

#

target_sources (ImogenRemote PRIVATE ${ImogenRemote_sourceFiles})

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR}/.. PREFIX "" FILES ${ImogenRemote_sourceFiles})

target_include_directories (ImogenRemote PRIVATE ${Imogen_sourceDir})

target_link_libraries (ImogenRemote PRIVATE bv_SharedCode)

set_target_properties (ImogenRemote PROPERTIES FOLDER "" XCODE_GENERATE_SCHEME ON)  # prevets IDE from generating an extra nested folder

#
# Add binary data

file (REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/Builds/juce_binarydata_ImogenRemoteGraphicAssets)
juce_add_binary_data (ImogenRemoteGraphicAssets SOURCES ${Imogen_assetFiles})
set_target_properties (ImogenRemoteGraphicAssets PROPERTIES FOLDER "ImogenRemote")

target_link_libraries (ImogenRemote PRIVATE ImogenRemoteGraphicAssets)

#

imogenConfig_step2()

#

if (NOT DEFINED bv_use_OSC)
    set (bv_use_OSC FALSE)
endif()

if (bv_use_OSC)
    target_compile_definitions (ImogenRemote PUBLIC IMOGEN_USE_OSC=1)
else()
    target_compile_definitions (ImogenRemote PUBLIC IMOGEN_USE_OSC=0)
endif()

target_compile_definitions (ImogenRemote PUBLIC 
    IMOGEN_REMOTE_APP=1
    IMOGEN_HEADLESS=0
    IMOGEN_USE_ABLETON_LINK=0
    bvsb_USE_MTS_ESP=0
    IMOGEN_USE_MTS_ESP=0
    )

#
