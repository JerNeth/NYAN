cmake_minimum_required (VERSION 3.14)

CPMAddPackage(NAME vma
	GITHUB_REPOSITORY  GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
	VERSION 3.1.0
	DOWNLOAD_ONLY YES
)

add_library(vma_implementation STATIC 
	vk_mem_alloc.cpp
)
target_include_directories(vma_implementation SYSTEM PUBLIC
	"${vma_SOURCE_DIR}/include"
)
