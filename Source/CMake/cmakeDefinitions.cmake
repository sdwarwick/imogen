
# various useful definitions & declarations for Imogen


# general settings

set (formats "AU" "VST3" "Standalone")  # valid formats: Standalone Unity VST3 AU AUv3 AAX VST. For AAX or VST, the path to the respective SDK must be provided using juce_set_aax_sdk_path or juce_set_vst2_sdk_path

set (compileFeatures cxx_std_17)

#

# descriptive strings for cache variables

set (ds_testing "Enable Imogen unit tests")

set (ds_juceDir "Path to the JUCE library code")

set (ds_catchDir "Path to the Catch2 code")

set (ds_launchAPH "Automatically launch the JUCE AudioPluginHost")

set (ds_launchSAL "Automatically launch the Imogen standalone")

set (ds_preferSALforAll "Use the Imogen standalone for the All build target's executable")

set (ds_SALpath "Path to the Imogen standalone executable file")

set (ds_APHpath "Path to the JUCE AudioPluginHost executable")

#

# initializing all the cache variables -- none are forced here!

set (IMOGEN_unitTesting ON CACHE BOOL "${ds_testing}")  # set this to 'true' to enable a unit testing executable target

set (imogen_juceDir  ${CMAKE_CURRENT_SOURCE_DIR}/Source/JUCE             CACHE FILEPATH "${ds_juceDir}")  # if this subdirectory isn't found, this script will automatically download the JUCE library code from GitHub
set (imogen_catchDir ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/catch2-src CACHE FILEPATH "${ds_catchDir}")  # if this subdirectory isn't found and unit testing is enabled, this script will automatically download Catch2 from GitHub

set (IMOGEN_launchAudioPluginHostOnBuild OFF CACHE BOOL "${ds_launchAPH}")        #  when true, this automatically launches the AudioPluginHost on build for all targets except the Standalone, which uses itself as its own executable
set (IMOGEN_launchStandaloneOnBuild      OFF CACHE BOOL "${ds_launchSAL}")        #  if launchAudioPluginHostOnBuild is false and this argument is true, then the Standalone will launch automatically for the Standalone and All targets, and the individual plugin formats will have no executable
set (IMOGEN_preferStandaloneForAllTarget OFF CACHE BOOL "${ds_preferSALforAll}")  #  if both launchAudioPluginHostOnBuild and launchStandaloneOnBuild are true, then if this flag is set to true, the All target will use the Standalone as its executable instead of the AudioPluginHost. If this flag is false, the All target would default to using the AudioPluginHost.

set (imogen_standalone_exec_path "" CACHE FILEPATH "${ds_SALpath}")
set (imogen_AudioPluginHost_Path "" CACHE FILEPATH "${ds_APHpath}")

#

# various child directories of the source tree

set (dspModulesPath ${sourceDir}/DSP_modules)  # The location of the custom JUCE-style modules that make up the shared Imogen codebase. Again, these could conceivably be in a different place...  ¯\_(ツ)_/¯

set (pluginSourcesDir       ${sourceDir}/PluginSources)  # The rest of the source tree (child folders of the main sourceDir specified above)
set (guiSourcePath          ${sourceDir}/GUI)
set (HelpScreenSourcePath   ${guiSourcePath}/HelpScreen)
set (IOPanelSourcePath      ${guiSourcePath}/IOControlPanel)
set (MidiControlSourcePath  ${guiSourcePath}/MidiControlPanel)
set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (GraphicAssetsDir ${guiSourcePath}/GraphicAssets)  # The location of the graphical asset files (images, etc)

set (testFilesPath ${sourceDir}/Tests)  # The location of the source files in which unit tests are defined

#

# the groups of actual source files 

set (sourceFiles
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

if (IMOGEN_unitTesting)

	set (testFiles
		${testFilesPath}/tests.cpp
		${testFilesPath}/HarmonizerTests.cpp) 

endif()

#

# the set of custom JUCE-style modules needed for this project

set (customModulesNeeded
	${dspModulesPath}/bv_GeneralUtils
	${dspModulesPath}/bv_PitchDetector
	${dspModulesPath}/bv_Harmonizer
	${dspModulesPath}/bv_ImogenEngine)

#

set (fetchcontentincluded FALSE)  # simple include guard for the 'FetchContent' package

#



