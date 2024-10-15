cmake_minimum_required (VERSION 3.14)
include(FetchContent)
if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif()


CPMAddPackage(NAME volk
	GITHUB_REPOSITORY zeux/volk
	GIT_TAG	vulkan-sdk-${Vulkan_VERSION}
)

target_compile_definitions(volk INTERFACE 
    VK_NO_PROTOTYPES
)