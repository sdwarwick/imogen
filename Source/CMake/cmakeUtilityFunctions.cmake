
# Imogen CMake utility functions

# Author: Ben Vining


# =================================================================================================================================================

# this function generates a list of the formats being built, based on the boolean options for each in the cache.

function (imogen_makeFormatsList)

    if (DEFINED formats)  # determine if the list contains the 'standalone' format

        set (indexfinder 0)
        list (FIND formats "Standalone" indexfinder)

        if (indexfinder EQUAL -1)
            set (IMOGEN_buildStandalone FALSE PARENT_SCOPE)
        else()
            set (IMOGEN_buildStandalone TRUE PARENT_SCOPE)
        endif()

        unset (indexfinder)
        return()

    endif()

    #

    set (formatChoosers IMOGEN_buildStandalone IMOGEN_buildVST3 IMOGEN_buildAU IMOGEN_buildUnity IMOGEN_buildAUv3 IMOGEN_buildAAX IMOGEN_buildVST)

    set (formatlist "")
    set (numFormatsThereShouldBe 0)

    set (boolVarName "")
    set (chooserIndex 0)

    foreach (chooser ${formatChoosers})

        set (boolVarName "${chooser}")
        
        if (NOT ${boolVarName})
            continue()
        endif()

        math (EXPR numFormatsThereShouldBe "${numFormatsThereShouldBe} + 1")

        list (FIND formatChoosers ${chooser} chooserIndex)

        if (${chooserIndex} EQUAL 0) 
            list (APPEND formatlist "Standalone")
        elseif (${chooserIndex} EQUAL 1) 
            list (APPEND formatlist "VST3")
        elseif (${chooserIndex} EQUAL 2) 
            list (APPEND formatlist "AU")
        elseif (${chooserIndex} EQUAL 3) 
            list (APPEND formatlist "Unity")
        elseif (${chooserIndex} EQUAL 4) 
            list (APPEND formatlist "AUv3")
        elseif (${chooserIndex} EQUAL 5)
            list (APPEND formatlist "AAX")
            message (WARNING "Reminder: The AAX SDK path must be manually provided by altering the main Imogen CMakeLists.txt to call juce_set_aax_sdk_path before the juce_add_plugin call.")
        elseif (${chooserIndex} EQUAL 6) 
            list (APPEND formatlist "VST")
            message (WARNING "Reminder: The VST2 SDK path must be manually provided by altering the main Imogen CMakeLists.txt to call juce_set_vst2_sdk_path before the juce_add_plugin call.")
        endif()

    endforeach()

    unset (formatChoosers)
    unset (boolVarName)
    unset (chooserIndex)

    set (numFormats 0)
    list (LENGTH formatlist numFormats)

    if (NOT ${numFormats} EQUAL ${numFormatsThereShouldBe})
        message (FATAL_ERROR "Error in generating list of formats.")
    endif()

    unset (numFormatsThereShouldBe)

    if (${numFormats} LESS_EQUAL 1)
        message (FATAL_ERROR "At least one build format must be selected.")
    endif()

    unset (numFormats)

    set (formats ${formatlist} PARENT_SCOPE)

    unset (formatlist)

endfunction()


# =================================================================================================================================================


# Attempts to locate the Imogen Standalone executable, if it exists (has been built already). If the executable file does not exist, but "Standalone" is a current build format, then this function will resolve the prospective path to where the executable file WILL be after it is built, and this path will be used as the applicable schemes' executable

function (imogen_configureStandaloneExecutable)

    if (NOT IMOGEN_buildStandalone AND NOT IMOGEN_launchStandaloneOnBuild AND NOT IMOGEN_preferStandaloneForAllTarget)  # nothing we're doing with the current build requires the standalone executable at all...
        set (IMOGEN_launchStandaloneOnBuild FALSE PARENT_SCOPE)
        return()
    endif()

    set (IMOGEN_launchStandaloneOnBuild TRUE PARENT_SCOPE)

    message (STATUS "Configuring Imogen Standalone executable...")

    set (standalonePath ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Imogen_artefacts/Debug/Standalone)

    if (APPLE)
        set (standalonePath ${standalonePath}/Imogen.app)
    elseif (UNIX)
        set (standalonePath ${standalonePath}/Imogen.elf)
    elseif (WIN32)
        set (standalonePath ${standalonePath}/Imogen.exe)
    endif()

    if (NOT ( DEFINED imogen_standalone_exec_path AND EXISTS ${imogen_standalone_exec_path} ))  # maybe the user has supplied a valid path to the executable somewhere else on their system?
        
        if (IMOGEN_buildStandalone OR EXISTS ${standalonePath})
        	set (imogen_standalone_exec_path ${standalonePath} CACHE FILEPATH "${ds_SALpath}" FORCE)
        else()
        	message (WARNING "Standalone executable not found, and Standalone is not a current build target. Auto-launch feature disabled.")
            set (IMOGEN_launchStandaloneOnBuild FALSE PARENT_SCOPE)
            unset (standalonePath)
        	return()
        endif()

    endif()

    set (IMOGEN_launchStandaloneOnBuild TRUE PARENT_SCOPE)

    if (NOT IMOGEN_buildStandalone)
        message (WARNING "The Standalone executable was located and can be used, but you are not rebuilding the Standalone with this build, so its behavior may not reflect the most recent code changes.")
    elseif (NOT "${imogen_standalone_exec_path}" STREQUAL "${standalonePath}")
    	message (WARNING "The Standalone is being built with this build, but the resolved standalone executable path does not match the path to where the standalone executable will be built. The Imogen Standalone that launches automatically with this build may not reflect the most recent code changes in its behavior.")
    endif()

    unset (standalonePath)

