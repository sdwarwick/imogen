
#

if (DEFINED bv_ignoreAllThirdPartyLibs)
    set (bv_use_AbletonLink FALSE)
    set (bv_use_MTS_ESP FALSE)
else()
    if (NOT DEFINED bv_use_AbletonLink)
       set (bv_use_AbletonLink TRUE)
    endif()

    if (NOT DEFINED bv_use_MTS_ESP)
       set (bv_use_MTS_ESP TRUE)
    endif()
endif()

#

if (DEFINED bv_formats)
    set (lv2detector -1)
    list (FIND bv_formats LV2 lv2detector)

    if (NOT ${lv2detector} EQUAL -1)
        set_source_files_properties (${bv_juceDir}/extras/Build/lv2_ttl_generator/lv2_ttl_generator.c PROPERTIES LANGUAGE CXX)
        set (bv_use_AbletonLink FALSE)
    endif()
else()
    set (bv_formats AU VST3 Standalone Unity)
endif()

#


set (ImogenIconPath ${Imogen_sourceDir}/../assets/graphics/imogen_icon.png)

set (Imogen_Common_SourceFiles
    ${Imogen_sourceDir}/Common/ImogenCommon.h
    ${Imogen_sourceDir}/Common/ImogenParameters.h
    )

set (Imogen_Processor_SourceFiles
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessor.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessorParameters.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessorState.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessor.h)

set (Imogen_GUI_SourceFiles
    ${Imogen_sourceDir}/GUI/ImogenGUI.h
    ${Imogen_sourceDir}/GUI/ImogenGUI.cpp
    ${Imogen_sourceDir}/GUI/MainDialComponent/MainDialComponent.h
    ${Imogen_sourceDir}/GUI/MainDialComponent/MainDialComponent.cpp
    ${Imogen_sourceDir}/GUI/LookAndFeel/ImogenLookAndFeel.h
    ${Imogen_sourceDir}/GUI/LookAndFeel/ImogenLookAndFeel.cpp)

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

set (Imogen_Plugin_Flags
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
    FORMATS                         ${bv_formats}
    MICROPHONE_PERMISSION_ENABLED   TRUE
    MICROPHONE_PERMISSION_TEXT      "Imogen requires audio input to be able to produce its output. Please enable the microphone, or you won't hear anything when you press the keys."
    )

##################################################################################################

function (imogenConfig_step2)

if (DEFINED BV_IGNORE_VDSP OR NOT APPLE)

    target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC JUCE_USE_VDSP_FRAMEWORK=0 BV_USE_VDSP=0)
    
    if (DEFINED BV_IGNORE_MIPP)
        target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC BV_USE_MIPP=0)
    else()
        message (STATUS "Configuring MIPP...")
		target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC MIPP_ENABLE_BACKTRACE BV_USE_MIPP=1)
    	target_include_directories (${CMAKE_PROJECT_NAME} PUBLIC "${Imogen_sourceDir}/../third-party/MIPP/src" "MIPP")
    endif()
else()
    target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC BV_USE_VDSP=1 JUCE_USE_VDSP_FRAMEWORK=1)
endif()

#

target_compile_features (${CMAKE_PROJECT_NAME} PUBLIC cxx_std_17)

target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC 
    BV_HAS_BINARY_DATA=1
    )

endfunction()

