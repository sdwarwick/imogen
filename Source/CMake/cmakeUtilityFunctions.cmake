
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

        list (REMOVE_DUPLICATES formats)

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

            return() 

        else()  # we must attempt to construct the list from the boolean cache variables... continue on to the logic below 

            message (WARNING "No valid format strings provided; attempting to construct format list from boolean cache variables...")
            
        endif()

    endif (DEFINED formats)

    #

    set (formatChoosers imogen_buildStandalone imogen_buildVST3 imogen_buildAU imogen_buildUnity imogen_buildAUv3 imogen_buildAAX imogen_buildVST)

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
        message (AUTHOR_WARNING "Unknown error in generating list of formats :(")
    endif()

    set (formats ${formatlist} PARENT_SCOPE)

endfunction()

#


# =================================================================================================================================================


# Attempts to locate the Imogen Standalone executable, if it exists (has been built already). If the executable file does not exist, but "Standalone" is a current build format, then this function will resolve the prospective path to where the executable file WILL be after it is built, and this path will be used as the applicable schemes' executable

function (imogen_configureStandaloneExecutable)

    if (NOT ( IMOGEN_launchStandaloneOnBuild OR imogen_buildStandalone OR (IMOGEN_launchAudioPluginHostOnBuild AND IMOGEN_preferStandaloneForAllTarget) ))
        set (IMOGEN_launchStandaloneOnBuild FALSE PARENT_SCOPE)
        return()
    endif()

    message (STATUS "Configuring Imogen Standalone executable...")

    set (standalonePath ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Imogen_artefacts/Debug/Standalone/Imogen${_imgn_xtn})

    if (NOT (DEFINED imogen_standalone_exec_path AND EXISTS ${imogen_standalone_exec_path}))

        if (imogen_buildStandalone OR EXISTS ${standalonePath})

        	set (imogen_standalone_exec_path ${standalonePath} CACHE FILEPATH "${ds_SALpath}" FORCE)

        else()  # check the other build format folder, just in case that format has been built before...

            set (standalonePath ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Imogen_artefacts/Release/Standalone/Imogen${_imgn_xtn})

            if (EXISTS ${standalonePath})
                set (imogen_standalone_exec_path ${standalonePath} CACHE FILEPATH "${ds_SALpath}" FORCE)
            else()
                message (WARNING "Standalone executable not found, and Standalone is not a current build target. Auto-launch feature disabled.")
                set (IMOGEN_launchStandaloneOnBuild FALSE PARENT_SCOPE)
                return()
            endif()

        endif()

    endif()

    set (IMOGEN_launchStandaloneOnBuild TRUE PARENT_SCOPE)

    message (STATUS "Imogen Standalone executable found at ${imogen_standalone_exec_path}")

    if (NOT imogen_buildStandalone)
        message (WARNING "The Standalone executable was located and can be used, but you are not rebuilding the Standalone with this build, so its behavior may not reflect the most recent code changes.")
    elseif (NOT "${imogen_standalone_exec_path}" STREQUAL "${standalonePath}")
    	message (WARNING "The Standalone is being built with this build, but the resolved standalone executable path does not match the path to where the standalone executable will be built. The Imogen Standalone that launches automatically with this build may not reflect the most recent code changes in its behavior.")
    endif()

endfunction()


# =================================================================================================================================================


#  attempts to locate the JUCE AudioPluginHost executable, if it exists. If the executable can't be found, this function will attempt to build it automatically

