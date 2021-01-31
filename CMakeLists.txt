# IMOGEN 

# This CMake script will generate an IDE project containing all the Imogen source code files, as well as the following build targets: a static library for the shared Imogen codebase; VST & AU plugins, and a standalone application.


cmake_minimum_required (VERSION 3.15) 

set (CMAKE_CXX_COMPILER AppleClang)

project (Imogen VERSION 0.0.1 LANGUAGES CXX)

set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set_property (GLOBAL PROPERTY USE_FOLDERS YES)

option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)


# include (CTest)


set (sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)


set (juce_directory ${sourceDir}/JUCE)  # the subdirectory containing the JUCE library code

add_subdirectory (${juce_directory})  # Juce can also be included using find_package, but that is more complex and I couldn't get it to work, so I just copied the library code into a subdirectory of my Source folder. Works for now.


set_directory_properties (PROPERTIES 
    JUCE_COMPANY_NAME             BenViningMusicSoftware
    JUCE_COMPANY_WEBSITE          www.benvining.com
    JUCE_COMPANY_EMAIL            ben.the.vining@gmail.com
    JUCE_COPY_PLUGIN_AFTER_BUILD  TRUE
    JUCE_VST3_COPY_DIR            /Users/benvining/Library/Audio/Plug-Ins/VST/MyPlugins
    JUCE_AU_COPY_DIR              /Users/benvining/Library/Audio/Plug-Ins/Components
    JUCE_COMPANY_COPYRIGHT        This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License.
    )


set (dspModulesPath ${sourceDir}/DSP_modules)

juce_add_modules (
    ${dspModulesPath}/bv_GeneralUtils
    ${dspModulesPath}/bv_PitchDetector
    ${dspModulesPath}/bv_Harmonizer
    ${dspModulesPath}/bv_ImogenEngine
    )


# this command adds a static library target with the shared plugin code (that can be linked to for the testing target down below), as well as build targets for VST & AU plugins and a standalone:

juce_add_plugin (${CMAKE_PROJECT_NAME} 
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
    FORMATS AU VST3 Standalone
    VST3_CATEGORIES Pitch Shift
    AU_MAIN_TYPE kAudioUnitType_MusicEffect
    ICON_BIG ${CMAKE_CURRENT_LIST_DIR}/imogen_icon.png
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

# PLUGIN SOURCE FILES

set (pluginSources
    ${sourceDir}/PluginProcessor.cpp
    ${sourceDir}/PluginProcessor.h
    ${sourceDir}/PluginEditor.cpp
    ${sourceDir}/PluginEditor.h
    )

source_group (
    TREE ${sourceDir}
    PREFIX "Plugin Sources"
    FILES ${pluginSources})


#####

# GUI SOURCE FILES

set (guiSourcePath ${sourceDir}/GUI)

set (guiSources
    ${guiSourcePath}/LookAndFeel.h
    ${guiSourcePath}/EnableSidechainWarning.h)

source_group (
    TREE ${guiSourcePath}
    PREFIX "GUI"
    FILES ${guiSources})


set (HelpScreenSourcePath ${guiSourcePath}/HelpScreen)

set (HelpScreenSources
    ${HelpScreenSourcePath}/HelpScreen.cpp
    ${HelpScreenSourcePath}/HelpScreen.h)

source_group (
    TREE ${HelpScreenSourcePath}
    PREFIX "GUI/HelpScreen"
    FILES ${HelpScreenSources})


set (IOPanelSourcePath ${guiSourcePath}/IOControlPanel)

set (IOControlSources
    ${IOPanelSourcePath}/IOControlPanel.cpp
    ${IOPanelSourcePath}/IOControlPanel.h
    ${IOPanelSourcePath}/LimiterControlPanel.h)

source_group (
    TREE ${IOPanelSourcePath}
    PREFIX "GUI/IOControlPanel"
    FILES ${IOControlSources})



set (MidiControlSourcePath ${guiSourcePath}/MidiControlPanel)

set (MidiControlSources
    ${MidiControlSourcePath}/MidiControlPanel.cpp
    ${MidiControlSourcePath}/MidiControlPanel.h)

source_group (
    TREE ${MidiControlSourcePath}
    PREFIX "GUI/MidiControlPanel"
    FILES ${MidiControlSources})



set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (StaffDisplaySources
    ${StaffDisplaySourcePath}/StaffDisplay.cpp
    ${StaffDisplaySourcePath}/StaffDisplay.h)

source_group (
    TREE ${StaffDisplaySourcePath}
    PREFIX "GUI/StaffDisplay"
    FILES ${StaffDisplaySources})



#####

target_sources (${CMAKE_PROJECT_NAME} PRIVATE 
    ${pluginSources} 
    ${guiSources}
    ${HelpScreenSources}
    ${IOControlSources}
    ${MidiControlSources}
    ${StaffDisplaySources}
    )


target_include_directories (${CMAKE_PROJECT_NAME} PRIVATE ${sourceDir} ${guiSourcePath})


#####


# this creates a static library target which JUCE will populate with Imogen's graphic assets as binary files, so it can then be linked to

set (GraphicAssetsDir ${guiSourcePath}/GraphicAssets)

juce_add_binary_data (ImogenGraphicAssets SOURCES 
    ${GraphicAssetsDir}/1-1_note_semibreve.svg 
    ${GraphicAssetsDir}/closeIcon.png 
    ${GraphicAssetsDir}/grandStaff.png
    )


#####


target_link_libraries (${CMAKE_PROJECT_NAME}
    PRIVATE
        ImogenGraphicAssets
        bv_ImogenEngine
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )



#####

# Test executable


# set (testFilesPath ${sourceDir}/Tests) 

# set (testFiles 
#     ${testFilesPath}/tests.cpp 
#     ${testFilesPath}/tests.h) 

# source_group (
#    TREE ${testFilesPath}
#    PREFIX "Tests"
#    FILES ${testFiles})


# add_executable (tests ${testFiles})

# target_include_directories (tests PRIVATE ${testFilesPath})

# target_compile_features (tests PRIVATE cxx_std_17)


# target_link_libraries (tests
#    PRIVATE
#        bv_ImogenEngine
#    PUBLIC
#        juce::juce_recommended_config_flags
#        juce::juce_recommended_lto_flags
#        juce::juce_recommended_warning_flags
#    )


# add_test (NAME tests COMMAND tests)

# enable_testing()
