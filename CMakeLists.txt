
cmake_minimum_required (VERSION 3.18) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX) 

set (BV_USE_MTS_ESP TRUE)
#set (BV_USE_ABLETON_LINK TRUE)

include (scripts/get_shared_code.cmake)

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)

bv_add_modules (${Imogen_sourceDir}/modules)

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


#################### Configure the main build of Imogen ####################

juce_add_plugin (Imogen
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
    BACKGROUND_AUDIO_ENABLED        FALSE # for iOS
    PLUGIN_NAME                     Imogen
    PLUGIN_MANUFACTURER_CODE        Benv
    PLUGIN_CODE                     Imgn
    FORMATS                         AU VST3 Standalone Unity
    DESCRIPTION                     "Real-time vocal harmonizer instrument"
    DOCUMENT_BROWSER_ENABLED        TRUE
    NEEDS_CURL                      TRUE
    NEEDS_WEB_BROWSER               TRUE
    )

configure_default_juce_plugin (Imogen)

target_sources (Imogen PRIVATE ${Imogen_sourceDir}/plugin/Main.cpp)

target_include_directories (Imogen PRIVATE ${Imogen_sourceDir})

add_binary_data_folder (Imogen assets)

target_link_libraries (Imogen PRIVATE imogen_dsp imogen_gui)


#################### Configure the remote GUI app build ####################

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

target_sources (ImogenRemote PRIVATE ${Imogen_sourceDir}/remote_app/Main.cpp)

target_include_directories (ImogenRemote PRIVATE ${Imogen_sourceDir})

add_binary_data_folder (ImogenRemote assets)

target_link_libraries (ImogenRemote PRIVATE imogen_gui bv_app_utils)


