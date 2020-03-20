cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SmallEngine)


if(MSVC)
    message("Compiler" ${MSVC_VERSION})
    add_compile_options(/WX)
    add_compile_options(/MDd)
    if (MSVC_VERSION GREATER_EQUAL "1900")
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        if (_cpp_latest_flag_supported)
            add_compile_options("/std:c++latest")
        endif()
    endif()
else()
    add_compile_options(-Wall -Wextra)
endif()


# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------



#find_package(Threads REQUIRED)
# GTest
include("${CMAKE_SOURCE_DIR}/vendor/GTest.cmake")

# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/include/local.cmake")


# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/src/local.cmake")

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/test/local.cmake")

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------

add_executable(Main ${SRC_CPP})
