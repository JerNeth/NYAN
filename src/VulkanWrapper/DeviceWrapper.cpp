#include "VulkanWrapper/DeviceWrapper.hpp"

#include <cassert>

vulkan::LogicalDeviceWrapper::LogicalDeviceWrapper(const VkDevice device, const VkAllocationCallbacks* allocator) noexcept :
	m_handle(device),
	m_allocator(allocator)
{
	volkLoadDeviceTable(&m_table, m_handle);




	//VK_KHR_synchronization2 
	if (!m_table.vkCmdPipelineBarrier2)
		m_table.vkCmdPipelineBarrier2 = m_table.vkCmdPipelineBarrier2KHR;
	if (!m_table.vkCmdResetEvent2)
		m_table.vkCmdResetEvent2 = m_table.vkCmdResetEvent2KHR;
	if (!m_table.vkCmdSetEvent2)
		m_table.vkCmdSetEvent2 = m_table.vkCmdSetEvent2KHR;
	if (!m_table.vkCmdWaitEvents2)
		m_table.vkCmdWaitEvents2 = m_table.vkCmdWaitEvents2KHR;
	if (!m_table.vkCmdWriteTimestamp2)
		m_table.vkCmdWriteTimestamp2 = m_table.vkCmdWriteTimestamp2KHR;
	if (!m_table.vkQueueSubmit2)
		m_table.vkQueueSubmit2 = m_table.vkQueueSubmit2KHR;

	//VK_KHR_dynamic_rendering
	if (!m_table.vkCmdBeginRendering)
		m_table.vkCmdBeginRendering = m_table.vkCmdBeginRenderingKHR;
	if (!m_table.vkCmdEndRendering)
		m_table.vkCmdEndRendering = m_table.vkCmdEndRenderingKHR;

	//VK_KHR_maintenance4 
	if (!m_table.vkGetDeviceBufferMemoryRequirements)
		m_table.vkGetDeviceBufferMemoryRequirements = m_table.vkGetDeviceBufferMemoryRequirementsKHR;
	if (!m_table.vkGetDeviceImageMemoryRequirements)
		m_table.vkGetDeviceImageMemoryRequirements = m_table.vkGetDeviceImageMemoryRequirementsKHR;
	if (!m_table.vkGetDeviceImageSparseMemoryRequirements)
		m_table.vkGetDeviceImageSparseMemoryRequirements = m_table.vkGetDeviceImageSparseMemoryRequirementsKHR;

	//VK_EXT_extended_dynamic_state  
	if (!m_table.vkCmdBindVertexBuffers2)
		m_table.vkCmdBindVertexBuffers2 = m_table.vkCmdBindVertexBuffers2EXT;
	if (!m_table.vkCmdSetCullMode)
		m_table.vkCmdSetCullMode = m_table.vkCmdSetCullModeEXT;
	if (!m_table.vkCmdSetDepthBoundsTestEnable)
		m_table.vkCmdSetDepthBoundsTestEnable = m_table.vkCmdSetDepthBoundsTestEnableEXT;
	if (!m_table.vkCmdSetDepthCompareOp)
		m_table.vkCmdSetDepthCompareOp = m_table.vkCmdSetDepthCompareOpEXT;
	if (!m_table.vkCmdSetDepthTestEnable)
		m_table.vkCmdSetDepthTestEnable = m_table.vkCmdSetDepthTestEnableEXT;
	if (!m_table.vkCmdSetDepthWriteEnable)
		m_table.vkCmdSetDepthWriteEnable = m_table.vkCmdSetDepthWriteEnableEXT;
	if (!m_table.vkCmdSetFrontFace)
		m_table.vkCmdSetFrontFace = m_table.vkCmdSetFrontFaceEXT;
	if (!m_table.vkCmdSetPrimitiveTopology)
		m_table.vkCmdSetPrimitiveTopology = m_table.vkCmdSetPrimitiveTopologyEXT;
	if (!m_table.vkCmdSetScissorWithCount)
		m_table.vkCmdSetScissorWithCount = m_table.vkCmdSetScissorWithCountEXT;
	if (!m_table.vkCmdSetStencilOp)
		m_table.vkCmdSetStencilOp = m_table.vkCmdSetStencilOpEXT;
	if (!m_table.vkCmdSetStencilTestEnable)
		m_table.vkCmdSetStencilTestEnable = m_table.vkCmdSetStencilTestEnableEXT;
	if (!m_table.vkCmdSetViewportWithCount)
		m_table.vkCmdSetViewportWithCount = m_table.vkCmdSetViewportWithCountEXT;

	//VK_EXT_extended_dynamic_state2   
	if (!m_table.vkCmdSetDepthBiasEnable)
		m_table.vkCmdSetDepthBiasEnable = m_table.vkCmdSetDepthBiasEnableEXT;
	if (!m_table.vkCmdSetPrimitiveRestartEnable)
		m_table.vkCmdSetPrimitiveRestartEnable = m_table.vkCmdSetPrimitiveRestartEnableEXT;
	if (!m_table.vkCmdSetRasterizerDiscardEnable)
		m_table.vkCmdSetRasterizerDiscardEnable = m_table.vkCmdSetRasterizerDiscardEnableEXT;

	//VK_EXT_private_data    
	if (!m_table.vkCreatePrivateDataSlot)
		m_table.vkCreatePrivateDataSlot = m_table.vkCreatePrivateDataSlotEXT;
	if (!m_table.vkDestroyPrivateDataSlot)
		m_table.vkDestroyPrivateDataSlot = m_table.vkDestroyPrivateDataSlotEXT;
	if (!m_table.vkGetPrivateData)
		m_table.vkGetPrivateData = m_table.vkGetPrivateDataEXT;
	if (!m_table.vkSetPrivateData)
		m_table.vkSetPrivateData = m_table.vkSetPrivateDataEXT;

	//VK_KHR_copy_commands2    
	if (!m_table.vkCmdBlitImage2)
		m_table.vkCmdBlitImage2 = m_table.vkCmdBlitImage2KHR;
	if (!m_table.vkCmdCopyBuffer2)
		m_table.vkCmdCopyBuffer2 = m_table.vkCmdCopyBuffer2KHR;
	if (!m_table.vkCmdCopyBufferToImage2)
		m_table.vkCmdCopyBufferToImage2 = m_table.vkCmdCopyBufferToImage2KHR;
	if (!m_table.vkCmdCopyImage2)
		m_table.vkCmdCopyImage2 = m_table.vkCmdCopyImage2KHR;
	if (!m_table.vkCmdCopyImageToBuffer2)
		m_table.vkCmdCopyImageToBuffer2 = m_table.vkCmdCopyImageToBuffer2KHR;
	if (!m_table.vkCmdResolveImage2)
		m_table.vkCmdResolveImage2 = m_table.vkCmdResolveImage2KHR;

	//VK_KHR_buffer_device_address 
	if (!m_table.vkGetBufferDeviceAddress)
		m_table.vkGetBufferDeviceAddress = m_table.vkGetBufferDeviceAddressKHR;
	if (!m_table.vkGetBufferOpaqueCaptureAddress)
		m_table.vkGetBufferOpaqueCaptureAddress = m_table.vkGetBufferOpaqueCaptureAddressKHR;
	if (!m_table.vkGetDeviceMemoryOpaqueCaptureAddress)
		m_table.vkGetDeviceMemoryOpaqueCaptureAddress = m_table.vkGetDeviceMemoryOpaqueCaptureAddressKHR;

	//VK_KHR_create_renderpass2  
	if (!m_table.vkCmdBeginRenderPass2)
		m_table.vkCmdBeginRenderPass2 = m_table.vkCmdBeginRenderPass2KHR;
	if (!m_table.vkCmdEndRenderPass2)
		m_table.vkCmdEndRenderPass2 = m_table.vkCmdEndRenderPass2KHR;
	if (!m_table.vkCmdNextSubpass2)
		m_table.vkCmdNextSubpass2 = m_table.vkCmdNextSubpass2KHR;
	if (!m_table.vkCreateRenderPass2)
		m_table.vkCreateRenderPass2 = m_table.vkCreateRenderPass2KHR;

	//VK_KHR_draw_indirect_count 
	if (!m_table.vkCmdDrawIndexedIndirectCount)
		m_table.vkCmdDrawIndexedIndirectCount = m_table.vkCmdDrawIndexedIndirectCountKHR;
	if (!m_table.vkCmdDrawIndirectCount)
		m_table.vkCmdDrawIndirectCount = m_table.vkCmdDrawIndirectCountKHR;

	//VK_KHR_timeline_semaphore  
	if (!m_table.vkGetSemaphoreCounterValue)
		m_table.vkGetSemaphoreCounterValue = m_table.vkGetSemaphoreCounterValueKHR;
	if (!m_table.vkSignalSemaphore)
		m_table.vkSignalSemaphore = m_table.vkSignalSemaphoreKHR;
	if (!m_table.vkWaitSemaphores)
		m_table.vkWaitSemaphores = m_table.vkWaitSemaphoresKHR;

	//VK_EXT_host_query_reset
	if (!m_table.vkResetQueryPool)
		m_table.vkResetQueryPool = m_table.vkResetQueryPoolEXT;

}

