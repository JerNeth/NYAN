cmake_minimum_required (VERSION 3.26)
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CheckIPOSupported)

cmake_policy(SET CMP0091 NEW)
project(NYAN
        VERSION 0.0.1
        LANGUAGES C CXX)
#include("UserSettings.txt")
#Project Wide Settings
#All Options which change the ABI come here
message("Detected ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    string(REGEX REPLACE "/W[3|4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_options(/wd26812)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)

#find_package(Vulkan REQUIRED COMPONENTS glslc SPIRV-Tools volk)
find_package(Vulkan REQUIRED COMPONENTS glslc)

# SPIRV-Cross
#if(NOT Vulkan_SPIRV-Tools_FOUND OR TRUE)
if(TRUE)
    message("spriv-cross-core not found fetching from repo")
    include("${CMAKE_SOURCE_DIR}/vendor/SPIRV-Cross.cmake")
    set(SPIRV_LIB spirv-cross-core)
else()
    #NOTE: requires cmake 3.24
    #NOTE: currently not working
    set(SPIRV_LIB Vulkan::SPIRV-Tools)
endif()

# GLFW
include("${CMAKE_SOURCE_DIR}/vendor/GLFW.cmake")

# GTest
include("${CMAKE_SOURCE_DIR}/vendor/GTest.cmake")

# Imgui
include("${CMAKE_SOURCE_DIR}/vendor/Imgui.cmake")

# VMA
include("${CMAKE_SOURCE_DIR}/vendor/VMA.cmake")

# Volk
include("${CMAKE_SOURCE_DIR}/vendor/Volk.cmake")

# STB
include("${CMAKE_SOURCE_DIR}/vendor/STB.cmake")

# ENTT
include("${CMAKE_SOURCE_DIR}/vendor/EnTT.cmake")

# ENTT Editor
add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/EnTTEditor")

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

add_subdirectory(shader)


# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------


#add_library(Third-Party-Dependencies STATIC ${THIRD_PARTY_SRC})
add_library(Engine STATIC "")

target_sources(Engine 
    PRIVATE ${VULKAN_SRC}
            ${GLFW_SRC} 
            ${RENDERER_SRC} 
            ${CORE_SRC} 
            ${UTIL_SRC} 
            ${MATH_SRC} 
            ${GLTFREADER_SRC}
    PUBLIC ${INCLUDE_DIR}
    )



#set_target_properties(DDGITest PROPERTIES 
#    LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")

#target_link_libraries(Third-Party-Dependencies spirv-cross-core vma Vulkan::Vulkan volk::volk imgui fbx z xml2)
target_link_libraries(vma_implementation PUBLIC volk::volk)
#target_link_libraries(Engine glfw Vulkan::Vulkan spirv-cross-core vma volk::volk imgui stb EnTT imgui_entt_entity_editor )
#target_link_libraries(Engine PUBLIC glfw Vulkan::Vulkan volk::volk vma ${SPIRV_LIB} EnTT imgui_entt_entity_editor imgui implot)

target_link_libraries(Engine PUBLIC glfw volk::volk vma_implementation ${SPIRV_LIB} EnTT imgui_entt_entity_editor imgui implot)
target_link_libraries(tinygltf PUBLIC stb)
target_link_libraries(Engine PRIVATE stb tinygltf )
target_link_libraries(Engine PUBLIC shaders)


check_ipo_supported(RESULT ipo_result OUTPUT output
                    LANGUAGES C CXX)
if(ipo_result)
  set_property(TARGET Engine PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO is not supported: ${output}")
endif()

target_compile_options(Engine PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:GNU>:-Wall>
    $<$<CXX_COMPILER_ID:Clang>:-W4 -Wno-braced-scalar-init>)
    
target_compile_definitions(Engine PUBLIC 
    $<$<CXX_COMPILER_ID:MSVC>:>
    $<$<CXX_COMPILER_ID:Clang>:__clang__>)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #target_compile_options(Main PUBLIC /WX /std:c++latest)
    #target_compile_options(Demo PRIVATE /WX)
    #target_compile_options(Third-Party-Dependencies PRIVATE /w)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #target_compile_options(Demo PRIVATE -W4)
    #target_compile_options(Third-Party-Dependencies PRIVATE -Wno-everything)
    #target_compile_options(Main PUBLIC -std=c++20 -Wall -Wextra)
endif()

#target_compile_features(FBXReader PUBLIC 
#    cxx_std_20
#)
target_compile_features(Engine PUBLIC 
    cxx_std_23
)
set_target_properties(Engine PROPERTIES 
    UNITY_BUILD on
)

target_link_options(Engine PUBLIC
    $<$<NOT:$<CONFIG:DEBUG>>:/INCREMENTAL:NO> # Disable incremental linking.
    $<$<NOT:$<CONFIG:DEBUG>>:/OPT:REF> # Remove unreferenced functions and data.
    $<$<NOT:$<CONFIG:DEBUG>>:/OPT:ICF> # Identical COMDAT folding.
    $<$<CONFIG:DEBUG>:/INCREMENTAL> # Do incremental linking
    $<$<CONFIG:DEBUG>:/OPT:NOREF> # No unreferenced data elimination.
    $<$<CONFIG:DEBUG>:/OPT:NOICF> # No Identical COMDAT folding.
)

if(WIN32)
    target_compile_definitions(Engine PUBLIC VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(Engine PUBLIC VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    target_compile_definitions(Engine PUBLIC VK_USE_PLATFORM_XLIB_KHR)
endif()

#target_precompile_headers(Engine PRIVATE include/VulkanWrapper/VulkanIncludes.h )


add_executable(Demo ${DEMO_SRC})
add_executable(SwapchainTest ${SWAPTCHAIN_TEST_SRC})
add_executable(HeadlessTest ${HEADLESS_TEST_SRC})

target_link_libraries(Demo Engine)
target_link_libraries(SwapchainTest Engine)
target_link_libraries(HeadlessTest Engine)


# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/test/local.cmake")
