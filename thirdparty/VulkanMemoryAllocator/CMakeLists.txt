cmake_minimum_required (VERSION 3.14)
include(FetchContent)

find_package(VulkanMemoryAllocator QUIET)

if(NOT VulkanMemoryAllocator_FOUND)
	message("VulkanMemoryAllocator not installed, checking if locally specified")
	if(NOT DEFINED repo_vma OR NOT EXISTS ${repo_vma})
		message("repo_vma not locally specified, fetching remote")
		set(repo_vma https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git)
	else()
		message("repo_vma locally specified: " ${repo_vma})
	endif()
	FetchContent_Declare(
		vma
		GIT_REPOSITORY ${repo_vma}
		GIT_TAG        v3.1.0
		GIT_SHALLOW    true
	)
	FetchContent_GetProperties(vma)
	FetchContent_MakeAvailable(vma)
endif()

add_library(vma_implementation STATIC 
	vk_mem_alloc.cpp
)
target_include_directories(vma_implementation SYSTEM PUBLIC
	"${vma_SOURCE_DIR}/include"
)

target_link_libraries(vma_implementation PUBLIC GPUOpen::VulkanMemoryAllocator)