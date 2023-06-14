cmake_minimum_required (VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG        master
  GIT_SHALLOW    true
)
FetchContent_MakeAvailable(vma)


file(WRITE "${vma_SOURCE_DIR}/vk_mem_alloc.cpp" "#include \"vk_mem_alloc.h\"")

add_library(vma STATIC 
	"${vma_SOURCE_DIR}/vk_mem_alloc.cpp"
)
target_include_directories(vma SYSTEM PUBLIC
	"${vma_SOURCE_DIR}/include"
)
target_compile_definitions(vma PUBLIC 
    VMA_STATIC_VULKAN_FUNCTIONS=0
	VMA_DYNAMIC_VULKAN_FUNCTIONS=0
)
target_compile_definitions(vma PRIVATE 
    VMA_IMPLEMENTATION
)