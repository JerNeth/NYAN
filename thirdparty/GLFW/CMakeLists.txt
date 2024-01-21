cmake_minimum_required(VERSION 3.11)

include(FetchContent)


if(NOT DEFINED repo_glfw OR NOT EXISTS ${repo_glfw})
	message("GLFW not locally specified, fetching remote")
	set(repo_glfw https://github.com/glfw/glfw.git)
endif()
FetchContent_Declare(
	glfw
	GIT_REPOSITORY ${repo_glfw}
	GIT_TAG        master
	GIT_SHALLOW    true
)

set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

FetchContent_MakeAvailable(glfw)
#add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})