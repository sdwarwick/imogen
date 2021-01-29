# IMOGEN 

# This CMake script will generate an IDE project containing all the Imogen source code files, as well as JUCE build targets for VST and AudioUnit plugins.


cmake_minimum_required (VERSION 3.15) 

set (CMAKE_CXX_COMPILER AppleClang)

project (Imogen VERSION 0.0.1 LANGUAGES CXX)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

# include (CTest)


set (juce_directory ${CMAKE_CURRENT_LIST_DIR}/Source/JUCE)  # the subdirectory containing the JUCE library code. 

add_subdirectory (${juce_directory})  # Juce can also be included using find_package, but that is more complex and I couldn't get it to work, so I just copied the library code into a subdirectory of my Source folder. Works for now.


set_directory_properties (PROPERTIES 
    JUCE_COMPANY_NAME             BenViningMusicSoftware
    JUCE_COMPANY_WEBSITE          www.benvining.com
    JUCE_COMPANY_EMAIL            ben.the.vining@gmail.com
    JUCE_COPY_PLUGIN_AFTER_BUILD  TRUE
    JUCE_VST3_COPY_DIR            /Users/benvining/Library/Audio/Plug-Ins/VST/MyPlugins
    JUCE_AU_COPY_DIR              /Users/benvining/Library/Audio/Plug-Ins/Components
    JUCE_COMPANY_COPYRIGHT        This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining.
    )


juce_add_plugin (${CMAKE_PROJECT_NAME} # creates build targets for each plugin format specified, and a "BUILD_ALL" target
    PRODUCT_NAME ${CMAKE_PROJECT_NAME}
    VERSION ${CMAKE_PROJECT_VERSION}
    BUNDLE_ID com.BenViningMusicSoftware.Imogen
    
    PLUGIN_NAME ${CMAKE_PROJECT_NAME}
    PLUGIN_MANUFACTURER_CODE Benv
    DESCRIPTION Real-time vocal harmonizer instrument
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    PLUGIN_CODE Imgn
    FORMATS AU VST3    
    VST3_CATEGORIES Pitch Shift
    AU_MAIN_TYPE kAudioUnitType_MusicEffect
    )

juce_generate_juce_header (${CMAKE_PROJECT_NAME})


target_compile_features (${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)

target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    )


#####

# DSP SOURCE FILES

set (dspSourcePath ${CMAKE_CURRENT_LIST_DIR}/Source/DSP)

set (dspSources
    ${dspSourcePath}/ImogenEngine.cpp
    ${dspSourcePath}/ImogenEngine.h
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
    PREFIX "DSP/Harmonizer"
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
    PREFIX "DSP/Harmonizer/GrainExtractor"
    FILES ${GrainExtractorSources})

# panning manager 

set (PanningManagerSourcePath ${HarmonizerSourcePath}/PanningManager)

set (PanningManagerSources
    ${PanningManagerSourcePath}/PanningManager.cpp
    ${PanningManagerSourcePath}/PanningManager.h)

source_group (
    TREE ${PanningManagerSourcePath}
    PREFIX "DSP/Harmonizer/PanningManager"
    FILES ${PanningManagerSources})

#

# Pitch detector source files 

set (PitchDetectorSourcePath ${dspSourcePath}/PitchDetector)

set (PitchDetectorSources
    ${PitchDetectorSourcePath}/PitchDetector.cpp
    ${PitchDetectorSourcePath}/PitchDetector.h)

source_group (
    TREE ${PitchDetectorSourcePath}
    PREFIX "DSP/PitchDetector"
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
    PREFIX "DSP/Utils"
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
    PREFIX "GUI/HelpScreen"
    FILES ${HelpScreenSources})


# IO control panel

set (IOPanelSourcePath ${guiSourcePath}/IOControlPanel)

set (IOControlSources
    ${IOPanelSourcePath}/IOControlPanel.cpp
    ${IOPanelSourcePath}/IOControlPanel.h
    ${IOPanelSourcePath}/LimiterControlPanel.h)

source_group (
    TREE ${IOPanelSourcePath}
    PREFIX "GUI/IOControlPanel"
    FILES ${IOControlSources})



# Midi control panel

set (MidiControlSourcePath ${guiSourcePath}/MidiControlPanel)

set (MidiControlSources
    ${MidiControlSourcePath}/MidiControlPanel.cpp
    ${MidiControlSourcePath}/MidiControlPanel.h)

source_group (
    TREE ${MidiControlSourcePath}
    PREFIX "GUI/MidiControlPanel"
    FILES ${MidiControlSources})



# Staff display

set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (StaffDisplaySources
    ${StaffDisplaySourcePath}/StaffDisplay.cpp
    ${StaffDisplaySourcePath}/StaffDisplay.h)

source_group (
    TREE ${StaffDisplaySourcePath}
    PREFIX "GUI/StaffDisplay"
    FILES ${StaffDisplaySources})



#####

set (ImogenSourceCode   # this struct defines the collection of all non-juce or c++::std source code files that are used by Imogen.

    # DSP files:
    ${dspSources}
    ${PitchDetectorSources}
    ${HarmonizerSources}
    ${GrainExtractorSources}
    ${PanningManagerSources}
    ${UtilSources}
    
    # GUI files:
    ${guiSources}
    ${HelpScreenSources}
    ${IOControlSources}
    ${MidiControlSources}
    ${StaffDisplaySources}
    )


target_sources (${CMAKE_PROJECT_NAME} PRIVATE ${ImogenSourceCode})


target_include_directories (${CMAKE_PROJECT_NAME} PRIVATE ${dspSourcePath} ${guiSourcePath})


#####

# graphic asset binary files 

set (GraphicAssetsDir ${guiSourcePath}/GraphicAssets)

juce_add_binary_data (ImogenGraphicAssets SOURCES 
    ${GraphicAssetsDir}/1-1_note_semibreve.svg 
    ${GraphicAssetsDir}/closeIcon.png 
    ${GraphicAssetsDir}/grandStaff.png)


#####


target_link_libraries (${CMAKE_PROJECT_NAME}
    INTERFACE
        ImogenGraphicAssets
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



#####

# Test executable

# set (testFilesPath ${CMAKE_CURRENT_LIST_DIR}/Source/Tests) 

# set (testFiles ${testFilesPath}/tests.cpp ${testFilesPath}/tests.h) 

# source_group (
#    TREE ${testFilesPath}
#    PREFIX "Tests"
#    FILES ${testFiles})

# add_executable (tests ${testFiles} ${ImogenSourceCode})

# target_include_directories (tests PRIVATE ${dspSourcePath} ${guiSourcePath} ${testFilesPath})

# target_compile_features (tests PRIVATE cxx_std_17)

# juce_generate_juce_header (tests)

# target_compile_definitions (tests PUBLIC
#    JUCE_WEB_BROWSER=0
#    JUCE_USE_CURL=0
#    JUCE_STRICT_REFCOUNTEDPTR=1
#    JUCE_VST3_CAN_REPLACE_VST2=0
#    )


# target_link_libraries (tests
#    PRIVATE
#        ImogenGraphicAssets
#        juce::juce_audio_utils
#        juce::juce_audio_plugin_client
#        juce::juce_dsp
#        juce::juce_graphics)


# add_test (NAME tests COMMAND tests)

# enable_testing()
