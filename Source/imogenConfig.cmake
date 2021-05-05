
if (NOT DEFINED IMOGEN_BUILD_FOR_ELK)
    set (IMOGEN_BUILD_FOR_ELK FALSE)
endif()

if (DEFINED bv_ignoreAllThirdPartyLibs)
    set (BV_IGNORE_VDSP)
    set (BV_IGNORE_MIPP)
endif()

#

set (ImogenIconPath ${Imogen_sourceDir}/../assets/graphics/imogen_icon.png)

set (Imogen_Common_SourceFiles
    ${Imogen_sourceDir}/Common/ImogenCommon.h
    ${Imogen_sourceDir}/Common/ImogenParameters.h
    ${Imogen_sourceDir}/Common/ImogenState.h
    )

set (Imogen_Processor_SourceFiles
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessor.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessorParameters.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessorState.cpp
    ${Imogen_sourceDir}/PluginProcessor/PluginProcessor.h)

set (Imogen_GUI_SourceFiles
    ${Imogen_sourceDir}/GUI/GUI_Framework.h
    ${Imogen_sourceDir}/GUI/ImogenGUI.h
    ${Imogen_sourceDir}/GUI/ImogenGUI.cpp
    ${Imogen_sourceDir}/GUI/Holders/ImogenGuiHolder.h
    ${Imogen_sourceDir}/GUI/MainDialComponent/MainDialComponent.h
    ${Imogen_sourceDir}/GUI/MainDialComponent/MainDialComponent.cpp
    ${Imogen_sourceDir}/GUI/LookAndFeel/ImogenLookAndFeel.h
    ${Imogen_sourceDir}/GUI/LookAndFeel/ImogenLookAndFeel.cpp)

#

set (Imogen_assetFiles
	${ImogenIconPath}
	)

#

set (Imogen_Common_Flags
    PRODUCT_NAME                ${CMAKE_PROJECT_NAME}
    VERSION                     ${CMAKE_PROJECT_VERSION}
    BUNDLE_ID                   com.BenViningMusicSoftware.Imogen
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    ICON_BIG                    ${ImogenIconPath}
    COMPANY_NAME                BenViningMusicSoftware
    COMPANY_WEBSITE             www.benvining.com
    COMPANY_EMAIL               ben.the.vining@gmail.com
    COMPANY_COPYRIGHT           "This software is provided as-is, with no guarantee of completion or fitness for any particular purpose, by Ben Vining, under the terms and conditions of the GNU Public License."
    STATUS_BAR_HIDDEN           TRUE  # for iOS
    REQUIRES_FULL_SCREEN        TRUE  # for iOS
    IPAD_SCREEN_ORIENTATIONS    UIInterfaceOrientationUnknown, UIInterfaceOrientationLandscapeLeft, UIInterfaceOrientationLandscapeRight
    TARGETED_DEVICE_FAMILY      2     # target iPad only
    DOCUMENT_EXTENSIONS         xml
    SEND_APPLE_EVENTS_PERMISSION_ENABLED FALSE
    )

set (Imogen_Plugin_Flags
    PLUGIN_NAME                     Imogen
    PLUGIN_MANUFACTURER_CODE        Benv
    DESCRIPTION                     "Real-time vocal harmonizer instrument"
    IS_SYNTH                        FALSE
    IS_MIDI_EFFECT                  FALSE
    DISABLE_AAX_MULTI_MONO          TRUE
    VST_NUM_MIDI_INS                1
    VST_NUM_MIDI_OUTS               1
    VST3_CATEGORIES                 "Pitch Shift"
    AU_MAIN_TYPE                    "kAudioUnitType_MusicEffect"
    FORMATS                         ${bv_formats}
    MICROPHONE_PERMISSION_ENABLED   TRUE
    MICROPHONE_PERMISSION_TEXT      "Imogen requires audio input to be able to produce its output. Please enable the microphone, or you won't hear anything when you press the keys."
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

