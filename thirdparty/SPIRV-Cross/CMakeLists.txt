cmake_minimum_required (VERSION 3.14)
include(FetchContent)


find_package(SPIRV_Cross QUIET)

if(spirv_cross_shared_FOUND)
    set(SPIRV_LIB spirv-cross-shared)
else()
    message("spriv-cross-core not found fetching from repo")
	if(NOT DEFINED repo_SPIRV_Cross OR NOT EXISTS ${repo_SPIRV_Cross})
		message("Spirv-cross not locally specified, fetching remote")
		set(repo_SPIRV_Cross https://github.com/KhronosGroup/SPIRV-Cross)
	endif()
	FetchContent_Declare(
		SPIRV-Cross
		GIT_REPOSITORY ${repo_SPIRV_Cross}
		GIT_TAG        vulkan-sdk-1.3.275
		GIT_SHALLOW    true
	)
	FetchContent_GetProperties(SPIRV-Cross)

	FetchContent_MakeAvailable(SPIRV-Cross)
    #set(SPIRV_LIB spirv-cross-cpp)
endif()