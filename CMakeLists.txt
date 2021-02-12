cmake_minimum_required (VERSION 3.14)
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


project(SmallEngine)

#Project Wide Settings
#All Options which change the ABI come here
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message("Detected MSVC")
    #add_compile_options(/WX)
    #set(CMAKE_CXX_FLAGS_RELEASE "/MT")
    #set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    add_compile_options(/MTd /wd26812)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("Detected Clang")
    #set(CMAKE_CXX_FLAGS_RELEASE "-MT")
    #set(CMAKE_CXX_FLAGS_DEBUG "-MTd")
    add_compile_options(-MTd)
    #add_compile_options(-Wall -Wextra -Wno-error=documentation -Wno-error=reserved-id-macro -Wno-error=documentation-unknown-command)
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)
#find_package(Openmp REQUIRED)

find_package(Vulkan REQUIRED)


# SPIRV-Cross
include("${CMAKE_SOURCE_DIR}/vendor/SPIRV-Cross.cmake")

# GLFW
include("${CMAKE_SOURCE_DIR}/vendor/GLFW.cmake")

# GTest
include("${CMAKE_SOURCE_DIR}/vendor/GTest.cmake")

# Imgui
include("${CMAKE_SOURCE_DIR}/vendor/Imgui.cmake")

# SDL2
#include("${CMAKE_SOURCE_DIR}/vendor/SDL2.cmake")
# Would work, but why use it instead of GLFW

# Volk
include("${CMAKE_SOURCE_DIR}/vendor/Volk.cmake")


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


add_library(Third-Party-Dependencies STATIC ${THIRD_PARTY_SRC} ${IMGUI_SRC})
add_library(Engine STATIC ${VULKAN_SRC} ${GLFW_SRC} ${RENDERER_SRC} ${CORE_SRC})
add_dependencies(Engine shaders)

add_executable(Demo ${DEMO_SRC})# )
add_executable(Demo2 ${DEMO2_SRC})

target_link_libraries(Third-Party-Dependencies spirv-cross-core Vulkan::Vulkan volk)
target_link_libraries(Engine glfw Vulkan::Vulkan Third-Party-Dependencies)
target_link_libraries(Demo Engine)
target_link_libraries(Demo2 Engine)

target_precompile_headers(Engine PRIVATE include/VulkanWrapper/VulkanIncludes.h )

target_compile_options(Engine PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:GNU>:-Wall>
    $<$<CXX_COMPILER_ID:Clang>:-Wall>)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #target_compile_options(Main PUBLIC /WX /std:c++latest)
    target_compile_options(Demo PRIVATE /WX)
    target_compile_options(Third-Party-Dependencies PRIVATE /w)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(Demo PRIVATE -W4)
    target_compile_options(Third-Party-Dependencies PRIVATE -Wno-everything)
    #target_compile_options(Main PUBLIC -std=c++20 -Wall -Wextra)
endif()

target_compile_features(Engine PUBLIC 
    cxx_std_20
)
# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/test/local.cmake")
