#include "Manager.h"

#include "LogicalDevice.h"

#include "Utility/Exceptions.h"

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

VkFence vulkan::FenceManager::request_raw_fence()
{
	if (m_fences.empty()) {
		VkFence fence;
		VkFenceCreateInfo fenceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		};
		vkCreateFence(r_device.get_device(), &fenceCreateInfo, r_device.get_allocator(), &fence);
		return  fence;
	}
	else {
		auto fence = m_fences.back();
		m_fences.pop_back();
		return fence;
	}
}

void vulkan::FenceManager::reset_fence(VkFence fence) {
	if (fence == VK_NULL_HANDLE)
		return;
	auto fence_ = fence;
	auto status = vkGetFenceStatus(r_device.get_device(), fence);
	assert(status == VK_SUCCESS);
	m_fences.push_back(fence_);
	if (status == VK_SUCCESS) {
		vkResetFences(r_device.get_device(), 1, &fence_);
	}
	else {
		throw Utility::VulkanException(status);
	}
}
vulkan::SemaphoreManager::~SemaphoreManager() noexcept
{
	for (auto semaphore : m_semaphores)
		vkDestroySemaphore(r_device.get_device(), semaphore, r_device.get_allocator());
}

VkSemaphore vulkan::SemaphoreManager::request_semaphore()
{
	if (m_semaphores.empty()) {
		VkSemaphore semaphore = VK_NULL_HANDLE;
		VkSemaphoreCreateInfo semaphoreCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.flags = 0
		};
		if (auto result = vkCreateSemaphore(r_device.get_device(), &semaphoreCreateInfo, r_device.get_allocator(), &semaphore); result != VK_SUCCESS) {
			throw Utility::VulkanException(result);
		}
		return semaphore;
	}
	else {
		auto semaphore = *m_semaphores.begin();
		m_semaphores.erase(m_semaphores.begin());
		return semaphore;
	}
}

void vulkan::SemaphoreManager::recycle_semaphore(VkSemaphore semaphore)
{
	if (semaphore == VK_NULL_HANDLE)
		return;
	m_semaphores.insert(semaphore);
}
