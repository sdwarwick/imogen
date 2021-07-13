macro (_bv_download_cpm)

  if (DEFINED ENV{CPM_SOURCE_CACHE})
    set (BV_CPM_PATH $ENV{CPM_SOURCE_CACHE}/CPM.cmake CACHE INTERNAL "Path to the CPM.cmake script")
  else()
    set (BV_CPM_PATH ${CMAKE_CURRENT_LIST_DIR}/CPM.cmake CACHE INTERNAL "Path to the CPM.cmake script")
  endif()

  if (NOT EXISTS ${BV_CPM_PATH})

    message (STATUS "Downloading CPM.cmake to ${BV_CPM_PATH}")
  
    file (DOWNLOAD
        https://raw.githubusercontent.com/cpm-cmake/CPM.cmake/master/cmake/CPM.cmake
        ${BV_CPM_PATH})

  endif()

  include (${BV_CPM_PATH})

endmacro()

#

if (DEFINED BV_CPM_PATH)
  if (EXISTS ${BV_CPM_PATH})
    include (${BV_CPM_PATH})
  else()
    _bv_download_cpm()
  endif()
else()
  _bv_download_cpm()
endif()

#

CPMAddPackage (
        NAME Shared-code
        GIT_REPOSITORY https://github.com/benthevining/Shared-code.git
        GIT_TAG origin/main)