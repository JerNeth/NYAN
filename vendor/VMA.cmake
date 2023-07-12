cmake_minimum_required (VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG        master
  GIT_SHALLOW    true
)
#set(VMA_STATIC_VULKAN_FUNCTIONS OFF)
#set(VMA_DYNAMIC_VULKAN_FUNCTIONS OFF)
#FetchContent_MakeAvailable(vma)
FetchContent_GetProperties(vma)
if(NOT vma_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(vma)
endif()

file(WRITE "${vma_SOURCE_DIR}/vk_mem_alloc.cpp" "#include \"vk_mem_alloc.h\"")

add_library(vma_implementation STATIC 
	"${vma_SOURCE_DIR}/vk_mem_alloc.cpp"
)
target_include_directories(vma_implementation SYSTEM PUBLIC
	"${vma_SOURCE_DIR}/include"
)
target_compile_definitions(vma_implementation PUBLIC 
    VMA_STATIC_VULKAN_FUNCTIONS=0
	VMA_DYNAMIC_VULKAN_FUNCTIONS=0
)
target_compile_definitions(vma_implementation PRIVATE 
    VMA_IMPLEMENTATION
)