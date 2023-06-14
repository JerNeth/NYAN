cmake_minimum_required (VERSION 3.14)
include(FetchContent)
FetchContent_Declare(
    SPIRV-Cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross
    GIT_TAG        sdk-1.3.250.0
    GIT_SHALLOW    true
)
FetchContent_GetProperties(SPIRV-Cross)

FetchContent_MakeAvailable(SPIRV-Cross)