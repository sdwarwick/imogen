cmake_minimum_required (VERSION 3.19) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX) 

add_subdirectory (scripts)

set (Imogen_Common_Flags
    ${BV_DEFAULT_BRAND_FLAGS}
    PRODUCT_NAME                ${PROJECT_NAME}
    VERSION                     ${CMAKE_PROJECT_VERSION}
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    ICON_BIG                    ${CMAKE_CURRENT_LIST_DIR}/assets/graphics/imogen_icon.png
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
    PLUGIN_CODE                     Imgn
    FORMATS                         ${BV_PLUGIN_FORMATS}
    DESCRIPTION                     "Real-time vocal harmonizer instrument"
    DOCUMENT_BROWSER_ENABLED        TRUE
    NEEDS_CURL                      TRUE
    NEEDS_WEB_BROWSER               TRUE
    )

bv_configure_juce_plugin (TARGET Imogen BROWSER MTS-ESP)

target_sources (Imogen PRIVATE ${BV_PROJECT_SOURCE_DIR}/plugin_main.cpp)

target_include_directories (Imogen PRIVATE ${BV_PROJECT_SOURCE_DIR})

bv_add_resources_folder (TARGET Imogen FOLDER assets)

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

bv_configure_juce_app (TARGET ImogenRemote BROWSER MTS-ESP)

target_sources (ImogenRemote PRIVATE ${BV_PROJECT_SOURCE_DIR}/remote_main.cpp)

target_include_directories (ImogenRemote PRIVATE ${BV_PROJECT_SOURCE_DIR})

bv_add_resources_folder (TARGET ImogenRemote FOLDER assets)

target_link_libraries (ImogenRemote PRIVATE imogen_gui bv_app_utils)

