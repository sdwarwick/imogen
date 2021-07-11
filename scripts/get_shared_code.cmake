if (NOT DEFINED CPM_DOWNLOAD_LOCATION)
  set (CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
endif()

if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))

  message (STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  
  file (DOWNLOAD
       https://raw.githubusercontent.com/cpm-cmake/CPM.cmake/master/cmake/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION})

endif()

include (${CPM_DOWNLOAD_LOCATION})


CPMAddPackage(
        NAME Shared-code
        GIT_REPOSITORY https://github.com/benthevining/Shared-code.git
        GIT_TAG origin/main)