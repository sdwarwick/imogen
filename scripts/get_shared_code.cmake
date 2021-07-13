include (${CMAKE_CURRENT_LIST_DIR}/get_cpm.cmake)

CPMAddPackage (
        NAME Shared-code
        GIT_REPOSITORY https://github.com/benthevining/Shared-code.git
        GIT_TAG origin/main)