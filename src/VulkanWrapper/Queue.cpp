#include "VulkanWrapper/Queue.hpp"

#include "Utility/Exceptions.h"

#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/PhysicalDevice.hpp"


vulkan::Queue::Queue(LogicalDevice& device, const Type type, const VkQueue queueHandle, const uint32_t queueFamilyIndex, const float priority) noexcept :
	VulkanObject(device, queueHandle),
	m_type(type),
	m_queueFamilyIndex(queueFamilyIndex),
	m_priority(priority)
{
	//Required for vkQueueSubmit2
	assert(r_device.get_physical_device().get_vulkan13_features().synchronization2);
}

std::expected<void, vulkan::Error> vulkan::Queue::wait_idle() const noexcept
{
	assert(m_handle);
	if (const auto result = vkQueueWaitIdle(m_handle); result != VK_SUCCESS)
	{
		return std::unexpected{vulkan::Error{ result }};
	}

	return {};
}

std::expected<void, vulkan::Error> vulkan::Queue::submit2(const std::span<VkSubmitInfo2> submits, const VkFence fence) const noexcept
{
	assert(m_handle);
	assert(submits.size() <= std::numeric_limits<uint32_t>::max());
	for (const auto& submit : submits) {
		assert(submit.waitSemaphoreInfoCount == 0 || submit.pWaitSemaphoreInfos != nullptr);
		assert(submit.signalSemaphoreInfoCount == 0 || submit.pSignalSemaphoreInfos != nullptr);
		assert(submit.commandBufferInfoCount == 0 || submit.pCommandBufferInfos != nullptr);
		assert(!submit.flags); //Queue not created with VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT
		for (uint32_t i = 0; i < submit.commandBufferInfoCount; ++i) {
			const auto& commandBufferInfo = submit.pCommandBufferInfos[i];
			assert(!commandBufferInfo.deviceMask);
			assert(commandBufferInfo.sType == VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO);
		}
		for (uint32_t i = 0; i < submit.signalSemaphoreInfoCount; ++i) {
			const auto& signal = submit.pSignalSemaphoreInfos[i];
			assert(signal.sType == VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO);
			assert(signal.pNext == nullptr);
			assert(signal.semaphore != VK_NULL_HANDLE);
			assert(!(signal.stageMask & VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR) || r_device.get_physical_device().get_extensions().ray_tracing_pipeline);
			assert(!(signal.stageMask & VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT) || r_device.get_physical_device().get_extensions().mesh_shader);
			assert(!(signal.stageMask & VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT) || r_device.get_physical_device().get_extensions().mesh_shader);//Strictly not spec, but mesh_shader covers both
		}
		for (uint32_t i = 0; i < submit.waitSemaphoreInfoCount; ++i) {
			const auto& wait = submit.pWaitSemaphoreInfos[i];
			assert(wait.sType == VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO);
			assert(wait.pNext == nullptr);
			assert(wait.semaphore != VK_NULL_HANDLE);
			assert(!(wait.stageMask & VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR) || r_device.get_physical_device().get_extensions().ray_tracing_pipeline);
			assert(!(wait.stageMask & VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT) || r_device.get_physical_device().get_extensions().mesh_shader);
			assert(!(wait.stageMask & VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT) || r_device.get_physical_device().get_extensions().mesh_shader);//Strictly not spec, but mesh_shader covers both
			for (uint32_t j = 0; j < submit.signalSemaphoreInfoCount; ++j) {
				const auto& signal = submit.pSignalSemaphoreInfos[j];
				if (signal.semaphore == wait.semaphore)
					assert((signal.value > wait.value) || (signal.value == 0 && wait.value == 0));
			}
		}
	}
	if (const auto result = vkQueueSubmit2(m_handle, static_cast<uint32_t>(submits.size()), submits.data(), fence); result != VK_SUCCESS)
	{
		return std::unexpected{vulkan::Error{ result }};
	}
	return {};
}

std::expected<void, vulkan::Error> vulkan::Queue::present(Swapchain& swapchain, std::span<VkSemaphore> waitSemaphores) const noexcept
{
	assert(m_presentCapable);
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		//.waitSemaphoreCount = semaphore != VK_NULL_HANDLE,
		//.pWaitSemaphores = &semaphore,
		//.swapchainCount = 1,
		//.pSwapchains = &m_vkHandle,
		//.pImageIndices = &m_swapchainImageIndex,
		.pResults = NULL
	};
	if (auto result = vkQueuePresentKHR(m_handle, &presentInfo); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			//destroy swapchain
			return std::unexpected{vulkan::Error{ result }};
		}
		else if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
		{
			return std::unexpected{vulkan::Error{ result }};
			//TODO handle this case if fullscreen support required.
			//throw Utility::VulkanException(result);
		}
		else if (result == VK_ERROR_DEVICE_LOST)
		{
			return std::unexpected{vulkan::Error{ result }};
			//throw Utility::DeviceLostException(std::format("vkQueuePresentKHR: {:#x}", reinterpret_cast<uint64_t>(m_handle)));
		}
		else if (result == VK_ERROR_SURFACE_LOST_KHR)
		{
			return std::unexpected{vulkan::Error{ result }};
			//throw Utility::SurfaceLostException(std::format("vkQueuePresentKHR: {:#x}", reinterpret_cast<uint64_t>(m_handle)));
		}
		else {
			return std::unexpected{vulkan::Error{ result }};
			//throw Utility::VulkanException(result);
		}
	}
	return {};
}

void vulkan::Queue::set_present_capable(const bool capable) noexcept
{
	m_presentCapable = capable;
}
