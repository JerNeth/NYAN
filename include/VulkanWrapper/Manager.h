#ifndef VKMANAGER_H
#define VKMANAGER_H
#pragma once
#include "VulkanIncludes.h"

namespace vulkan {
	class LogicalDevice;
	class FenceHandle;
	class FenceManager {
	public:
		FenceManager(LogicalDevice& device) : r_device(device) {

		}
		~FenceManager()noexcept;
		FenceManager(FenceManager&) = delete;
		FenceManager(FenceManager&& other) = delete;
		FenceHandle request_fence();
		void reset_fence(VkFence fence);
	private:
		LogicalDevice& r_device;
		std::vector<VkFence> m_fences;
	};
	class FenceHandle {
		friend class FenceManager;

	private:
		FenceHandle(FenceManager& manager, VkFence vkHandle) :
			m_vkHandle(vkHandle),
			r_manager(manager)
		{

		}
	public:
		FenceHandle(FenceManager& manager) :
			r_manager(manager)
		{

		}
		FenceHandle(FenceHandle&) = delete;
		FenceHandle(FenceHandle&& other) noexcept :
			m_vkHandle(other.m_vkHandle),
			r_manager(other.r_manager)
		{
			other.m_vkHandle = VK_NULL_HANDLE;
		}
		FenceHandle& operator=(FenceHandle&) = delete;
		FenceHandle& operator=(FenceHandle&& other) 
		{
			if (this != &other && &r_manager == &other.r_manager) {
				VkFence temp_handle = other.m_vkHandle;
				other.m_vkHandle = m_vkHandle;
				m_vkHandle = temp_handle;
			}
			return *this;
		}
		VkFence get_handle() const noexcept{
			return m_vkHandle;
		}
		void clear() {
			r_manager.reset_fence(m_vkHandle);
			m_vkHandle = VK_NULL_HANDLE;
		}
		operator bool()  const noexcept {
			return m_vkHandle != VK_NULL_HANDLE;
		}
		~FenceHandle() {
			r_manager.reset_fence(m_vkHandle);
		}
		operator VkFence() {
			return m_vkHandle;
		}
	private:
		VkFence m_vkHandle = VK_NULL_HANDLE;
		FenceManager& r_manager;
	};
	class SemaphoreManager {
	public:
		SemaphoreManager(LogicalDevice& device) : r_device(device) {

		}
		~SemaphoreManager()noexcept;
		SemaphoreManager(SemaphoreManager&) = delete;
		SemaphoreManager(SemaphoreManager&& other) = delete;
		VkSemaphore request_semaphore();
		void recycle_semaphore(VkSemaphore semaphore);
	private:
		LogicalDevice& r_device;
		std::vector<VkSemaphore> m_semaphores;
	};
}
#endif //VKMANAGER_H!