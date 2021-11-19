cmake_minimum_required (VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  VMA
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG        master
  GIT_SHALLOW    true
)
FetchContent_MakeAvailable(VMA)
