
cmake_minimum_required (VERSION 3.18) 

project (ImogenRemote VERSION 0.0.1 LANGUAGES CXX) 

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/../Source)

include (${Imogen_sourceDir}/imogenConfig.cmake)

#

set (ImogenRemote_sourceFiles
    ${Imogen_sourceDir}/ImogenCommon.h
    ${Imogen_GUI_SourceFiles}
    ${Imogen_OSC_SourceFiles}
	${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.h
    ${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.cpp
    ${Imogen_sourceDir}/Remote/Main.cpp)

juce_add_module ("${Imogen_sourceDir}/Shared-code/bv_SharedCode")

#

juce_add_gui_app (ImogenRemote
    PRODUCT_NAME                ImogenRemote
    VERSION                     ${CMAKE_PROJECT_VERSION}
    BUNDLE_ID                   com.BenViningMusicSoftware.ImogenRemote
    DESCRIPTION                 "Remote control for the Imogen plugin"
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    ICON_BIG                    ${ImogenIconPath}
    COMPANY_NAME                BenViningMusicSoftware
    COMPANY_WEBSITE             www.benvining.com
    COMPANY_EMAIL               ben.the.vining@gmail.com
    COMPANY_COPYRIGHT           "This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License."
    MICROPHONE_PERMISSION_ENABLED        FALSE
    SEND_APPLE_EVENTS_PERMISSION_ENABLED FALSE
    DOCUMENT_BROWSER_ENABLED             TRUE
    STATUS_BAR_HIDDEN                    TRUE  # for iOS
    REQUIRES_FULL_SCREEN                 TRUE  # for iOS
    IPAD_SCREEN_ORIENTATIONS             UIInterfaceOrientationUnknown, UIInterfaceOrientationLandscapeLeft, UIInterfaceOrientationLandscapeRight
    TARGETED_DEVICE_FAMILY               2     # target iPad only
    DOCUMENT_EXTENSIONS xml
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
