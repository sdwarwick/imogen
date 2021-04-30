#            _             _   _                _                _                 _               _
#           /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
#           \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
#           /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
#          / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
#         / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
#        / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
#       / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
#   ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
#  /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
#  \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
#  This file is part of the Imogen codebase.
 
#  @2021 by Ben Vining. All rights reserved.

#  imogenConfig.cmake :		This file contains the build configuration elements common to both the main Imogen build and the Imogen Remote build.


if (NOT DEFINED IMOGEN_BUILD_FOR_ELK)
    set (IMOGEN_BUILD_FOR_ELK FALSE)
endif()


set (ImogenCore_sourceFiles
	${Imogen_sourceDir}/ImogenCommon.h
    ${Imogen_sourceDir}/GUI/GUI_Framework.h
    ${Imogen_sourceDir}/GUI/ImogenGUI.h
    ${Imogen_sourceDir}/GUI/ImogenGUI.cpp
    ${Imogen_sourceDir}/GUI/Holders/ImogenGuiHolder.h
    ${Imogen_sourceDir}/GUI/MainDialComponent/MainDialComponent.h
    ${Imogen_sourceDir}/GUI/MainDialComponent/MainDialComponent.cpp
    ${Imogen_sourceDir}/GUI/LookAndFeel/ImogenLookAndFeel.h
    ${Imogen_sourceDir}/GUI/LookAndFeel/ImogenLookAndFeel.cpp
    ${Imogen_sourceDir}/OSC/OSC.h
    ${Imogen_sourceDir}/OSC/OSC_reciever.h
    ${Imogen_sourceDir}/OSC/OSC_sender.h
    )

#

set (Imogen_assetFiles
	${Imogen_sourceDir}/../assets/imogen_icon.png
	)

#

set_property (GLOBAL PROPERTY USE_FOLDERS YES)
set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Build Targets")

if (APPLE)
	set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "")  # universal macOS binaries 
	set (CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum OS X deployment version" FORCE)  # minimum MacOS version to build for
elseif (WIN32)
	set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # static linking on Windows
	set (CMAKE_WIN32_EXECUTABLE TRUE)
endif()

set (CMAKE_XCODE_GENERATE_SCHEME OFF)  # schemes are maually generated for each target to avoid clutter from modules getting schemes, etc

set (CMAKE_SUPPRESS_REGENERATION TRUE)  # no "zero-check" target
set (CMAKE_OPTIMIZE_DEPENDENCIES TRUE)
set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option (JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option (JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)
option (JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)

#

if (ANDROID)
    add_definitions (
    "-DJUCE_ANDROID=1" 
    "-DJUCE_PUSH_NOTIFICATIONS=1" 
    "-DJUCE_PUSH_NOTIFICATIONS_ACTIVITY=\"com/rmsl/juce/JuceActivity\"" 
    )
endif()

#

# ADD JUCE #

if (DEFINED bv_juceDir)
	add_subdirectory (${bv_juceDir} ${CMAKE_CURRENT_SOURCE_DIR}/Builds/JUCE)
else()
	if (NOT DEFINED bv_juceGitRepoToUse)
        set (bv_juceGitRepoToUse https://github.com/juce-framework/JUCE.git)
	endif()

	if (NOT DEFINED bv_juceGitRepo_TagToUse)
        if ("${bv_juceGitRepoToUse}" STREQUAL "https://github.com/juce-framework/JUCE.git")
	        set (bv_juceGitRepo_TagToUse origin/develop)
		else()
	        set (bv_juceGitRepo_TagToUse origin)
		endif()
	endif()

	message (STATUS "Fetching the latest version of JUCE from GitHub repo: ${bv_juceGitRepoToUse} with tag: ${bv_juceGitRepo_TagToUse} ")

	include (FetchContent)

	FetchContent_Declare (juce
	GIT_REPOSITORY ${bv_juceGitRepoToUse}
	GIT_TAG        ${bv_juceGitRepo_TagToUse})

	FetchContent_MakeAvailable (juce)

	set (bv_juceDir ${CMAKE_CURRENT_LIST_DIR}/Builds/_deps/juce-src)
endif()

#

if (ANDROID)
    set (OBOE_DIR "../JUCE/modules/juce_audio_devices/native/oboe")
    add_subdirectory (${OBOE_DIR} ./oboe)

    add_library ("cpufeatures" STATIC "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
    set_source_files_properties ("${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c" PROPERTIES COMPILE_FLAGS "-Wno-sign-conversion -Wno-gnu-statement-expression")

    enable_language (ASM)
endif()

##################################################################################################

function (imogenConfig_step2)

if (ANDROID)
    find_library (log "log")
    find_library (android "android")
    find_library (glesv2 "GLESv2")
    find_library (egl "EGL")
    set (cpufeatures_lib "cpufeatures")
    set (oboe_lib "oboe")

    target_include_directories (${CMAKE_PROJECT_NAME} PRIVATE
        "${ANDROID_NDK}/sources/android/cpufeatures"
        "${OBOE_DIR}/include")

    target_link_libraries (${CMAKE_PROJECT_NAME} PUBLIC ${log} ${android} ${glesv2} ${egl} ${cpufeatures_lib} ${oboe_lib})
endif()

#

if (DEFINED BV_IGNORE_VDSP OR NOT APPLE)

    target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC JUCE_USE_VDSP_FRAMEWORK=0 BV_USE_VDSP=0)
    
    if (DEFINED BV_IGNORE_MIPP)
        target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC BV_USE_MIPP=0)
    else()
        message (STATUS "Configuring MIPP...")
		target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC MIPP_ENABLE_BACKTRACE BV_USE_MIPP=1)
    	target_include_directories (${CMAKE_PROJECT_NAME} PUBLIC "${Imogen_sourceDir}/../third-party/MIPP/src" "MIPP")
    endif()
else()
    target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC BV_USE_VDSP=1 JUCE_USE_VDSP_FRAMEWORK=1)
endif()

#

target_link_libraries (${CMAKE_PROJECT_NAME} PUBLIC
	    juce::juce_recommended_config_flags
	    juce::juce_recommended_lto_flags
	    juce::juce_recommended_warning_flags
)

target_compile_features (${CMAKE_PROJECT_NAME} PUBLIC cxx_std_17)

target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC 
    JUCE_STRICT_REFCOUNTEDPTR=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_MODAL_LOOPS_PERMITTED=0
    JUCE_LOAD_CURL_SYMBOLS_LAZILY=1
    )

if (${IMOGEN_BUILD_FOR_ELK})
    target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC 
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    )
else()
    target_compile_definitions (${CMAKE_PROJECT_NAME} PUBLIC 
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    )
endif()

endfunction()

