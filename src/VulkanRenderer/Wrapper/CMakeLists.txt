add_library(VulkanWrapper)

target_sources(VulkanWrapper
	PRIVATE
		AccelerationStructure.cpp
		Allocator.cpp
		Buffer.cpp
		CommandBuffer.cpp
		CommandPool.cpp
		DeletionQueue.cpp
		DescriptorPool.cpp
		DescriptorSet.cpp
		DescriptorSetLayout.cpp
		Fence.cpp
		Framebuffer.cpp
		Image.cpp
		ImageView.cpp
		Instance.cpp
		LogicalDevice.cpp
		Object.cpp
		PhysicalDevice.cpp
		Pipeline.cpp
		PipelineCache.cpp
		PipelineLayout.cpp
		Queue.cpp
		RenderPass.cpp
		Sampler.cpp
		Semaphore.cpp
		Shader.cpp
		Surface.cpp
		Swapchain.cpp
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		AccelerationStructure.cppm
		Allocator.cppm
		Buffer.cppm
		CommandBuffer.cppm
		CommandPool.cppm
		Common.cppm
		DeletionQueue.cppm
		DescriptorPool.cppm
		DescriptorSet.cppm
		DescriptorSetLayout.cppm
		Error.cppm
		Fence.cppm
		Formats.cppm
		Framebuffer.cppm
		Image.cppm
		ImageView.cppm
		Instance.cppm
		LogicalDevice.cppm
		LogicalDeviceWrapper.cppm
		ModuleSettings.cppm
		Object.cppm
		PhysicalDevice.cppm
		Pipeline.cppm
		PipelineCache.cppm
		PipelineLayout.cppm
		Queue.cppm
		RenderPass.cppm
		Sampler.cppm
		Semaphore.cppm
		Shader.cppm
		Surface.cppm
		Swapchain.cppm
		VulkanWrapper.cppm
)
	
target_compile_features(VulkanWrapper PUBLIC cxx_std_23)

add_library(NYAN::Vulkan ALIAS VulkanWrapper)

target_link_libraries(VulkanWrapper 
	PRIVATE 
		volk::volk
		vma_implementation
		spirv-cross-cpp
		Threads::Threads
)

target_link_libraries(VulkanWrapper 
	PUBLIC 
		NYAN::Assert
		NYAN::Log
		NYAN::Data
		magic_enum::magic_enum
)

if(WIN32)
    target_compile_definitions(VulkanWrapper PUBLIC VK_USE_PLATFORM_WIN32_KHR)
    target_compile_definitions(VulkanWrapper PRIVATE NOMINMAX)
elseif(APPLE)
    target_compile_definitions(VulkanWrapper PUBLIC VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
    target_compile_definitions(VulkanWrapper PUBLIC VK_USE_PLATFORM_XLIB_KHR)
endif()

check_ipo_supported(RESULT ipo_result OUTPUT output
                    LANGUAGES CXX)
if(ipo_result AND NOT CONFIG:DEBUG)
  #set_property(TARGET Engine PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  set_property(TARGET VulkanWrapper PROPERTY
    INTERPROCEDURAL_OPTIMIZATION
      $<$<NOT:$<CONFIG:DEBUG>>:TRUE> 
      $<$<CONFIG:DEBUG>:FALSE>
  )
else()
  message(WARNING "IPO is not supported: ${output}")
endif()

set_property(TARGET VulkanWrapper PROPERTY
    UNITY_BUILD
      $<$<NOT:$<CONFIG:DEBUG>>:ON> 
      $<$<CONFIG:DEBUG>:OFF>
)