endfunction()


# =================================================================================================================================================


#  attempts to locate the JUCE AudioPluginHost executable, if it exists. If the executable can't be found, this function will attempt to build it automatically

function (imogen_configureAudioPluginHostExecutable)

    message (STATUS "Configuring JUCE AudioPluginHost executable...")

    if (DEFINED imogen_AudioPluginHost_Path AND EXISTS ${imogen_AudioPluginHost_Path}) # maybe the user has supplied a valid path to the executable already?
    	return()
    endif()

    set (pluginHostPath ${IMOGEN_juceDir}/extras/AudioPluginHost/Builds)

    if (APPLE)
        set (pluginHostPath ${pluginHostPath}/MacOSX/build/Debug/AudioPluginHost.app)
    elseif (UNIX)
        set (pluginHostPath ${pluginHostPath}/LinuxMakefile/build/Debug/AudioPluginHost.elf)
    elseif (WIN32)
        set (pluginHostPath ${pluginHostPath}/VisualStudio2019/build/Debug/AudioPluginHost.exe)
    endif()

    if (EXISTS ${pluginHostPath})
    	set (imogen_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "${ds_APHpath}" FORCE)
        unset (pluginHostPath)
    	return()
    endif()

    set (pluginHostPath ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/juce-src/build-aph/extras/AudioPluginHost/AudioPluginHost_artefacts/Debug)

    if (APPLE)
        set (pluginHostPath ${pluginHostPath}/AudioPluginHost.app)
    elseif (UNIX)
        set (pluginHostPath ${pluginHostPath}/AudioPluginHost.elf)
    elseif (WIN32)
        set (pluginHostPath ${pluginHostPath}/AudioPluginHost.exe)
    endif()

    if (EXISTS ${pluginHostPath})
        set (imogen_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "${ds_APHpath}" FORCE)
        unset (pluginHostPath)
        return()
    endif()
 
    message (STATUS "AudioPluginHost executable not found; attempting to build now...")

    execute_process (
            WORKING_DIRECTORY ${imogen_juceDir}
            COMMAND "${CMAKE_COMMAND}"
            "-Bbuild-aph"
            "-DCMAKE_BUILD_TYPE=Debug"
            "-DIS_BUILDING_APH=1"
            "-DJUCE_BUILD_EXTRAS=1")

    execute_process (
            WORKING_DIRECTORY ${imogen_juceDir}
            COMMAND "${CMAKE_COMMAND}"
            "--build" "build-aph"
            "--target" "AudioPluginHost"
            "--config" "Debug")

    if (EXISTS ${pluginHostPath})
        message (STATUS "JUCE AudioPluginHost built successfully!")
        set (imogen_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "${ds_APHpath}" FORCE)
    else()
        message (WARNING "AudioPluginHost executable could not be built; auto-launch feature disabled")
        set (IMOGEN_launchAudioPluginHostOnBuild FALSE PARENT_SCOPE)
    endif()

    unset (pluginHostPath)

endfunction()


# =================================================================================================================================================


# This function cleans up folder organization, putting each individual plugin format target generated by the juce_add_plugin call into a "Build Targets" folder, and also configures XCode scheme executables for each target

