#  IMOGEN 

#  AUTHOR: BEN VINING


cmake_minimum_required (VERSION 3.18) 

project (Imogen VERSION 0.0.1 LANGUAGES CXX) 

#

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)  # This could conceivably be in a different place, but the source tree must be intact 

set (pluginSourcesDir       ${Imogen_sourceDir}/PluginSources)  # The rest of the source tree (child folders of the main sourceDir specified above)
set (guiSourcePath          ${Imogen_sourceDir}/GUI)
set (HelpScreenSourcePath   ${guiSourcePath}/HelpScreen)
set (IOPanelSourcePath      ${guiSourcePath}/IOControlPanel)
set (MidiControlSourcePath  ${guiSourcePath}/MidiControlPanel)
set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (Imogen_graphicAssetsDir ${guiSourcePath}/GraphicAssets)  # The location of the graphical asset files (images, etc)

set (Imogen_testFilesPath ${Imogen_sourceDir}/Tests)  # The location of the source files in which unit tests are defined


#

# the set of custom JUCE-style modules needed for this project

set (Imogen_customModulesPath ${Imogen_sourceDir}/DSP_modules)

set (Imogen_customModulesNeeded
    bv_GeneralUtils
    bv_PitchDetector
    bv_Harmonizer
    bv_ImogenEngine)


# the groups of actual source files 

set (Imogen_sourceFiles
    ${pluginSourcesDir}/PluginProcessor.cpp
    ${pluginSourcesDir}/PluginProcessor.h
    ${pluginSourcesDir}/PluginEditor.cpp
    ${pluginSourcesDir}/PluginEditor.h
    ${guiSourcePath}/LookAndFeel.h
    ${guiSourcePath}/EnableSidechainWarning.h
    ${HelpScreenSourcePath}/HelpScreen.cpp
    ${HelpScreenSourcePath}/HelpScreen.h
    ${IOPanelSourcePath}/IOControlPanel.cpp  
    ${IOPanelSourcePath}/IOControlPanel.h
    ${IOPanelSourcePath}/LimiterControlPanel.h
    ${MidiControlSourcePath}/MidiControlPanel.cpp
    ${MidiControlSourcePath}/MidiControlPanel.h
    ${StaffDisplaySourcePath}/StaffDisplay.cpp
    ${StaffDisplaySourcePath}/StaffDisplay.h)

set (Imogen_graphicAssetFiles 
    ${Imogen_graphicAssetsDir}/1-1_note_semibreve.svg 
    ${Imogen_graphicAssetsDir}/closeIcon.png 
    ${Imogen_graphicAssetsDir}/grandStaff.png 
    ${CMAKE_CURRENT_SOURCE_DIR}/imogen_icon.png)

set (Imogen_testFiles
    ${Imogen_testFilesPath}/tests.cpp
    ${Imogen_testFilesPath}/HarmonizerTests.cpp) 

#

include (JUCE-CmakeWrapper/CMakeLists.txt)

#

