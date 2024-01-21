#pragma once

#include <vector>
#include <array>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"

namespace vulkan
{
	class DeletionQueue
	{
	private:
		struct DeletionData
		{
			std::vector<VkImageView> imageViews;
			std::vector<VkBufferView> bufferViews;
			std::vector<VkSampler> samplers;
			std::vector<VkImage> images;
			std::vector<VkBuffer> buffers;
			std::vector<VkAccelerationStructureKHR> accelerationStructures;
			std::vector<VkFramebuffer> framebuffers;
			std::vector<VkSemaphore> semaphores;
			std::vector<VmaAllocation> allocations;
			std::vector<VkDescriptorPool> descriptorPools;
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
			std::vector<VkCommandPool> commandPools;
			std::vector<VkQueryPool> queryPools;
		};
	public:
		explicit DeletionQueue(LogicalDevice& device) noexcept;

		DeletionQueue(DeletionQueue&) = delete;
		DeletionQueue& operator=(DeletionQueue&) = delete;

		DeletionQueue(DeletionQueue&& other) noexcept;
		DeletionQueue& operator=(DeletionQueue&& other) noexcept;
		~DeletionQueue();

		void advance_epoch() noexcept;

		void queue_image_view_deletion(VkImageView imageView) noexcept;
		void queue_buffer_view_deletion(VkBufferView bufferView) noexcept;
		void queue_sampler_deletion(VkSampler sampler) noexcept;
		void queue_image_deletion(VkImage image) noexcept;
		void queue_buffer_deletion(VkBuffer buffer) noexcept;
		void queue_acceleration_structure_deletion(VkAccelerationStructureKHR accelerationStructure) noexcept;
		void queue_framebuffer_deletion(VkFramebuffer framebuffer) noexcept;
		void queue_semaphore_deletion(VkSemaphore semaphore) noexcept;
		void queue_allocation_deletion(VmaAllocation allocation) noexcept;
		void queue_descriptor_pool_deletion(VkDescriptorPool descriptorPool) noexcept;
		void queue_descriptor_set_layout_deletion(VkDescriptorSetLayout descriptorSetLayout) noexcept;
		void queue_command_pool_deletion(VkCommandPool commandPool) noexcept;
		void queue_query_pool_deletion(VkQueryPool queryPool) noexcept;
	private:
		void clear_epoch(size_t epoch) noexcept;

		static constexpr size_t numEpochs = 2;
		LogicalDevice& r_device;
		size_t m_currentEpoch{ 0 };
		std::array<bool, numEpochs> m_deletionEmpty{};
		std::array<DeletionData, numEpochs> m_deletionQueues{};
	};
	
}
