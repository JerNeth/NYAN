cmake_minimum_required (VERSION 3.14)
include(FetchContent)
if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif()
    message("Not supported yet")
endif()

FetchContent_Declare(
  volk
  GIT_REPOSITORY https://github.com/zeux/volk.git
  GIT_TAG        master
  GIT_SHALLOW    true
)
FetchContent_MakeAvailable(volk)
