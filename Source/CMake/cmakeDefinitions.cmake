
# various useful definitions & declarations for Imogen


# descriptive strings for cache variables

	set (ds_testing "Enable Imogen unit tests")

	set (ds_juceDir "Path to the JUCE library code")

	set (ds_catchDir "Path to the Catch2 code")

	set (ds_launchAPH "Automatically launch the JUCE AudioPluginHost")

	set (ds_launchSAL "Automatically launch the Imogen standalone")

	set (ds_runPiv "Automatically run pluginval on build")

	set (ds_preferSALforAll "Use the Imogen standalone for the All build target's executable")

	set (ds_SALpath "Path to the Imogen standalone executable file")

	set (ds_APHpath "Path to the JUCE AudioPluginHost executable")

	set (ds_pivPath "Path to the pluginval executable")

# general settings

set (imogen_juceDir  ${CMAKE_CURRENT_SOURCE_DIR}/Source/JUCE             CACHE FILEPATH "${ds_juceDir}")  # if this subdirectory isn't found, this script will automatically download the JUCE library code from GitHub
set (imogen_catchDir ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/catch2-src CACHE FILEPATH "${ds_catchDir}") # if this subdirectory isn't found and unit testing is enabled, this script will automatically download Catch2 from GitHub

set (imogen_standalone_exec_path "" CACHE FILEPATH "${ds_SALpath}")  # path to the Imogen standalone executable, if it exists 
set (imogen_AudioPluginHost_Path "" CACHE FILEPATH "${ds_APHpath}")  # path to the JUCE AudioPluginHost executable, if it exists
set (imogen_pluginval_path "" CACHE FILEPATH "${ds_pivPath}")  # path to the pluginval executable, if it exists 

set (imogen_compileFeatures "cxx_std_17" CACHE STRING "Build compile features")

set (allow_build_APH TRUE)  # set this to false to prevent this script from attempting to build the AudioPluginHost, if necessary 
set (APH_build_format Debug)  # if the APH is built, this variable can be set to Debug or Release to control which mode the APH is built in.

set (fetchcontentincluded FALSE)  # simple include guard for the 'FetchContent' package

set_property (GLOBAL PROPERTY USE_FOLDERS YES)
set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Build Targets")

set (CMAKE_OSX_ARCHITECTURES "arm64" "x86_64" CACHE STRING "Universal MacOS binaries" FORCE)  # universal macOS binaries 
set (CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version" FORCE)  # minimum MacOS version to build for

set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # static linking on Windows
set (CMAKE_WIN32_EXECUTABLE TRUE)

set (CMAKE_XCODE_GENERATE_SCHEME OFF)  # schemes are maually generated for each target to avoid clutter from modules getting schemes, etc
set (CMAKE_XCODE_SCHEME_THREAD_SANITIZER OFF)
set (CMAKE_XCODE_SCHEME_UNDEFINED_BEHAVIOR_SANITIZER ON)

set (CMAKE_SUPPRESS_REGENERATION TRUE)  # no "zero-check" target
set (CMAKE_OPTIMIZE_DEPENDENCIES TRUE)
set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option (JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)
option (JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)

#

# various child directories of the source tree

set (dspModulesPath ${sourceDir}/DSP_modules)  # The location of the custom JUCE-style modules that make up the shared Imogen codebase. Again, these could conceivably be in a different place...  ¯\_(ツ)_/¯

set (pluginSourcesDir       ${sourceDir}/PluginSources)  # The rest of the source tree (child folders of the main sourceDir specified above)
set (guiSourcePath          ${sourceDir}/GUI)
set (HelpScreenSourcePath   ${guiSourcePath}/HelpScreen)
set (IOPanelSourcePath      ${guiSourcePath}/IOControlPanel)
set (MidiControlSourcePath  ${guiSourcePath}/MidiControlPanel)
set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

set (graphicAssetsDir ${guiSourcePath}/GraphicAssets)  # The location of the graphical asset files (images, etc)

set (testFilesPath ${sourceDir}/Tests)  # The location of the source files in which unit tests are defined


#

# the set of custom JUCE-style modules needed for this project

set (customModulesNeeded
	${dspModulesPath}/bv_GeneralUtils
	${dspModulesPath}/bv_PitchDetector
	${dspModulesPath}/bv_Harmonizer
	${dspModulesPath}/bv_ImogenEngine)


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

set (graphicAssetFiles 
	${graphicAssetsDir}/1-1_note_semibreve.svg 
	${graphicAssetsDir}/closeIcon.png 
	${graphicAssetsDir}/grandStaff.png 
	${CMAKE_CURRENT_SOURCE_DIR}/imogen_icon.png)

set (testFiles
	${testFilesPath}/tests.cpp
	${testFilesPath}/HarmonizerTests.cpp) 

#

if (APPLE)
	set (_imgn_xtn ".app")
elseif (UNIX)
	set (_imgn_xtn "")
elseif (WIN32)
	set (_imgn_xtn ".exe")
endif()

#


