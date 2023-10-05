#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "DeletionQueue.hpp"
#include "VulkanWrapper/DeletionQueue.hpp"

#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/DeviceWrapper.hpp"


namespace vulkan
{
	vulkan::DeletionQueue::DeletionQueue(LogicalDevice& device) noexcept :
		r_device(device)
	{
	}

	vulkan::DeletionQueue::DeletionQueue(DeletionQueue&& other) noexcept :
		r_device(other.r_device),
		m_currentEpoch(other.m_currentEpoch),
		m_deletionQueues(std::move(other.m_deletionQueues)),
		m_deletionEmpty(std::move(other.m_deletionEmpty))
	{
		for (size_t epoch = 0; epoch < numEpochs; ++epoch)
			m_deletionEmpty[epoch] = true;
	}

	vulkan::DeletionQueue& vulkan::DeletionQueue::operator=(DeletionQueue&& other) noexcept
	{
		if(this != std::addressof(other))
		{
			std::swap(m_currentEpoch, other.m_currentEpoch);
			std::swap(m_deletionQueues, other.m_deletionQueues);
			std::swap(m_deletionEmpty, other.m_deletionEmpty);
		}
		return *this;
	}

	vulkan::DeletionQueue::~DeletionQueue()
	{
		for (size_t epoch = 0; epoch < numEpochs; ++epoch)
			clear_epoch(epoch);
	}

	void DeletionQueue::advance_epoch() noexcept
	{
		m_currentEpoch = (m_currentEpoch + 1) % numEpochs;
		clear_epoch(m_currentEpoch);
	}

	void DeletionQueue::queue_image_view_deletion(VkImageView imageView) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.imageViews.push_back(imageView);
	}

	void DeletionQueue::queue_buffer_view_deletion(VkBufferView bufferView) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.bufferViews.push_back(bufferView);
	}

	void vulkan::DeletionQueue::queue_sampler_deletion(const VkSampler sampler) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.samplers.push_back(sampler);
	}
	void DeletionQueue::queue_image_deletion(VkImage image) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.images.push_back(image);
	}
	void DeletionQueue::queue_buffer_deletion(VkBuffer buffer) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.buffers.push_back(buffer);
	}
	void DeletionQueue::queue_acceleration_structure_deletion(VkAccelerationStructureKHR accelerationStructure) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.accelerationStructures.push_back(accelerationStructure);
	}
	void DeletionQueue::queue_framebuffer_deletion(VkFramebuffer framebuffer) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.framebuffers.push_back(framebuffer);
	}
	void DeletionQueue::queue_semaphore_deletion(VkSemaphore semaphore) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.semaphores.push_back(semaphore);
	}
	void DeletionQueue::queue_allocation_deletion(VmaAllocation allocation) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.allocations.push_back(allocation);
	}
	void DeletionQueue::queue_descriptor_pool_deletion(VkDescriptorPool descriptorPool) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.descriptorPools.push_back(descriptorPool);
	}
	void vulkan::DeletionQueue::queue_command_pool_deletion(VkCommandPool commandPool) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.commandPools.push_back(commandPool);
	}
	void DeletionQueue::queue_query_pool_deletion(VkQueryPool queryPool) noexcept
	{
		assert(m_currentEpoch < numEpochs);
		auto& currentQueue = m_deletionQueues[m_currentEpoch];
		m_deletionEmpty[m_currentEpoch] = false;
		currentQueue.queryPools.push_back(queryPool);
	}
	void vulkan::DeletionQueue::clear_epoch(size_t epoch) noexcept
	{
		assert(epoch < numEpochs);
		if(!m_deletionEmpty[epoch])
		{
			m_deletionEmpty[epoch] = true;

			auto& currentQueue = m_deletionQueues[epoch];

			for (auto imageView : currentQueue.imageViews)
				r_device.get_device().vkDestroyImageView(imageView, r_device.get_allocator());
			currentQueue.imageViews.clear();

			for (auto bufferView : currentQueue.bufferViews)
				r_device.get_device().vkDestroyBufferView(bufferView, r_device.get_allocator());
			currentQueue.bufferViews.clear();

			for (auto sampler : currentQueue.samplers)
				r_device.get_device().vkDestroySampler(sampler, r_device.get_allocator());
			currentQueue.samplers.clear();

			for (auto image : currentQueue.images)
				r_device.get_device().vkDestroyImage(image, r_device.get_allocator());
			currentQueue.images.clear();

			for (auto buffer : currentQueue.buffers)
				r_device.get_device().vkDestroyBuffer(buffer, r_device.get_allocator());
			currentQueue.buffers.clear();

			for (auto accelerationStructure : currentQueue.accelerationStructures)
				r_device.get_device().vkDestroyAccelerationStructureKHR(accelerationStructure, r_device.get_allocator());
			currentQueue.accelerationStructures.clear();

			for (auto framebuffer : currentQueue.framebuffers)
				r_device.get_device().vkDestroyFramebuffer(framebuffer, r_device.get_allocator());
			currentQueue.framebuffers.clear();

			for (auto semaphore : currentQueue.semaphores)
				r_device.get_device().vkDestroySemaphore(semaphore, r_device.get_allocator());
			currentQueue.semaphores.clear();

			for (auto allocation : currentQueue.allocations)
				r_device.get_vma_allocator().free_allocation(allocation);
			currentQueue.allocations.clear();

			for (auto pool : currentQueue.descriptorPools)
				r_device.get_device().vkDestroyDescriptorPool(pool, r_device.get_allocator());
			currentQueue.descriptorPools.clear();

			for (auto pool : currentQueue.commandPools)
				r_device.get_device().vkDestroyCommandPool(pool, r_device.get_allocator());
			currentQueue.commandPools.clear();

			for (auto pool : currentQueue.queryPools)
				r_device.get_device().vkDestroyQueryPool(pool, r_device.get_allocator());
			currentQueue.queryPools.clear();
		}
	}
}