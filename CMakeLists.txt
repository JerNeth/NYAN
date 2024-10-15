cmake_minimum_required (VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  # This specific value changes as experimental support evolves. See
  # `Help/dev/experimental.rst` in the CMake source corresponding to
  # your CMake build for the exact value to use.
  "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

include(CheckIPOSupported)

set(CPM_DOWNLOAD_VERSION 0.40.2) 
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()

include(${CPM_DOWNLOAD_LOCATION})

cmake_policy(SET CMP0168 NEW)
cmake_policy(SET CMP0091 NEW)
project(NYAN
        VERSION 0.0.1
        LANGUAGES CXX C)

# Tell CMake that we explicitly want `import std`. This will initialize the
# property on all targets declared after this to 1
set(CMAKE_CXX_MODULE_STD 1)

#Project Wide Settings
#All Options which change the ABI come here
message("Detected ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    string(REGEX REPLACE "/W[3|4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_options(/wd26812)
    add_compile_options(/DELAY:NOBIND)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
endif()



# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

option(BUILD_TESTS "Build tests" ON)
option(BUILD_SAMPLES "Build samples" ON)


# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)

find_package(Vulkan REQUIRED COMPONENTS glslc)

add_subdirectory(thirdparty)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

add_subdirectory(src) 

# ---------------------------------------------------------------------------
# Shaders
# ---------------------------------------------------------------------------

add_subdirectory(shader)

# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------


target_link_libraries(vma_implementation 
        PUBLIC volk::volk)

# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------

if (BUILD_TESTS)
#Has to be top level to be discovered by VS Test Explorer
    enable_testing()
    add_subdirectory(tests)
endif ()

# ---------------------------------------------------------------------------
# Samples
# ---------------------------------------------------------------------------

if (BUILD_SAMPLES)
    add_subdirectory(samples)
endif ()