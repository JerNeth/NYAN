add_library(VulkanWrapper)

target_sources(VulkanWrapper
	PRIVATE
		"AccelerationStructure.cpp"
		"Allocator.cpp"
		"Buffer.cpp"
		"CommandBuffer.cpp"
		"CommandPool.cpp"
		"DeletionQueue.cpp"
		"DescriptorPool.cpp"
		"DescriptorSet.cpp"
		"DescriptorSetLayout.cpp"
		"Image.cpp"
		"Instance.cpp"
		"LogicalDevice.cpp"
		"Object.cpp"
		"PhysicalDevice.cpp"
		"Pipeline.cpp"
		"PipelineCache.cpp"
		"PipelineLayout.cpp" 
		"Queue.cpp"
		"Sampler.cpp"
		"Shader.cpp"
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		"AccelerationStructure.ixx"
		"Allocator.ixx" 
		"Buffer.ixx"
		"CommandBuffer.ixx" 
		"CommandPool.ixx"
		"DeletionQueue.ixx" 
		"DescriptorPool.ixx" 
		"DescriptorSet.ixx" 
		"DescriptorSetLayout.ixx"
		"Error.ixx" 
		"Image.ixx"  
		"Instance.ixx"  
		"LogicalDevice.ixx" 
		"LogicalDeviceWrapper.ixx"
		"Object.ixx"
		"PhysicalDevice.ixx" 
		"Pipeline.ixx" 
		"PipelineCache.ixx"
		"PipelineLayout.ixx" 
		"Queue.ixx" 
		"Sampler.ixx"
		"Shader.ixx"
		"VulkanWrapper.ixx"
)
	
target_compile_features(VulkanWrapper PUBLIC cxx_std_23)

add_library(NYAN::VulkanWrapper ALIAS VulkanWrapper)

target_link_libraries(VulkanWrapper 
	PRIVATE 
		volk::volk
		vma_implementation
		spirv-cross-cpp
		magic_enum::magic_enum
		Threads::Threads
)
target_link_libraries(VulkanWrapper 
	PRIVATE 
		NYAN::Log
		NYAN::Data
)

if(WIN32)
    target_compile_definitions(VulkanWrapper PUBLIC VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(VulkanWrapper PUBLIC VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    target_compile_definitions(VulkanWrapper PUBLIC VK_USE_PLATFORM_XLIB_KHR)
endif()