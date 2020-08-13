cmake_minimum_required (VERSION 3.14)
include(FetchContent)
FetchContent_Declare(
    SPIRV-Cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross
    GIT_TAG        master
    GIT_SHALLOW    true
)
FetchContent_GetProperties(SPIRV-Cross)

if(NOT SPIRV-Cross_POPULATED)
    FetchContent_Populate(SPIRV-Cross)

    # Set any custom variables, etc. here, then
    # populate the content as part of this build
    set(SPIRV-Cross_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/SPIRV-Cross-src)
    set(SPIRV-Cross_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/SPIRV-Cross-build)
    #add_subdirectory(${SPIRVCross_SOURCE_DIR} EXCLUDE_FROM_ALL)
    add_subdirectory(${SPIRV-Cross_SOURCE_DIR} ${SPIRV-Cross_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()