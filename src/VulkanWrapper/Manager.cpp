#include "..\..\include\VulkanWrapper\Manager.h"
#include "..\..\include\VulkanWrapper\Manager.h"
#include "Manager.h"
#include "LogicalDevice.h"

vulkan::FenceManager::~FenceManager() noexcept {
	for (auto fence : m_fences) {
		vkDestroyFence(r_device.get_device(), fence, r_device.get_allocator());
	}
}

vulkan::FenceHandle vulkan::FenceManager::request_fence()
{
	if (m_fences.empty()) {
		VkFence fence;
		VkFenceCreateInfo fenceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		};
		vkCreateFence(r_device.get_device(), &fenceCreateInfo, r_device.get_allocator(), &fence);
		return FenceHandle(*this, fence);
	}
	else {
		auto fence = m_fences.back();
		m_fences.pop_back();
		return FenceHandle(*this, fence);
	}
	
}

void vulkan::FenceManager::reset_fence(VkFence fence) {
	if (fence == VK_NULL_HANDLE)
		return;
	auto fence_ = fence;
	vkResetFences(r_device.get_device(), 1, &fence_);
	m_fences.push_back(fence_);
}
vulkan::SemaphoreManager::~SemaphoreManager() noexcept
{
	for (auto semaphore : m_semaphores) {
		vkDestroySemaphore(r_device.get_device(), semaphore, r_device.get_allocator());
	}
}

VkSemaphore vulkan::SemaphoreManager::request_semaphore()
{
	if (m_semaphores.empty()) {
		VkSemaphore semaphore;
		VkSemaphoreCreateInfo semaphoreCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};
		if (auto result = vkCreateSemaphore(r_device.get_device(), &semaphoreCreateInfo, r_device.get_allocator(), &semaphore); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not create Semaphore, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not create Semaphore, out of device memory");
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
		return semaphore;
	}
	else {
		auto semaphore = m_semaphores.back();
		m_semaphores.pop_back();
		return semaphore;
	}
}

void vulkan::SemaphoreManager::recycle_semaphore(VkSemaphore semaphore)
{
	if (semaphore == VK_NULL_HANDLE)
		return;
	m_semaphores.push_back(semaphore);
}
