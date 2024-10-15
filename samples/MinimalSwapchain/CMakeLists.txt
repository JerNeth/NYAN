add_subdirectory(shader EXCLUDE_FROM_ALL)

add_executable(MinimalSwapchain MinimalSwapchain.cpp)

#set_target_properties(MinimalSwapchain PROPERTIES 
#    LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")

target_link_libraries(MinimalSwapchain 
	PRIVATE 
		NYAN::Vulkan
		NYAN::Renderer 
		sample_swapchain_shaders
		volk::volk
		vma_implementation
		NYAN::Log
		NYAN::Data
		NYAN::SDLWrapper
)