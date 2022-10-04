cmake_minimum_required (VERSION 3.20)
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


cmake_policy(SET CMP0091 NEW)
project(SmallEngine)
include("UserSettings.txt")

#Project Wide Settings
#All Options which change the ABI come here
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message("Detected MSVC")
    string(REGEX REPLACE "/W[3|4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_options(/wd26812)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("Detected Clang")
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)

find_package(Vulkan REQUIRED)

find_package(spirv-cross-core)

# SPIRV-Cross
if(NOT spirv-cross-core_FOUND)
    message("spriv-cross-core not found fetching from repo")
    include("${CMAKE_SOURCE_DIR}/vendor/SPIRV-Cross.cmake")
endif()

# GLFW
include("${CMAKE_SOURCE_DIR}/vendor/GLFW.cmake")

# GTest
include("${CMAKE_SOURCE_DIR}/vendor/GTest.cmake")

# Imgui
include("${CMAKE_SOURCE_DIR}/vendor/Imgui.cmake")

# VMA
include("${CMAKE_SOURCE_DIR}/vendor/VMA.cmake")

# SDL2
#include("${CMAKE_SOURCE_DIR}/vendor/SDL2.cmake")
# Would work, but why use it instead of GLFW

# Volk
include("${CMAKE_SOURCE_DIR}/vendor/Volk.cmake")

# FBX SDK
include("${CMAKE_SOURCE_DIR}/vendor/FBXSDK.cmake")

# STB
include("${CMAKE_SOURCE_DIR}/vendor/STB.cmake")

# ENTT
include("${CMAKE_SOURCE_DIR}/vendor/EnTT.cmake")

# ENTT Editor
include("${CMAKE_SOURCE_DIR}/vendor/EnTTEditor.cmake")

# Optick
add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/optick")

# TinyGLTF
add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/tinyGLTF")

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


#add_library(Third-Party-Dependencies STATIC ${THIRD_PARTY_SRC})
add_library(Engine STATIC ${VULKAN_SRC} ${GLFW_SRC} ${RENDERER_SRC} ${CORE_SRC} ${UTIL_SRC} ${MATH_SRC})
add_library(FBXReader STATIC ${FBXREADER_SRC})
target_include_directories(FBXReader PUBLIC ${FBXREADER_H})
target_link_libraries(FBXReader fbx z xml2)



add_dependencies(Engine shaders)

#add_executable(Demo ${DEMO_SRC})
#add_executable(Demo2 ${DEMO2_SRC})
add_executable(FBXTest ${FBXTest_SRC})
add_executable(RTTest ${RTTest_SRC})
add_executable(GLTFTest ${GLTFTest_SRC})
add_executable(HybridTest ${HybridTest_SRC})
add_executable(DDGITest ${DDGITest_SRC})

#set_target_properties(DDGITest PROPERTIES 
#    LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")

#target_link_libraries(Third-Party-Dependencies spirv-cross-core vma Vulkan::Vulkan volk::volk imgui fbx z xml2)
target_link_libraries(vma PUBLIC volk::volk)
#target_link_libraries(Engine glfw Vulkan::Vulkan spirv-cross-core vma volk::volk imgui OptickCore stb EnTT imgui_entt_entity_editor )
target_link_libraries(Engine PUBLIC glfw Vulkan::Vulkan volk::volk vma  spirv-cross-core  EnTT imgui_entt_entity_editor )
target_link_libraries(Engine PRIVATE imgui OptickCore stb)
#target_link_libraries(Demo Engine)
#target_link_libraries(Demo2 Engine)
target_link_libraries(FBXTest Engine FBXReader)
target_link_libraries(GLTFTest Engine tinygltf)
target_link_libraries(RTTest Engine FBXReader)
target_link_libraries(HybridTest Engine FBXReader)
target_link_libraries(DDGITest Engine FBXReader)

#target_precompile_headers(Engine PRIVATE include/VulkanWrapper/VulkanIncludes.h )

target_compile_options(Engine PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:GNU>:-Wall>
    $<$<CXX_COMPILER_ID:Clang>:-Wall>)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #target_compile_options(Main PUBLIC /WX /std:c++latest)
    #target_compile_options(Demo PRIVATE /WX)
    #target_compile_options(Third-Party-Dependencies PRIVATE /w)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #target_compile_options(Demo PRIVATE -W4)
    #target_compile_options(Third-Party-Dependencies PRIVATE -Wno-everything)
    #target_compile_options(Main PUBLIC -std=c++20 -Wall -Wextra)
endif()

target_compile_features(FBXReader PUBLIC 
    cxx_std_20
)
target_compile_features(Engine PUBLIC 
    cxx_std_20
)
if(WIN32)
    target_compile_definitions(Engine PUBLIC VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(Engine PUBLIC VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    target_compile_definitions(Engine PUBLIC VK_USE_PLATFORM_XLIB_KHR)
endif()

# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/test/local.cmake")
