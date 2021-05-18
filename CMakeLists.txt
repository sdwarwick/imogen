
cmake_minimum_required (VERSION 3.18) 

include (CMake/FindShared-code.cmake)

project (Imogen VERSION 0.0.1 LANGUAGES CXX) 

set_default_juce_options (${PROJECT_NAME})

######

set (Imogen_sourceDir ${CMAKE_CURRENT_LIST_DIR}/Source)

include (${Imogen_sourceDir}/imogenConfig.cmake)

#

set (Imogen_sourceFiles
    ${Imogen_Common_SourceFiles}
    ${Imogen_Processor_SourceFiles}
    ${Imogen_GUI_SourceFiles}
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.cpp
    ${Imogen_sourceDir}/GUI/Holders/Plugin_Editor/PluginEditor.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_StandaloneWrapper/StandaloneWrapper.h
    ${Imogen_sourceDir}/GUI/Holders/Plugin_StandaloneWrapper/StandaloneWrapper.cpp)

#

set (thingsToLinkTo "bv_SharedCode")

juce_add_module ("${Imogen_sourceDir}/DSP_modules/bv_Harmonizer")
list (APPEND thingsToLinkTo bv_Harmonizer)

juce_add_module ("${Imogen_sourceDir}/DSP_modules/bv_ImogenEngine")
list (APPEND thingsToLinkTo bv_ImogenEngine)

juce_add_module ("${Imogen_sourceDir}/DSP_modules/SynthBase/bv_SynthBase")
list (APPEND thingsToLinkTo "bv_SynthBase")

#

juce_add_plugin (Imogen
    ${Imogen_Common_Flags}
    ${Imogen_Plugin_Flags}
    PLUGIN_CODE                 Imgn
    DOCUMENT_BROWSER_ENABLED    TRUE
    BACKGROUND_AUDIO_ENABLED	FALSE # for iOS
    NEEDS_CURL                  TRUE
    NEEDS_WEB_BROWSER           TRUE
    )

#

if (TARGET Imogen_LV2)
    set_target_properties (Imogen PROPERTIES JUCE_LV2_URI https://github.com/benthevining/imogen)
endif()

#

target_sources (Imogen PRIVATE ${Imogen_sourceFiles})

source_group (TREE ${Imogen_sourceDir} PREFIX "" FILES ${Imogen_sourceFiles})

target_include_directories (Imogen PRIVATE ${Imogen_sourceDir})

#

add_binary_data_folder (${PROJECT_NAME} assets)

#

target_link_libraries (Imogen PRIVATE ${thingsToLinkTo})

#

if (${bv_use_AbletonLink})
	include (${CMAKE_CURRENT_LIST_DIR}/third-party/ableton-link/AbletonLinkConfig.cmake)
	target_link_libraries (Imogen PRIVATE Ableton::Link)
	target_compile_definitions (Imogen PUBLIC IMOGEN_USE_ABLETON_LINK=1)
else()
	target_compile_definitions (Imogen PUBLIC IMOGEN_USE_ABLETON_LINK=0)
endif()

#

if (${bv_use_MTS_ESP})
	target_include_directories (Imogen PUBLIC "third-party/MTS-ESP/Client" "MTS-ESP")
	target_compile_definitions (Imogen PUBLIC bvsb_USE_MTS_ESP=1)
else()
	target_compile_definitions (Imogen PUBLIC bvsb_USE_MTS_ESP=0)
endif()

#

imogenConfig_step2()

#

target_compile_definitions (Imogen PUBLIC 
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    IMOGEN_REMOTE_APP=0
    IMOGEN_HEADLESS=0
    )

#
