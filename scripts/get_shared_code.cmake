if (NOT DEFINED BV_CPM_PATH)
  set (BV_CPM_PATH "${CMAKE_BINARY_DIR}/cmake/CPM.cmake" CACHE INTERNAL "Path to the CPM.cmake script")
endif()

if (NOT (EXISTS ${BV_CPM_PATH}))

  message (STATUS "Downloading CPM.cmake to ${BV_CPM_PATH}")
  
  file (DOWNLOAD
       https://raw.githubusercontent.com/cpm-cmake/CPM.cmake/master/cmake/CPM.cmake
       ${BV_CPM_PATH})

endif()

include (${BV_CPM_PATH})


CPMAddPackage(
        NAME Shared-code
        GIT_REPOSITORY https://github.com/benthevining/Shared-code.git
        GIT_TAG origin/main)