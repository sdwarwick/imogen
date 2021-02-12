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
	    set (IMOGEN_unitTesting FALSE CACHE BOOL "Enable Imogen unit tests" FORCE)
	    message (WARNING "Test files directory not found, testing disabled")
	endif()
endfunction()

#

function (checkIfCanUseExecutables)
	if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
        message (WARNING "Auto-launching executables are currently XCode only; these have been disabled because CMake has detected that you are not generating for XCode.")
        set (IMOGEN_launchAudioPluginHostOnBuild FALSE CACHE BOOL "Automatically launch the JUCE AudioPluginHost" FORCE)
        set (IMOGEN_launchStandaloneOnBuild FALSE CACHE BOOL "Automatically launch the Imogen standalone" FORCE)
    elseif (NOT (APPLE OR UNIX OR WIN32))
        message (WARNING "Unrecognized operating system; auto-launching executables disabled")
        set (IMOGEN_launchAudioPluginHostOnBuild FALSE CACHE BOOL "Automatically launch the JUCE AudioPluginHost" FORCE)
        set (IMOGEN_launchStandaloneOnBuild FALSE CACHE BOOL "Automatically launch the Imogen standalone" FORCE)
    endif()
endfunction()

#

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
	if (NOT isBuildingStandalone AND NOT IMOGEN_launchStandaloneOnBuild AND NOT IMOGEN_preferStandaloneForAllTarget)
	    set (canUseStandaloneExec FALSE PARENT_SCOPE)
	else()
	    message (STATUS "Configuring Imogen Standalone executable...")

	    set (IMOGEN_standalone_exec_path ${CMAKE_CURRENT_LIST_DIR}/Builds/Imogen_artefacts/Debug/Standalone CACHE FILEPATH "Path to the Imogen standalone executable file" FORCE) 
	    
	    if (APPLE)
	        set (IMOGEN_standalone_exec_path ${IMOGEN_standalone_exec_path}/Imogen.app CACHE FILEPATH "Path to the Imogen standalone executable file" FORCE)
	    elseif (UNIX)
	        set (IMOGEN_standalone_exec_path ${IMOGEN_standalone_exec_path}/Imogen.elf CACHE FILEPATH "Path to the Imogen standalone executable file" FORCE)
	    elseif (WIN32)
	        set (IMOGEN_standalone_exec_path ${IMOGEN_standalone_exec_path}/Imogen.exe CACHE FILEPATH "Path to the Imogen standalone executable file" FORCE)
	    endif()

	    if (isBuildingStandalone OR EXISTS ${IMOGEN_standalone_exec_path})
	        set (canUseStandaloneExec TRUE PARENT_SCOPE)
	        
	        if (NOT isBuildingStandalone)
	            message (WARNING "The Standalone executable was located and can be used, but you are not rebuilding the Standalone with this build, so its behavior may not reflect the most recent code changes.")
	        endif()
	    else()
	        message (WARNING "Standalone executable not found, and Standalone is not a current build target. Auto-launch feature disabled.")
	        set (canUseStandaloneExec FALSE PARENT_SCOPE)
	        set (IMOGEN_launchStandaloneOnBuild FALSE CACHE BOOL "Automatically launch the Imogen standalone" FORCE)
	        set (IMOGEN_standalone_exec_path "" CACHE FILEPATH "Path to the Imogen standalone executable file" FORCE)
	    endif()
	endif()
endfunction()

#

function (configureAudioPluginHostExecutable)
	message (STATUS "Configuring JUCE AudioPluginHost executable...")

    set (IMOGEN_AudioPluginHost_Path ${IMOGEN_juceDir}/extras/AudioPluginHost/Builds CACHE FILEPATH "Path to the JUCE AudioPluginHost executable" FORCE)

    if (APPLE)
        set (IMOGEN_AudioPluginHost_Path ${IMOGEN_AudioPluginHost_Path}/MacOSX/build/Debug/AudioPluginHost.app CACHE FILEPATH "Path to the JUCE AudioPluginHost executable" FORCE)
    elseif (UNIX)
        set (IMOGEN_AudioPluginHost_Path ${IMOGEN_AudioPluginHost_Path}/LinuxMakefile/build/Debug/AudioPluginHost.elf CACHE FILEPATH "Path to the JUCE AudioPluginHost executable" FORCE)
    elseif (WIN32)
        set (IMOGEN_AudioPluginHost_Path ${IMOGEN_AudioPluginHost_Path}/VisualStudio2019/build/Debug/AudioPluginHost.exe CACHE FILEPATH "Path to the JUCE AudioPluginHost executable" FORCE)
    endif()

    if (NOT EXISTS ${IMOGEN_AudioPluginHost_Path})
    	message (WARNING "AudioPluginHost executable could not be found; auto-launch feature disabled")
        set (IMOGEN_launchAudioPluginHostOnBuild FALSE CACHE BOOL "Automatically launch the JUCE AudioPluginHost" FORCE)
        set (IMOGEN_AudioPluginHost_Path "" CACHE FILEPATH "Path to the JUCE AudioPluginHost executable" FORCE)

        # TO DO: automatically build the APH here...?

    endif()
endfunction()

#

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

	    set (thisTargetName "${CMAKE_PROJECT_NAME}_${target}")  # this is how JUCE automatically names the build targets created for each format

	    if (NOT TARGET ${thisTargetName})
	        continue()
	    endif()

	    set_target_properties (${thisTargetName} PROPERTIES FOLDER "Build Targets" XCODE_GENERATE_SCHEME ON)

	    if (NOT autoLaunchingAnything)
	        continue()
	    endif()

	    if (canUseStandaloneExec)
	        if ( "${target}" STREQUAL "Standalone" 
	         OR ("${target}" STREQUAL "All" AND useStandaloneForAllTarget))
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