function (imogen_configureIndividualBuildTargets)

    if (IMOGEN_launchAudioPluginHostOnBuild OR IMOGEN_launchStandaloneOnBuild)
        set (autoLaunchingAnything TRUE)

        if (IMOGEN_preferStandaloneForAllTarget OR NOT IMOGEN_launchAudioPluginHostOnBuild)
    		set (useStandaloneForAllTarget TRUE)
    	else()
    		set (useStandaloneForAllTarget FALSE)
    	endif()
    else()
        set (autoLaunchingAnything FALSE)
    endif()

    set (thisTargetName "")

    foreach (target ${formats}) 

        set (thisTargetName "Imogen_${target}")  # this is how JUCE automatically names the build targets created for each format

        if (NOT TARGET ${thisTargetName})
            continue()
        endif()

        set_target_properties (${thisTargetName} PROPERTIES FOLDER "Build Targets" XCODE_GENERATE_SCHEME ON)

        if (NOT autoLaunchingAnything)
            message (STATUS "Configuring ${thisTargetName}...")
            continue()
        endif()

        if (IMOGEN_launchStandaloneOnBuild)
            if ("${target}" STREQUAL "Standalone" OR ("${target}" STREQUAL "All" AND useStandaloneForAllTarget))
                set_target_properties (${thisTargetName} PROPERTIES XCODE_SCHEME_EXECUTABLE ${imogen_standalone_exec_path})
                message (STATUS "Executable for ${thisTargetName} set to 'Standalone'")
                continue()
            endif()
        elseif ("${target}" STREQUAL "Standalone")
            continue()
        endif()

        if (IMOGEN_launchAudioPluginHostOnBuild)
        	set_target_properties (${thisTargetName} PROPERTIES XCODE_SCHEME_EXECUTABLE ${imogen_AudioPluginHost_Path})
            message (STATUS "Executable for ${thisTargetName} set to 'AudioPluginHost'")
        endif()

    endforeach()

    unset (thisTargetName)
    unset (autoLaunchingAnything)
    unset (useStandaloneForAllTarget)

endfunction()


# =================================================================================================================================================


# configures unit testing 

function (imogen_configureUnitTesting)

    message (STATUS "Configuring unit testing...")

    if (EXISTS ${imogen_catchDir})

        add_subdirectory (${imogen_catchDir} REQUIRED)

    else()

        message (STATUS "Fetching Catch2 from GitHub...")

        if (NOT fetchcontentincluded)  # simple include guard 
            include (FetchContent)
            set (fetchcontentincluded TRUE PARENT_SCOPE)
        endif()

        FetchContent_Declare (Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v2.13.3)

        FetchContent_MakeAvailable (Catch2)

        set (imogen_catchDir ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/catch2-src CACHE FILEPATH "${ds_catchDir}" FORCE)

    endif()

    source_group (TREE ${testFilesPath} PREFIX "" FILES ${testFiles})

    add_executable (Tests ${testFiles})   # Test executable for unit testing

    set_target_properties (Tests PROPERTIES FOLDER "${CMAKE_PROJECT_NAME}" XCODE_GENERATE_SCHEME ON)

    target_include_directories (Tests PRIVATE ${testFilesPath})

    target_compile_features (Tests PRIVATE ${imogen_compileFeatures})

    target_link_libraries (Tests
        PRIVATE
            Catch2::Catch2
            bv_ImogenEngine
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

    include (${Catch2_SOURCE_DIR}/contrib/Catch.cmake)

    catch_discover_tests (Tests)

endfunction()


# =================================================================================================================================================


# Various smaller utility functions...

function (imogen_addJuce)

    if (EXISTS ${imogen_juceDir})
        add_subdirectory (${imogen_juceDir} REQUIRED)
        return()
    endif()

    message (STATUS "Finding JUCE...")

    if (NOT fetchcontentincluded)  # simple include guard 
        include (FetchContent)
        set (fetchcontentincluded TRUE PARENT_SCOPE)
    endif()
    
    FetchContent_Declare (juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        origin/develop)

    FetchContent_MakeAvailable (juce)

    set (imogen_juceDir ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/juce-src PARENT_SCOPE)

endfunction()

###

function (imogen_checkAllDirectories)

    if (NOT EXISTS ${imogen_sourceDir})
        message (FATAL_ERROR "Source code folder not found")
    elseif (NOT EXISTS ${dspModulesPath})
        message (FATAL_ERROR "DSP modules folder not found")
    elseif (NOT EXISTS ${imogen_GraphicAssetsDir})
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

function (imogen_checkIfCanUseExecutables)

    function (_imgn_turnEmOff)
    	set (IMOGEN_launchAudioPluginHostOnBuild FALSE PARENT_SCOPE)
        set (IMOGEN_launchStandaloneOnBuild FALSE PARENT_SCOPE)
    endfunction()

    if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
        message (WARNING "Auto-launching executables are currently XCode only; these have been disabled because CMake has detected that you are not generating for XCode.")
        _imgn_turnEmOff()
    elseif (NOT (APPLE OR UNIX OR WIN32))
        message (WARNING "Unrecognized operating system; auto-launching executables disabled")
        _imgn_turnEmOff()
    endif()

endfunction()

###