function (imogen_configureAudioPluginHostExecutable)

    if (NOT IMOGEN_launchAudioPluginHostOnBuild)
        return()
    endif()

    message (STATUS "Configuring JUCE AudioPluginHost executable...")

    if (DEFINED imogen_AudioPluginHost_Path AND EXISTS ${imogen_AudioPluginHost_Path})
        message (STATUS "AudioPluginHost executable found at ${imogen_AudioPluginHost_Path}")
        return()
    endif()

    if (NOT DEFINED APH_build_format)
        message (STATUS "No AudioPluginHost build format specified; defaulting to the same build type as the current build.")
        set (APH_build_format "${_imgn_buildType}")
    elseif (${APH_build_format} STREQUAL "DEBUG")
        set (APH_build_format Debug)
    elseif (${APH_build_format} STREQUAL "RELEASE")
        set (APH_build_format Release)
    elseif (NOT (${APH_build_format} STREQUAL "Debug" OR ${APH_build_format} STREQUAL "Release"))
        message (WARNING "Invalid AudioPluginHost build format specified; defaulting to Debug build mode.")
        set (APH_build_format Debug)
    endif()

    if (${APH_build_format} STREQUAL "Debug")
        set (other_APH_format "Release")
    else()
        set (other_APH_format "Debug")
    endif()

    if (APPLE)
        set (APHbuildfolder "MacOSX")
    elseif (UNIX)
        set (APHbuildfolder "LinuxMakefile")
    elseif (WIN32)
        set (APHbuildfolder "VisualStudio2019")
    endif()

    set (pluginHostPath ${IMOGEN_juceDir}/extras/AudioPluginHost/Builds/${APHbuildfolder}/build/${APH_build_format}/AudioPluginHost${_imgn_xtn})

    if (EXISTS ${pluginHostPath})
    	set (imogen_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "${ds_APHpath}" FORCE)
        message (STATUS "AudioPluginHost executable found at ${imogen_AudioPluginHost_Path}")
    	return()
    endif()

    set (pluginHostPath ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/juce-src/build-aph/extras/AudioPluginHost/AudioPluginHost_artefacts/${APH_build_format}/AudioPluginHost${_imgn_xtn})  # this is where the APH will end up after the automatic build process...

    if (EXISTS ${pluginHostPath})
        set (imogen_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "${ds_APHpath}" FORCE)
        message (STATUS "AudioPluginHost executable found at ${imogen_AudioPluginHost_Path}")
        return()
    endif()

    set (pluginHostPath ${IMOGEN_juceDir}/extras/AudioPluginHost/Builds/${APHbuildfolder}/build/${other_APH_format}/AudioPluginHost${_imgn_xtn})  # check the other build format folders, to check if those executables have been built before...

    if (EXISTS ${pluginHostPath})
        set (imogen_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "${ds_APHpath}" FORCE)
        message (STATUS "AudioPluginHost executable found at ${imogen_AudioPluginHost_Path}")
        return()
    endif()

    set (pluginHostPath ${CMAKE_CURRENT_SOURCE_DIR}/Builds/_deps/juce-src/build-aph/extras/AudioPluginHost/AudioPluginHost_artefacts/${other_APH_format}/AudioPluginHost${_imgn_xtn})  

    if (EXISTS ${pluginHostPath})
        set (imogen_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "${ds_APHpath}" FORCE)
        message (STATUS "AudioPluginHost executable found at ${imogen_AudioPluginHost_Path}")
        return()
    endif()


    if (NOT allow_build_APH)
        message (WARNING "AudioPluginHost executable not found, and building has been disabled. AudioPluginHost auto-launch feature disabled.")
        set (IMOGEN_launchAudioPluginHostOnBuild FALSE PARENT_SCOPE)
        return()
    endif()
 
    message (STATUS "AudioPluginHost executable not found; attempting to build now...")

    execute_process (
            WORKING_DIRECTORY ${imogen_juceDir}
            COMMAND "${CMAKE_COMMAND}"
            "-Bbuild-aph"
            "-DCMAKE_BUILD_TYPE=${APH_build_format}"
            "-DJUCE_BUILD_EXTRAS=1")

    execute_process (
            WORKING_DIRECTORY ${imogen_juceDir}
            COMMAND "${CMAKE_COMMAND}"
            "--build" "build-aph"
            "--target" "AudioPluginHost"
            "--config" "${APH_build_format}")

    set (pluginHostPath ${imogen_juceDir}/build-aph/extras/AudioPluginHost/AudioPluginHost_artefacts/${APH_build_format}/AudioPluginHost${_imgn_xtn})  # this is where the APH gets built to

    if (EXISTS ${pluginHostPath})
        message (STATUS "JUCE AudioPluginHost built successfully!")
        set (imogen_AudioPluginHost_Path ${pluginHostPath} CACHE FILEPATH "${ds_APHpath}" FORCE)
    else()
        message (WARNING "AudioPluginHost executable could not be built; auto-launch feature disabled")
        set (IMOGEN_launchAudioPluginHostOnBuild FALSE PARENT_SCOPE)
    endif()

