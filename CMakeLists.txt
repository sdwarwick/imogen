# IMOGEN 

cmake_minimum_required (VERSION 3.15) 

project (Imogen VERSION 0.0.1)

set_property (GLOBAL PROPERTY USE_FOLDERS YES)

add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/Source/JUCE) # the subdirectory containing the JUCE library code 
 

juce_add_plugin (${CMAKE_PROJECT_NAME}
    COMPANY_NAME "BenViningMusicSoftware"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE BenV
    PLUGIN_NAME ${CMAKE_PROJECT_NAME}
    PLUGIN_CODE Imgn
    FORMATS AU VST3
    PRODUCT_NAME "${CMAKE_PROJECT_NAME}"
    DISABLE_AAX_MULTI_MONO TRUE
    VST3_CATEGORIES Pitch Shift
    AU_MAIN_TYPE kAudioUnitType_MusicEffect
    AAX_CATEGORY AAX_ePlugInCategory_PitchShift
    )


target_compile_features (${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)


juce_generate_juce_header (${CMAKE_PROJECT_NAME})


target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    )


#####

# graphic asset binary files 

set (GraphicAssetsDir ${CMAKE_CURRENT_LIST_DIR}/Source/GUI/GraphicAssets)

juce_add_binary_data (ImogenGraphicAssets SOURCES 
    ${GraphicAssetsDir}/1-1_note_semibreve.svg 
    ${GraphicAssetsDir}/closeIcon.png 
    ${GraphicAssetsDir}/grandStaff.png)


#####

# DSP SOURCE FILES

set (dspSourcePath ${CMAKE_CURRENT_LIST_DIR}/Source/DSP)

set (dspSources
    ${dspSourcePath}/ImogenEngine.cpp
    ${dspSourcePath}/PluginProcessor.cpp
    ${dspSourcePath}/PluginProcessor.h)

source_group (
    TREE ${dspSourcePath}
    PREFIX "DSP"
    FILES ${dspSources})

#

# Harmonizer source files

set (HarmonizerSourcePath ${dspSourcePath}/HarmonizerDSP)

set (HarmonizerSources
    ${HarmonizerSourcePath}/HarmonizerVoice.cpp
    ${HarmonizerSourcePath}/Harmonizer.cpp
    ${HarmonizerSourcePath}/HarmonizerMidi.cpp
    ${HarmonizerSourcePath}/Harmonizer.h
    ${HarmonizerSourcePath}/HarmonizerUtilities.h)

source_group (
    TREE ${HarmonizerSourcePath}
    PREFIX "Harmonizer"
    FILES ${HarmonizerSources})

# grain extractor

set (GrainExtractorSourcePath ${HarmonizerSourcePath}/GrainExtractor)

set (GrainExtractorSources
    ${GrainExtractorSourcePath}/ZeroCrossingFinding.cpp
    ${GrainExtractorSourcePath}/PsolaPeakFinding.cpp
    ${GrainExtractorSourcePath}/GrainExtractor.cpp
    ${GrainExtractorSourcePath}/GrainExtractor.h)

source_group (
    TREE ${GrainExtractorSourcePath}
    PREFIX "GrainExtractor"
    FILES ${GrainExtractorSources})

# panning manager 

set (PanningManagerSourcePath ${HarmonizerSourcePath}/PanningManager)

set (PanningManagerSources
    ${PanningManagerSourcePath}/PanningManager.cpp
    ${PanningManagerSourcePath}/PanningManager.h)

source_group (
    TREE ${PanningManagerSourcePath}
    PREFIX "PanningManager"
    FILES ${PanningManagerSources})

#

# Pitch detector source files 

set (PitchDetectorSourcePath ${dspSourcePath}/PitchDetector)

set (PitchDetectorSources
    ${PitchDetectorSourcePath}/PitchDetector.cpp
    ${PitchDetectorSourcePath}/PitchDetector.h)

source_group (
    TREE ${PitchDetectorSourcePath}
    PREFIX "PitchDetector"
    FILES ${PitchDetectorSources})

#

# Utility source files 

set (UtilSourcePath ${dspSourcePath}/Utils)

set (UtilSources
    ${UtilSourcePath}/DelayBuffer.h
    ${UtilSourcePath}/FancyMidiBuffer.h
    ${UtilSourcePath}/GeneralUtils.h
    ${UtilSourcePath}/Panner.h)

source_group (
    TREE ${UtilSourcePath}
    PREFIX "Utils"
    FILES ${UtilSources})



#####

# GUI SOURCE FILES

set (guiSourcePath ${CMAKE_CURRENT_LIST_DIR}/Source/GUI)

set (guiSources
    ${guiSourcePath}/LookAndFeel.h
    ${guiSourcePath}/PluginEditor.cpp
    ${guiSourcePath}/PluginEditor.h
    ${guiSourcePath}/EnableSidechainWarning.h)

source_group (
    TREE ${guiSourcePath}
    PREFIX "GUI"
    FILES ${guiSources})


# help screen

set (HelpScreenSourcePath ${guiSourcePath}/HelpScreen)

set (HelpScreenSources
    ${HelpScreenSourcePath}/HelpScreen.cpp
    ${HelpScreenSourcePath}/HelpScreen.h)

source_group (
    TREE ${HelpScreenSourcePath}
    PREFIX "HelpScreen"
    FILES ${HelpScreenSources})


# IO control panel

set (IOPanelSourcePath ${guiSourcePath}/IOControlPanel)

set (IOControlSources
    ${IOPanelSourcePath}/IOControlPanel.cpp
    ${IOPanelSourcePath}/IOControlPanel.h
    ${IOPanelSourcePath}/LimiterControlPanel.h)

source_group (
    TREE ${IOPanelSourcePath}
    PREFIX "IOControlPanel"
    FILES ${IOControlSources})


# Midi control panel

set (MidiControlSourcePath ${guiSourcePath}/MidiControlPanel)

set (MidiControlSources
    ${MidiControlSourcePath}/MidiControlPanel.cpp
    ${MidiControlSourcePath}/MidiControlPanel.h)

source_group (
    TREE ${MidiControlSourcePath}
    PREFIX "MidiControlPanel"
    FILES ${MidiControlSources})


# Staff display

set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (StaffDisplaySources
    ${StaffDisplaySourcePath}/StaffDisplay.cpp
    ${StaffDisplaySourcePath}/StaffDisplay.h)

source_group (
    TREE ${StaffDisplaySourcePath}
    PREFIX "StaffDisplay"
    FILES ${StaffDisplaySources})


#####

target_sources (${CMAKE_PROJECT_NAME} PRIVATE 

    # DSP files:
    ${GrainExtractorSources}
    ${PanningManagerSources}
    ${UtilSources}
    ${HarmonizerSources}
    ${PitchDetectorSources}
    ${dspSources}

    # GUI files:
    ${HelpScreenSources}
    ${IOControlSources}
    ${MidiControlSources}
    ${StaffDisplaySources}
    ${guiSources}
    )


target_include_directories (${CMAKE_PROJECT_NAME} PRIVATE 
    ${dspSourcePath}
    ${HarmonizerSourcePath}
    ${GrainExtractorSourcePath}
    ${PanningManagerSourcePath}
    ${PitchDetectorSourcePath}
    ${UtilSourcePath}
    ${guiSourcePath}
    ${HelpScreenSourcePath}
    ${IOPanelSourcePath}
    ${MidiControlSourcePath}
    ${StaffDisplaySourcePath}
    )


target_link_libraries (${CMAKE_PROJECT_NAME}
    PRIVATE
        ImogenGraphicAssets
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_dsp
        juce::juce_graphics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )