cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SmallEngine)

if(MSVC)
    #add_compile_options(/W4 /WX)
    add_compile_options(/MDd)
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

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

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