endfunction()


# =================================================================================================================================================


# This function cleans up folder organization, putting each individual plugin format target generated by the juce_add_plugin call into a "Build Targets" folder, and also configures XCode scheme executables for each target

function (imogen_configureIndividualBuildTargets)

    set (autoLaunchingAnything FALSE)

    if (IMOGEN_launchAudioPluginHostOnBuild OR IMOGEN_launchStandaloneOnBuild)
        set (autoLaunchingAnything TRUE)

        if (IMOGEN_preferStandaloneForAllTarget OR NOT IMOGEN_launchAudioPluginHostOnBuild)
    		set (useStandaloneForAllTarget TRUE)
    	else()
    		set (useStandaloneForAllTarget FALSE)
    	endif()
    endif()

    list (APPEND formats "All")

    set (listOfExecutables "")

    set (thisTargetName "")

    foreach (target ${formats}) 

        set (thisTargetName "Imogen_${target}")  # this is how JUCE automatically names the build targets created for each format

        if (NOT TARGET ${thisTargetName})
            continue()
        endif()

        set_target_properties (${thisTargetName} PROPERTIES FOLDER "Build Targets" XCODE_GENERATE_SCHEME ON)

        if (NOT ${target} STREQUAL "All")
            list (APPEND listOfExecutables ${thisTargetName})
        endif()

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

    set (_imgn_listOfExecutables ${listOfExecutables} PARENT_SCOPE)

endfunction()


# =================================================================================================================================================


# this function configures unit testing 

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

    set_target_properties (Tests PROPERTIES FOLDER "Imogen" XCODE_GENERATE_SCHEME ON)

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

# this function configures CPack to generate a user-friendly installer for the built Imogen executables

