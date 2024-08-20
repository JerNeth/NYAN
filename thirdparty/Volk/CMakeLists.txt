cmake_minimum_required (VERSION 3.14)
include(FetchContent)
if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif()

if(NOT DEFINED repo_volk OR NOT EXISTS ${repo_volk})
	message("Volk not locally specified, fetching remote")
	set(repo_volk https://github.com/zeux/volk.git)
endif()
FetchContent_Declare(
	volk
	GIT_REPOSITORY ${repo_volk}
	GIT_TAG        vulkan-sdk-1.3.290
	GIT_SHALLOW    true
)
FetchContent_MakeAvailable(volk)

target_compile_definitions(volk INTERFACE 
    VK_NO_PROTOTYPES
)