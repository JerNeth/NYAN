cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SmallEngine)




# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)
find_package(Openmp REQUIRED)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})



# SPIRV-Cross
include("${CMAKE_SOURCE_DIR}/vendor/SPIRV-Cross.cmake")

if(MSVC)
    add_compile_options(/WX)
    add_compile_options(/MDd)
else()
    add_compile_options(-Wall -Wextra)
endif()

# GTest
include("${CMAKE_SOURCE_DIR}/vendor/GTest.cmake")

# GLFW
include("${CMAKE_SOURCE_DIR}/vendor/GLFW.cmake")

# Imgui
include("${CMAKE_SOURCE_DIR}/vendor/Imgui.cmake")

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

add_executable(Main ${SRC_CPP} ${IMGUI_SRC})
add_dependencies(Main shaders)

target_link_libraries(Main glfw Vulkan::Vulkan spirv-cross-core)

# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------

include("${CMAKE_SOURCE_DIR}/test/local.cmake")
