macro (download_cpm)
  set (BV_CPM_PATH ${CMAKE_CURRENT_LIST_DIR}/CPM.cmake CACHE INTERNAL "Path to the CPM.cmake script")

  message (STATUS "Downloading CPM.cmake to ${BV_CPM_PATH}")
  
  file (DOWNLOAD
       https://raw.githubusercontent.com/cpm-cmake/CPM.cmake/master/cmake/CPM.cmake
       ${BV_CPM_PATH})

  include (${BV_CPM_PATH})
endmacro()

#

if (DEFINED BV_CPM_PATH)
  if (EXISTS ${BV_CPM_PATH})
    include (${BV_CPM_PATH})
  else()
    download_cpm()
  endif()
else()
  download_cpm()
endif()