vulkan::LogicalDeviceWrapper::~LogicalDeviceWrapper() noexcept
{
	if (m_handle != VK_NULL_HANDLE) {
		vkDestroyDevice(m_allocator);
		m_handle = VK_NULL_HANDLE;
	}
}

vulkan::LogicalDeviceWrapper::LogicalDeviceWrapper(LogicalDeviceWrapper&& other) noexcept :
	m_table(other.m_table),
	m_allocator(other.m_allocator)
{
	m_handle = other.m_handle;
	other.m_handle = VK_NULL_HANDLE;
}

vulkan::LogicalDeviceWrapper& vulkan::LogicalDeviceWrapper::operator=(LogicalDeviceWrapper&& other) noexcept
{
	if(this != &other)
	{
		m_handle = other.m_handle;
		other.m_handle = VK_NULL_HANDLE;
		m_allocator = other.m_allocator;
		m_table = other.m_table;
	}
	return *this;
}

VkResult vulkan::LogicalDeviceWrapper::vmaCreateAllocator(VmaAllocatorCreateFlags flags,
	VkPhysicalDevice physicalDevice, const VkAllocationCallbacks* pAllocationCallbacks, VkInstance instance,
	uint32_t apiVersion, VmaAllocator* pAllocator) const noexcept
{
	VmaVulkanFunctions vulkanFunctions{
		.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
		.vkGetDeviceProcAddr = vkGetDeviceProcAddr,
		.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
		.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
		.vkAllocateMemory = m_table.vkAllocateMemory,
		.vkFreeMemory = m_table.vkFreeMemory,
		.vkMapMemory = m_table.vkMapMemory,
		.vkUnmapMemory = m_table.vkUnmapMemory,
		.vkFlushMappedMemoryRanges = m_table.vkFlushMappedMemoryRanges,
		.vkInvalidateMappedMemoryRanges = m_table.vkInvalidateMappedMemoryRanges,
		.vkBindBufferMemory = m_table.vkBindBufferMemory,
		.vkBindImageMemory = m_table.vkBindImageMemory,
		.vkGetBufferMemoryRequirements = m_table.vkGetBufferMemoryRequirements,
		.vkGetImageMemoryRequirements = m_table.vkGetImageMemoryRequirements,
		.vkCreateBuffer = m_table.vkCreateBuffer,
		.vkDestroyBuffer = m_table.vkDestroyBuffer,
		.vkCreateImage = m_table.vkCreateImage,
		.vkDestroyImage = m_table.vkDestroyImage,
		.vkCmdCopyBuffer = m_table.vkCmdCopyBuffer,
		.vkGetBufferMemoryRequirements2KHR = m_table.vkGetBufferMemoryRequirements2,
		.vkGetImageMemoryRequirements2KHR = m_table.vkGetImageMemoryRequirements2,
		.vkBindBufferMemory2KHR = m_table.vkBindBufferMemory2,
		.vkBindImageMemory2KHR = m_table.vkBindImageMemory2,
		.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2,
		.vkGetDeviceBufferMemoryRequirements = m_table.vkGetDeviceBufferMemoryRequirements,
		.vkGetDeviceImageMemoryRequirements = m_table.vkGetDeviceImageMemoryRequirements,
	};
	assert(m_handle);
	assert(instance);
	assert(physicalDevice);
	const VmaAllocatorCreateInfo allocatorInfo{
		.flags = flags,
		.physicalDevice = physicalDevice,
		.device = m_handle,
		.pAllocationCallbacks = pAllocationCallbacks,
		.pVulkanFunctions = &vulkanFunctions,
		.instance = instance,
		.vulkanApiVersion = apiVersion,
	};
	return ::vmaCreateAllocator(&allocatorInfo, pAllocator);
}
