
# various useful definitions & declarations for Imogen


# general settings

set (fetchcontentincluded FALSE)  # simple include guard for the 'FetchContent' package

set_property (GLOBAL PROPERTY USE_FOLDERS YES)
set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Build Targets")

set (CMAKE_OSX_ARCHITECTURES "arm64" "x86_64")  # universal macOS binaries 
set (CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version" FORCE)  # minimum MacOS version to build for

set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # static linking on Windows
set (CMAKE_WIN32_EXECUTABLE TRUE)

set (CMAKE_XCODE_GENERATE_SCHEME OFF)  # schemes are maually generated for each target to avoid clutter from modules getting schemes, etc
set (CMAKE_XCODE_SCHEME_THREAD_SANITIZER ON)
set (CMAKE_XCODE_SCHEME_UNDEFINED_BEHAVIOR_SANITIZER ON)

set (CMAKE_SUPPRESS_REGENERATION TRUE)  # no "zero-check" target
set (CMAKE_OPTIMIZE_DEPENDENCIES TRUE)
set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option (JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)
option (JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)

#

# various child directories of the source tree

set (dspModulesPath ${imogen_sourceDir}/DSP_modules)  # The location of the custom JUCE-style modules that make up the shared Imogen codebase. Again, these could conceivably be in a different place...  ¯\_(ツ)_/¯

set (pluginSourcesDir       ${imogen_sourceDir}/PluginSources)  # The rest of the source tree (child folders of the main sourceDir specified above)
set (guiSourcePath          ${imogen_sourceDir}/GUI)
set (HelpScreenSourcePath   ${guiSourcePath}/HelpScreen)
set (IOPanelSourcePath      ${guiSourcePath}/IOControlPanel)
set (MidiControlSourcePath  ${guiSourcePath}/MidiControlPanel)
set (StaffDisplaySourcePath ${guiSourcePath}/StaffDisplay)

if (IMOGEN_unitTesting)
	set (testFilesPath ${imogen_sourceDir}/Tests)  # The location of the source files in which unit tests are defined
endif()

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
	${imogen_GraphicAssetsDir}/1-1_note_semibreve.svg 
	${imogen_GraphicAssetsDir}/closeIcon.png 
	${imogen_GraphicAssetsDir}/grandStaff.png 
	${CMAKE_CURRENT_SOURCE_DIR}/imogen_icon.png)

if (IMOGEN_unitTesting)
	set (testFiles
		${testFilesPath}/tests.cpp
		${testFilesPath}/HarmonizerTests.cpp) 
endif()

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

# expected filename extensions for executable files

if (APPLE)
    set (_imgn_xtn ".app")
elseif (UNIX)
    set (_imgn_xtn ".elf")
elseif (WIN32)
    set (_imgn_xtn ".exe")
endif()

#