function (imogen_configureInstaller)

    foreach (executable ${_imgn_listOfExecutables})

        install (
            TARGETS ${executable}
            RUNTIME DESTINATION bin
            BUNDLE  DESTINATION bin
            LIBRARY DESTINATION lib
            RESOURCE DESTINATION "bin/${executable}/resource"
            COMPONENT ${executable})
        
    endforeach()

    #

    set (CPACK_PACKAGE_NAME "Imogen")
    set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "My funky project")
    set (CPACK_PACKAGE_VENDOR "BenViningMusicSoftware")
    # set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
    # set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
    set (CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
    set (CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
    set (CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
    set (CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
    set (CPACK_PACKAGE_INSTALL_DIRECTORY "Imogen ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

    if (WIN32 AND NOT UNIX)
      # There is a bug in NSI that does not handle full UNIX paths properly.
      # Make sure there is at least one set of four backlashes.

      # set (CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
      set (CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\Imogen.exe")
      set (CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Imogen")
      # set (CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
      # set (CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
      set (CPACK_NSIS_CONTACT "ben.the.vining@gmail.com")
      set (CPACK_NSIS_MODIFY_PATH ON)

    else()

      set (CPACK_STRIP_FILES "bin/Imogen")
      set (CPACK_SOURCE_STRIP_FILES "")

    endif()

    #

    set (CPACK_COMPONENTS_ALL ${_imgn_listOfExecutables})

    set (CPACK_PACKAGE_EXECUTABLES ${_imgn_listOfExecutables})

    include (CPack)

    # set_target_properties (Imogen_package PROPERTIES XCODE_GENERATE_SCHEME ON)

endfunction()


# =================================================================================================================================================

# configures pluginval to be run after building the targets

function (imogen_configurePluginval)

    message (STATUS "Configuring pluginval...")

    if (NOT EXISTS ${imogen_pluginval_path})

        set (pluginval_downloadDir ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Downloads/pluginval)

        set (pluginval_path ${pluginval_downloadDir}/pluginval${_imgn_xtn})

        if (EXISTS ${pluginval_path})

            set (imogen_pluginval_path ${pluginval_path} CACHE FILEPATH "${ds_pivPath}" FORCE)  

        else()

            if (APPLE)
                set (pluginval_pkgName "pluginval_macOS.zip")
            elseif (UNIX)
                set (pluginval_pkgName "pluginval_Linux.zip")
            elseif (WIN32)
                set (pluginval_pkgName "pluginval_Windows.zip")
            endif()

            set (pluginval_zipFilePath ${pluginval_downloadDir}/${pluginval_pkgName})

            if (NOT EXISTS ${pluginval_zipFilePath})

                message (STATUS "Downloading pluginval binaries...")

                file (DOWNLOAD
                    https://github.com/Tracktion/pluginval/releases/latest/download/${pluginval_pkgName}
                    ${pluginval_zipFilePath})

            endif()

            message (STATUS "Unzipping pluginval...")

            file (ARCHIVE_EXTRACT 
                INPUT ${pluginval_zipFilePath}
                DESTINATION ${pluginval_downloadDir})

            if (EXISTS ${pluginval_path})
                set (imogen_pluginval_path ${pluginval_path} CACHE FILEPATH "${ds_pivPath}" FORCE)  
            else()
                message (WARNING "Error in configuring pluginval; auto-run feature disabled.")
                set (IMOGEN_runPluginvalOnBuild FALSE PARENT_SCOPE)
                return()
            endif()

        endif()

    endif()

    if (APPLE)
        set (pluginval_path ${imogen_pluginval_path}/Contents/MacOS/pluginval)
    elseif (UNIX)
        set (pluginval_path ${imogen_pluginval_path}/Contents/Linux/pluginval)
    elseif (WIN32)
        set (pluginval_path ${imogen_pluginval_path}/Contents/Windows/pluginval)
    endif()

    if (${pluginval_intensityLevel} LESS 1)
        set (pluginval_intensityLevel 1 PARENT_SCOPE)
    elseif (${pluginval_intensityLevel} GREATER 10)
        set (pluginval_intensityLevel 10 PARENT_SCOPE)
    endif()

    foreach (executable ${_imgn_listOfExecutables})

        if (${executable} STREQUAL "Imogen_Standalone" OR ${executable} STREQUAL "Imogen_Unity")
            continue()
        endif()

        set (pluginPath ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Imogen_artefacts/Debug/)

        if (${executable} STREQUAL "Imogen_VST3")
            string (APPEND pluginPath "VST3/Imogen.vst3")
        elseif (${executable} STREQUAL "Imogen_VST")
            string (APPEND pluginPath "VST/Imogen.vst")
        elseif (${executable} STREQUAL "Imogen_AU")
            string (APPEND pluginPath "AU/Imogen.component")
        elseif (${executable} STREQUAL "Imogen_AUv3")
            string (APPEND pluginPath "AUv3/Imogen.component")
        elseif (${executable} STREQUAL "Imogen_AAX")
            string (APPEND pluginPath "AAX/Imogen.aaxplugin")
        endif()

        add_custom_command (
            TARGET ${executable}
            POST_BUILD
            COMMAND "${pluginval_path}"
            ARGS "--strictnesslevel" "${pluginval_intensityLevel}" "--validate-in-process" "--validate" "${pluginPath}"
            )

    endforeach()

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

    set (_imgn_directoryWarning "The Imogen source code can be redownloaded in its entirety from https://github.com/benthevining/imogen.")

    if (NOT EXISTS ${sourceDir})
        message (FATAL_ERROR "Source code folder not found. ${_imgn_directoryWarning}")
    elseif (NOT EXISTS ${dspModulesPath})
        message (FATAL_ERROR "DSP modules folder not found. ${_imgn_directoryWarning}")
    elseif (NOT EXISTS ${graphicAssetsDir})
        message (FATAL_ERROR "Graphic assets folder not found. ${_imgn_directoryWarning}")
    endif()

    foreach (path ${pluginSourcesDir} ${guiSourcePath} ${HelpScreenSourcePath} ${IOPanelSourcePath} ${MidiControlSourcePath} ${StaffDisplaySourcePath})
        if (NOT EXISTS ${path})
            message (FATAL_ERROR "Source tree not intact - one or more child folders missing. ${_imgn_directoryWarning}")
        endif()
    endforeach()

    if (IMOGEN_unitTesting AND NOT EXISTS ${testFilesPath})
        set (IMOGEN_unitTesting FALSE PARENT_SCOPE)
        message (WARNING "Test files directory not found, testing disabled.")
    endif()

endfunction()

###

function (imogen_checkOS)

    function (_imgn_turnOffAutolaunchers)
        set (IMOGEN_launchAudioPluginHostOnBuild FALSE PARENT_SCOPE)
        set (IMOGEN_launchStandaloneOnBuild FALSE PARENT_SCOPE)
    endfunction()

    if (IMOGEN_launchAudioPluginHostOnBuild OR IMOGEN_launchStandaloneOnBuild)
        if (NOT "${CMAKE_GENERATOR}" STREQUAL "Xcode")
            message (WARNING "Auto-launching executables are currently XCode only; these have been disabled because CMake has detected that you are not generating for XCode.")
            _imgn_turnOffAutolaunchers()
        elseif (NOT (APPLE OR UNIX OR WIN32))
            message (WARNING "Unrecognized operating system; auto-launching executables disabled.")
            _imgn_turnOffAutolaunchers()
        endif()
    endif()

    if (IMOGEN_runPluginvalOnBuild)
        if (NOT (APPLE OR UNIX OR WIN32))
            message (WARNING "Unrecognized operating system; auto-launch of pluginval disabled.")
            set (IMOGEN_runPluginvalOnBuild FALSE PARENT_SCOPE)
        endif()
    endif()

endfunction()

###

function (imogen_displayPossibleDownloadsWarningMessage)

    if (NOT (IMOGEN_unitTesting OR IMOGEN_runPluginvalOnBuild))
        message (STATUS "JUCE will be downloaded from GitHub, only if the most recent version can't be found locally.")
        return()
    endif()

    set (possibleDownloads "JUCE")

    if (IMOGEN_unitTesting)
        list (APPEND possibleDownloads "Catch2")
    endif()

    if (IMOGEN_runPluginvalOnBuild)
        list (APPEND possibleDownloads "pluginval")
    endif()

    set (numPossibleDwnlds 1)
    list (LENGTH possibleDownloads numPossibleDwnlds)
    math (EXPR numPossibleDwnlds "${numPossibleDwnlds} - 1")

    set (message "")

    set (numWrittenToMessage 0)

    foreach (download ${possibleDownloads})

        if (${numWrittenToMessage} EQUAL 0)
            set (message "${download}")
        elseif (${numWrittenToMessage} EQUAL ${numPossibleDwnlds})
            if (${numPossibleDwnlds} GREATER 1)
                string (APPEND message ", and ${download}")
            else()
                string (APPEND message " and ${download}")
            endif()
        else()
            string (APPEND message ", ${download}")
        endif()

        math (EXPR numWrittenToMessage "${numWrittenToMessage} + 1")
        
    endforeach()

    message (STATUS "${message} will be downloaded from GitHub, only if the most recent versions can't be found locally.")

endfunction()

###

function (imogen_clearOldBuildFiles)

    file (REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/Builds/juce_binarydata_ImogenGraphicAssets)

endfunction()

# =================================================================================================================================================


