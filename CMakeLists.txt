
cmake_minimum_required (VERSION 3.18) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX) 

#

set (bvsb_USE_MTS_ESP TRUE)
include (CMake/FindShared-code.cmake)

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)

juce_add_modules ("${Imogen_sourceDir}/ImogenCommon"
				  "${Imogen_sourceDir}/DSP/ImogenEngine"
				  "${Imogen_sourceDir}/GUI/ImogenGUI")

include (${CMAKE_CURRENT_LIST_DIR}/third-party/ableton-link/AbletonLinkConfig.cmake)

#

set (Imogen_Common_Flags
    PRODUCT_NAME                ${CMAKE_PROJECT_NAME}
    VERSION                     ${CMAKE_PROJECT_VERSION}
    BUNDLE_ID                   com.BenViningMusicSoftware.Imogen
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    ICON_BIG                    ${CMAKE_CURRENT_LIST_DIR}/assets/graphics/imogen_icon.png
    COMPANY_NAME                BenViningMusicSoftware
    COMPANY_WEBSITE             www.benvining.com
    COMPANY_EMAIL               ben.the.vining@gmail.com
    COMPANY_COPYRIGHT           "Copyright 2021 Ben Vining"
    STATUS_BAR_HIDDEN           TRUE  # for iOS
    REQUIRES_FULL_SCREEN        TRUE  # for iOS
    IPAD_SCREEN_ORIENTATIONS    UIInterfaceOrientationUnknown, UIInterfaceOrientationLandscapeLeft, UIInterfaceOrientationLandscapeRight
    TARGETED_DEVICE_FAMILY      2     # target iPad only
    DOCUMENT_EXTENSIONS         xml
    SEND_APPLE_EVENTS_PERMISSION_ENABLED FALSE
    )

set (Imogen_Plugin_Flags
	${Imogen_Common_Flags}
	IS_SYNTH                        FALSE
    IS_MIDI_EFFECT                  FALSE
    DISABLE_AAX_MULTI_MONO          TRUE
    VST_NUM_MIDI_INS                1
    VST_NUM_MIDI_OUTS               1
    VST3_CATEGORIES                 "Pitch Shift"
    AU_MAIN_TYPE                    "kAudioUnitType_MusicEffect"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    MICROPHONE_PERMISSION_TEXT      "Imogen requires audio input to be able to produce its output. Please enable the microphone, or you won't hear any audio output."
	BACKGROUND_AUDIO_ENABLED	    FALSE # for iOS
	)

set (Imogen_Processor_Source_Files
	${Imogen_sourceDir}/DSP/PluginProcessor/PluginProcessor.cpp
    ${Imogen_sourceDir}/DSP/PluginProcessor/PluginProcessor.h)

set (Imogen_Processor_Dependencies ImogenCommon ImogenEngine)



#################### Configure the main build of Imogen ####################

set (Imogen_sourceFiles
    ${Imogen_Processor_Source_Files}
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.cpp
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.h
    ${Imogen_sourceDir}/ApplicationBases/Plugin_StandaloneWrapper/StandaloneWrapper.h
    ${Imogen_sourceDir}/ApplicationBases/Plugin_StandaloneWrapper/StandaloneWrapper.cpp)

juce_add_plugin (Imogen
    ${Imogen_Plugin_Flags}
    PLUGIN_NAME                 Imogen
    PLUGIN_MANUFACTURER_CODE    Benv
    PLUGIN_CODE                 Imgn
    FORMATS                     AU VST3 Standalone Unity
    DESCRIPTION                 "Real-time vocal harmonizer instrument"
    DOCUMENT_BROWSER_ENABLED    TRUE
    NEEDS_CURL                  TRUE
    NEEDS_WEB_BROWSER           TRUE
    )

configure_default_juce_plugin (Imogen)

target_sources (Imogen PRIVATE ${Imogen_sourceFiles})

source_group (TREE ${Imogen_sourceDir} PREFIX "" FILES ${Imogen_sourceFiles})

target_include_directories (Imogen PRIVATE ${Imogen_sourceDir})

link_binary_data_to_target_create_if_needed (Imogen assets)

target_link_libraries (Imogen PRIVATE ${Imogen_Processor_Dependencies} Ableton::Link ImogenGUI)

target_compile_definitions (Imogen PUBLIC JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1 IMOGEN_HEADLESS=0)



#################### Configure the headless plugin build ####################
if (NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS") OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android"))

juce_add_plugin (ImogenHeadless
    ${Imogen_Plugin_Flags}
    PLUGIN_NAME                 ImogenHeadless
    PLUGIN_MANUFACTURER_CODE    Benv
    PLUGIN_CODE                 Imnh
    FORMATS                     AU VST3
    DESCRIPTION                 "Headless version of the Imogen plugin"
    DOCUMENT_BROWSER_ENABLED    FALSE
    NEEDS_CURL                  FALSE
    NEEDS_WEB_BROWSER           FALSE
    )

_configure_juce_app (ImogenHeadless FALSE TRUE)

target_sources (ImogenHeadless PRIVATE ${Imogen_Processor_Source_Files})

target_include_directories (ImogenHeadless PRIVATE ${Imogen_sourceDir})

link_binary_data_to_target_create_if_needed (ImogenHeadless assets)

target_link_libraries (ImogenHeadless PRIVATE ${Imogen_Processor_Dependencies} Ableton::Link)

target_compile_definitions (ImogenHeadless PUBLIC JUCE_HEADLESS_PLUGIN_CLIENT=1 IMOGEN_HEADLESS=1)

endif()


#################### Configure the remote GUI app build ####################

set (ImogenRemote_sourceFiles
    ${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.h
    ${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.cpp
    ${Imogen_sourceDir}/ApplicationBases/Remote/Main.cpp)

juce_add_gui_app (ImogenRemote
    ${Imogen_Common_Flags}
    DESCRIPTION                   "Remote control for the Imogen plugin"
    DOCUMENT_BROWSER_ENABLED      TRUE
    NEEDS_CURL                    TRUE
    NEEDS_WEB_BROWSER             TRUE
    MICROPHONE_PERMISSION_ENABLED FALSE
    BACKGROUND_AUDIO_ENABLED      TRUE # for iOS
    )

configure_default_juce_app (ImogenRemote)

target_sources (ImogenRemote PRIVATE ${ImogenRemote_sourceFiles})

source_group (TREE ${Imogen_sourceDir} PREFIX "" FILES ${ImogenRemote_sourceFiles})

target_include_directories (ImogenRemote PRIVATE ${Imogen_sourceDir})

link_binary_data_to_target_create_if_needed (ImogenRemote assets)

target_link_libraries (ImogenRemote PRIVATE ImogenCommon ImogenGUI)


