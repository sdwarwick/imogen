
# Imogen CMake utility functions

# Author: Ben Vining


# =================================================================================================================================================

# this function generates a list of the formats being built, based on the boolean options for each in the cache.

function (imogen_makeFormatsList)

    if (DEFINED formats)

        set (validFormatStrings Standalone VST3 AU Unity AUv3 AAX VST)

        set (listIndexFinder -1)

        foreach (format ${formats})  # remove any non-valid format string identifiers from the provided list 

            list (FIND validFormatStrings "${format}" listIndexFinder)

            if (listIndexFinder EQUAL -1)
                message (WARNING "Invalid format string '${format}' removed from formats list")
                list (REMOVE_ITEM formats "${format}")
            elseif (${format} STREQUAL "AAX")
                message (WARNING "Reminder: The AAX SDK path must be manually provided by altering the main Imogen CMakeLists.txt to call juce_set_aax_sdk_path before the juce_add_plugin call.")
            elseif (${format} STREQUAL "VST")
                message (WARNING "Reminder: The VST2 SDK path must be manually provided by altering the main Imogen CMakeLists.txt to call juce_set_vst2_sdk_path before the juce_add_plugin call.")
            endif()
            
        endforeach()

        unset (validFormatStrings)

        set (listLength 0)
        list (LENGTH formats listLength)

        if (listLength GREATER 0)  # we can use the user-provided string list, it contains at least 1 valid build format

            set (listIndexFinder -1)
            list (FIND formats "Standalone" listIndexFinder)

            if (listIndexFinder EQUAL -1)  # test if the list contains "Standalone"
                set (IMOGEN_buildStandalone FALSE PARENT_SCOPE)  # mask the cache variable to ensure this value is accurate for future functions 
            else()
                set (IMOGEN_buildStandalone TRUE PARENT_SCOPE)
            endif()

            message (STATUS "Successfully parsed format list: ${formats}")

            unset (listIndexFinder)
            unset (listLength)

            return() 

        else()  # we must attempt to construct the list from the boolean cache variables... continue on to the logic below 

            message (WARNING "No valid format strings provided; attempting to construct format list from boolean cache variables...")
            unset (listLength)
            unset (listIndexFinder)
            
        endif()

    endif (DEFINED formats)

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

    set (numFormats 0)
    list (LENGTH formatlist numFormats)

    if (${numFormats} EQUAL 0)
        message (FATAL_ERROR "At least one build format must be selected.")
    endif()

    if (NOT ${numFormats} EQUAL ${numFormatsThereShouldBe})
        message (AUTHOR_WARNING "Unknown error in generating list of formats.")
    endif()

    set (formats ${formatlist} PARENT_SCOPE)

    unset (formatChoosers)
    unset (boolVarName)
    unset (chooserIndex)
    unset (formatlist)
    unset (numFormatsThereShouldBe)
    unset (numFormats)

endfunction()

#


# =================================================================================================================================================


# Attempts to locate the Imogen Standalone executable, if it exists (has been built already). If the executable file does not exist, but "Standalone" is a current build format, then this function will resolve the prospective path to where the executable file WILL be after it is built, and this path will be used as the applicable schemes' executable

function (imogen_configureStandaloneExecutable)

    if (NOT ( IMOGEN_launchStandaloneOnBuild OR IMOGEN_buildStandalone OR (IMOGEN_launchAudioPluginHostOnBuild AND IMOGEN_preferStandaloneForAllTarget) ))
        set (IMOGEN_launchStandaloneOnBuild FALSE PARENT_SCOPE)
        return()
    endif()

    message (STATUS "Configuring Imogen Standalone executable...")

    set (standalonePath ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Imogen_artefacts/Debug/Standalone/Imogen)

    string (APPEND standalonePath "${_imgn_xtn}")

    if (NOT (DEFINED imogen_standalone_exec_path AND EXISTS ${imogen_standalone_exec_path}))
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

    if (NOT IMOGEN_launchAudioPluginHostOnBuild)
        return()
    elseif (DEFINED imogen_AudioPluginHost_Path AND EXISTS ${imogen_AudioPluginHost_Path})
        return()
    endif()

    message (STATUS "Configuring JUCE AudioPluginHost executable...")

    set (pluginHostPath ${IMOGEN_juceDir}/extras/AudioPluginHost/Builds/${_imgn_buildfolder}/build/Debug/AudioPluginHost)

    string (APPEND pluginHostPath "${_imgn_xtn}")

    if (EXISTS ${pluginHostPath})
    	set (imogen_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "${ds_APHpath}" FORCE)
        unset (pluginHostPath)
    	return()
    endif()

    set (pluginHostPath ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/juce-src/build-aph/extras/AudioPluginHost/AudioPluginHost_artefacts/Debug/AudioPluginHost)  # this is where the APH will end up after the automatic build process...

    string (APPEND pluginHostPath "${_imgn_xtn}")

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

    list (APPEND formats "All")

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
        return()
    endif()

    if (APPLE)
        set (_imgn_xtn ".app" PARENT_SCOPE)
        set (_imgn_buildfolder "MacOSX" PARENT_SCOPE)
    elseif (UNIX)
        set (_imgn_xtn ".elf" PARENT_SCOPE)
        set (_imgn_buildfolder "LinuxMakefile" PARENT_SCOPE)
    elseif (WIN32)
        set (_imgn_xtn ".exe")
        set (_imgn_buildfolder VisualStudio2019)
    else()
        message (WARNING "Unrecognized operating system; auto-launching executables disabled")
        _imgn_turnEmOff()
    endif()

endfunction()

###

# =================================================================================================================================================

#  cleans up memory allocation by unsetting all variables possibly set by the Imogen CMake script 

function (_imogen_unset_all_variables)
    unset (IMOGEN_launchAudioPluginHostOnBuild)
    unset (IMOGEN_launchStandaloneOnBuild)
    unset (IMOGEN_unitTesting)
    unset (IMOGEN_buildStandalone)
    unset (imogen_juceDir)
    unset (imogen_catchDir)
    unset (dspModulesPath)
    unset (pluginSourcesDir)
    unset (guiSourcePath)
    unset (HelpScreenSourcePath)
    unset (IOPanelSourcePath)
    unset (MidiControlSourcePath)
    unset (StaffDisplaySourcePath)
    unset (testFilesPath)
    unset (customModulesNeeded)
    unset (sourceFiles)
    unset (testFiles)
    unset (graphicAssetFiles)
    unset (ds_testing)
    unset (ds_juceDir)
    unset (ds_catchDir)
    unset (ds_launchAPH)
    unset (ds_launchSAL)
    unset (ds_preferSALforAll)
    unset (ds_SALpath)
    unset (ds_APHpath)
    unset (fetchcontentincluded)
    unset (formats)
endfunction()


