
# Imogen CMake utility functions

function (checkAllDirectories)

	if (NOT EXISTS ${sourceDir})
	    message (FATAL_ERROR "Source code folder not found")
	elseif (NOT EXISTS ${dspModulesPath})
	    message (FATAL_ERROR "DSP modules folder not found")
	elseif (NOT EXISTS ${GraphicAssetsDir})
	    message (FATAL_ERROR "Graphic assets folder not found")
	endif()

	foreach (path ${pluginSourcesDir} ${guiSourcePath} ${HelpScreenSourcePath} ${IOPanelSourcePath} ${MidiControlSourcePath} ${StaffDisplaySourcePath})
	    if (NOT EXISTS ${path})
	        message (FATAL_ERROR "Source tree not intact - one or more child folders missing")
	    endif()
	endforeach()

	if (IMOGEN_unitTesting AND NOT EXISTS ${testFilesPath})
	    set (IMOGEN_unitTesting FALSE PARENT_SCOPE)
	    message (WARNING "Test files directory not found, testing disabled")
	endif()
endfunction()

###

function (checkIfCanUseExecutables)

	function (_turnEmOff)
		set (IMOGEN_launchAudioPluginHostOnBuild FALSE PARENT_SCOPE)
        set (IMOGEN_launchStandaloneOnBuild FALSE PARENT_SCOPE)
	endfunction()

	if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
        message (WARNING "Auto-launching executables are currently XCode only; these have been disabled because CMake has detected that you are not generating for XCode.")
        _turnEmOff()
    elseif (NOT (APPLE OR UNIX OR WIN32))
        message (WARNING "Unrecognized operating system; auto-launching executables disabled")
        _turnEmOff()
    endif()
endfunction()

###

function (determineIfBuildingStandalone)

	set (isBuildingStandalone FALSE PARENT_SCOPE)

	foreach (format ${formats})      
	    if ("${format}" STREQUAL "Standalone")
	        set (isBuildingStandalone TRUE PARENT_SCOPE)
	        return()
	    endif()
	endforeach()
endfunction()

#

function (configureStandaloneExecutable)

	function (_standaloneNotFound)
		message (WARNING "Standalone executable not found, and Standalone is not a current build target. Auto-launch feature disabled.")
	    set (canUseStandaloneExec FALSE PARENT_SCOPE)
	    set (IMOGEN_launchStandaloneOnBuild FALSE PARENT_SCOPE)
	endfunction()

	if (NOT isBuildingStandalone AND NOT IMOGEN_launchStandaloneOnBuild AND NOT IMOGEN_preferStandaloneForAllTarget)
	    set (canUseStandaloneExec FALSE PARENT_SCOPE)
	    return()
	endif()

    message (STATUS "Configuring Imogen Standalone executable...")

    set (standalonePath ${CMAKE_CURRENT_LIST_DIR}/Builds/Imogen_artefacts/Debug/Standalone)

    if (APPLE)
        set (standalonePath ${standalonePath}/Imogen.app)
    elseif (UNIX)
        set (standalonePath ${standalonePath}/Imogen.elf)
    elseif (WIN32)
        set (standalonePath ${standalonePath}/Imogen.exe)
    endif()

    if (isBuildingStandalone OR EXISTS ${standalonePath})
    	set (IMOGEN_standalone_exec_path ${standalonePath} CACHE FILEPATH "Path to the Imogen standalone executable file")
    else()
    	_standaloneNotFound()
    	return()
    endif()

    if (NOT EXISTS ${IMOGEN_standalone_exec_path} AND isBuildingStandalone)
    	set (IMOGEN_standalone_exec_path ${standalonePath} CACHE FILEPATH "Path to the Imogen standalone executable file" FORCE)
    endif()

    if (isBuildingStandalone OR EXISTS ${IMOGEN_standalone_exec_path})
    	set (canUseStandaloneExec TRUE PARENT_SCOPE)
        
        if (NOT isBuildingStandalone)
            message (WARNING "The Standalone executable was located and can be used, but you are not rebuilding the Standalone with this build, so its behavior may not reflect the most recent code changes.")
        endif()

        if (NOT IMOGEN_launchStandaloneOnBuild)
		    set (IMOGEN_launchStandaloneOnBuild TRUE PARENT_SCOPE)
		endif()
    else()
    	_standaloneNotFound()
    endif()
