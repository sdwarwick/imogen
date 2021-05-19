
cmake_minimum_required (VERSION 3.18) 

include (CMake/FindShared-code.cmake)

project (Imogen VERSION 0.0.1 LANGUAGES CXX) 

######

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)

set (ImogenIconPath ${Imogen_sourceDir}/../assets/graphics/imogen_icon.png)

#

set (Imogen_Common_Flags
    PRODUCT_NAME                ${CMAKE_PROJECT_NAME}
    VERSION                     ${CMAKE_PROJECT_VERSION}
    BUNDLE_ID                   com.BenViningMusicSoftware.Imogen
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    ICON_BIG                    ${ImogenIconPath}
    COMPANY_NAME                BenViningMusicSoftware
    COMPANY_WEBSITE             www.benvining.com
    COMPANY_EMAIL               ben.the.vining@gmail.com
    COMPANY_COPYRIGHT           "This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License."
    STATUS_BAR_HIDDEN           TRUE  # for iOS
    REQUIRES_FULL_SCREEN        TRUE  # for iOS
    IPAD_SCREEN_ORIENTATIONS    UIInterfaceOrientationUnknown, UIInterfaceOrientationLandscapeLeft, UIInterfaceOrientationLandscapeRight
    TARGETED_DEVICE_FAMILY      2     # target iPad only
    DOCUMENT_EXTENSIONS         xml
    SEND_APPLE_EVENTS_PERMISSION_ENABLED FALSE
    )


####################


set (Imogen_sourceFiles
    ${Imogen_sourceDir}/DSP/PluginProcessor/PluginProcessor.cpp
    ${Imogen_sourceDir}/DSP/PluginProcessor/PluginProcessorParameters.cpp
    ${Imogen_sourceDir}/DSP/PluginProcessor/PluginProcessorState.cpp
    ${Imogen_sourceDir}/DSP/PluginProcessor/PluginProcessor.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.cpp
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_StandaloneWrapper/StandaloneWrapper.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_StandaloneWrapper/StandaloneWrapper.cpp)

#

juce_add_modules ("${Imogen_sourceDir}/ImogenCommon"
				  "${Imogen_sourceDir}/DSP/Harmonizer" 
				  "${Imogen_sourceDir}/DSP/ImogenEngine"
				  "${Imogen_sourceDir}/GUI/ImogenGUI")

juce_add_plugin (Imogen
    ${Imogen_Common_Flags}
    PLUGIN_NAME                     Imogen
    PLUGIN_MANUFACTURER_CODE        Benv
    DESCRIPTION                     "Real-time vocal harmonizer instrument"
    IS_SYNTH                        FALSE
    IS_MIDI_EFFECT                  FALSE
    DISABLE_AAX_MULTI_MONO          TRUE
    VST_NUM_MIDI_INS                1
    VST_NUM_MIDI_OUTS               1
    VST3_CATEGORIES                 "Pitch Shift"
    AU_MAIN_TYPE                    "kAudioUnitType_MusicEffect"
    FORMATS                         AU VST3 Standalone Unity
    MICROPHONE_PERMISSION_ENABLED   TRUE
    MICROPHONE_PERMISSION_TEXT      "Imogen requires audio input to be able to produce its output. Please enable the microphone, or you won't hear anything when you press the keys."
    PLUGIN_CODE                 Imgn
    DOCUMENT_BROWSER_ENABLED    TRUE
    BACKGROUND_AUDIO_ENABLED	FALSE # for iOS
    NEEDS_CURL                  TRUE
    NEEDS_WEB_BROWSER           TRUE
    )

set (bvsb_USE_MTS_ESP TRUE)
set_default_juce_options (Imogen)

#

if (TARGET Imogen_LV2)
    set_target_properties (Imogen PROPERTIES JUCE_LV2_URI https://github.com/benthevining/imogen)
endif()

#

target_sources (Imogen PRIVATE ${Imogen_sourceFiles})

source_group (TREE ${Imogen_sourceDir} PREFIX "" FILES ${Imogen_sourceFiles})

target_include_directories (Imogen PRIVATE ${Imogen_sourceDir})

add_binary_data_folder (Imogen assets)

include (${CMAKE_CURRENT_LIST_DIR}/third-party/ableton-link/AbletonLinkConfig.cmake)

target_link_libraries (Imogen PRIVATE Ableton::Link ImogenCommon Harmonizer ImogenEngine ImogenGUI)

target_compile_definitions (Imogen PUBLIC JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1)


####################


set (ImogenRemote_sourceFiles
    ${Imogen_Common_SourceFiles}
    ${Imogen_GUI_SourceFiles}
    ${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.h
    ${Imogen_sourceDir}/GUI/Holders/Remote_MainComponent/MainComponent.cpp
    ${Imogen_sourceDir}/Remote/Main.cpp)

juce_add_gui_app (ImogenRemote
    ${Imogen_Common_Flags}
    DESCRIPTION                   "Remote control for the Imogen plugin"
    DOCUMENT_BROWSER_ENABLED      TRUE
    MICROPHONE_PERMISSION_ENABLED FALSE
    BACKGROUND_AUDIO_ENABLED      TRUE # for iOS
    NEEDS_CURL                    TRUE
    NEEDS_WEB_BROWSER             TRUE
    )

set (bvsb_USE_MTS_ESP FALSE)
set_default_juce_options (ImogenRemote)

#

target_sources (ImogenRemote PRIVATE ${ImogenRemote_sourceFiles})

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR}/.. PREFIX "" FILES ${ImogenRemote_sourceFiles})

target_include_directories (ImogenRemote PRIVATE ${Imogen_sourceDir})

add_binary_data_folder (ImogenRemote assets)

target_link_libraries (ImogenRemote PRIVATE ImogenCommon ImogenGUI)

#

