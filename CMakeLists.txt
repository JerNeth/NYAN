cmake_minimum_required (VERSION 3.14)
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


project(SmallEngine)




# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)
#find_package(Openmp REQUIRED)

find_package(Vulkan REQUIRED)

#Project Wide Settings
#All Options which change the ABI come here
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message("Detected MSVC")
    #add_compile_options(/WX)
    add_compile_options(/MDd)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("Detected Clang")
    add_compile_options(-MDd)
    #add_compile_options(-Wall -Wextra -Wno-error=documentation -Wno-error=reserved-id-macro -Wno-error=documentation-unknown-command)
endif()
# SPIRV-Cross
include("${CMAKE_SOURCE_DIR}/vendor/SPIRV-Cross.cmake")

# GLFW
include("${CMAKE_SOURCE_DIR}/vendor/GLFW.cmake")

# GTest
include("${CMAKE_SOURCE_DIR}/vendor/GTest.cmake")

# Imgui
#include("${CMAKE_SOURCE_DIR}/vendor/Imgui.cmake")

# SDL2
#include("${CMAKE_SOURCE_DIR}/vendor/SDL2.cmake")
# Would work, but why use it instead of GLFW


# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/include/local.cmake")

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/src/local.cmake")

# ---------------------------------------------------------------------------
# Shaders
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/shader/local.cmake")


# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------

add_executable(Main ${SRC_CPP})# ${IMGUI_SRC})
add_dependencies(Main shaders)

target_link_libraries(Main glfw Vulkan::Vulkan spirv-cross-core )

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #target_compile_options(Main PUBLIC /WX /std:c++latest)
    target_compile_options(Main PUBLIC /WX)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(Main PUBLIC -std=c++20 -Wall -Wextra)
    #target_compile_options(Main PUBLIC -Wall -Wextra)
endif()
set_target_properties(Main PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS ON
)
target_compile_features(Main PUBLIC 
    cxx_std_20
)
# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/test/local.cmake")