endfunction()

###

function (configureAudioPluginHostExecutable)

	message (STATUS "Configuring JUCE AudioPluginHost executable...")

	set (pluginHostPath ${IMOGEN_juceDir}/extras/AudioPluginHost/Builds)

    if (APPLE)
        set (pluginHostPath ${pluginHostPath}/MacOSX/build/Debug/AudioPluginHost.app)
    elseif (UNIX)
        set (pluginHostPath ${pluginHostPath}/LinuxMakefile/build/Debug/AudioPluginHost.elf)
    elseif (WIN32)
        set (pluginHostPath ${pluginHostPath}/VisualStudio2019/build/Debug/AudioPluginHost.exe)
    endif()

	set (IMOGEN_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "Path to the JUCE AudioPluginHost executable")

	if (EXISTS ${IMOGEN_AudioPluginHost_Path})
		return()
	endif()

	set (IMOGEN_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "Path to the JUCE AudioPluginHost executable" FORCE)

	if (EXISTS ${IMOGEN_AudioPluginHost_Path})
		return()
	endif()

	# TO DO: automatically build the APH here...?

	message (WARNING "AudioPluginHost executable could not be found; auto-launch feature disabled")
    set (IMOGEN_launchAudioPluginHostOnBuild FALSE PARENT_SCOPE)
endfunction()

###

function (configureIndividualBuildTargets)

	# This function cleans up folder organization, putting each individual plugin format target generated by the juce_add_plugin call into a "Build Targets" folder, and also configures XCode scheme executables for each target

	if (IMOGEN_launchAudioPluginHostOnBuild OR IMOGEN_launchStandaloneOnBuild)
	    set (autoLaunchingAnything TRUE)
	else()
	    set (autoLaunchingAnything FALSE)
	endif()

	if (IMOGEN_preferStandaloneForAllTarget OR NOT IMOGEN_launchAudioPluginHostOnBuild)
		set (useStandaloneForAllTarget TRUE)
	else()
		set (useStandaloneForAllTarget FALSE)
	endif()

	#

	foreach (target ${formats} "All") 
	    set (thisTargetName "Imogen_${target}")  # this is how JUCE automatically names the build targets created for each format

	    if (NOT TARGET ${thisTargetName})
	        continue()
	    endif()

	    set_target_properties (${thisTargetName} PROPERTIES FOLDER "Build Targets" XCODE_GENERATE_SCHEME ON)

	    if (NOT autoLaunchingAnything)
	        continue()
	    endif()

	    if (canUseStandaloneExec)
	        if ("${target}" STREQUAL "Standalone" OR ("${target}" STREQUAL "All" AND useStandaloneForAllTarget))
	            set_target_properties (${thisTargetName} PROPERTIES XCODE_SCHEME_EXECUTABLE ${IMOGEN_standalone_exec_path})
	            message (STATUS "Executable for ${thisTargetName} set to 'Standalone'")
	            continue()
	        endif()
	    elseif ("${target}" STREQUAL "Standalone")
	        continue()
	    endif()

	    if (IMOGEN_launchAudioPluginHostOnBuild)
	    	set_target_properties (${thisTargetName} PROPERTIES XCODE_SCHEME_EXECUTABLE ${IMOGEN_AudioPluginHost_Path})
	        message (STATUS "Executable for ${thisTargetName} set to 'AudioPluginHost'")
	    endif()
	endforeach()
endfunction()

#


