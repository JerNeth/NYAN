module;

//#include <expected>
//#include <vector>
//#include <array>

#include "volk.h"
#include "vk_mem_alloc.h"

export module NYANVulkan:DeletionQueue;
import std;
import :LogicalDeviceWrapper;
import :Allocator;

export namespace nyan::vulkan
{
	class LogicalDevice;
	class DeletionQueue
	{
	public:
		friend class LogicalDevice;
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
			std::vector<VkPipeline> pipelines;
			std::vector<VkPipelineLayout> pipelineLayouts;
			std::vector<VkRenderPass> renderPasses;
		};
	public:

		explicit DeletionQueue(LogicalDeviceWrapper& device, Allocator& allocator) noexcept;

		DeletionQueue(DeletionQueue&) = delete;
		DeletionQueue& operator=(DeletionQueue&) = delete;

		DeletionQueue(DeletionQueue&& other) noexcept;
		DeletionQueue& operator=(DeletionQueue&& other) noexcept;
		~DeletionQueue();

		void advance_epoch() noexcept;

		void queue_deletion(VkImageView imageView) noexcept;
		void queue_deletion(VkBufferView bufferView) noexcept;
		void queue_deletion(VkSampler sampler) noexcept;
		void queue_deletion(VkImage image) noexcept;
		void queue_deletion(VkBuffer buffer) noexcept;
		void queue_deletion(VkAccelerationStructureKHR accelerationStructure) noexcept;
		void queue_deletion(VkFramebuffer framebuffer) noexcept;
		void queue_deletion(VkSemaphore semaphore) noexcept;
		void queue_deletion(VmaAllocation allocation) noexcept;
		void queue_deletion(VkDescriptorPool descriptorPool) noexcept;
		void queue_deletion(VkDescriptorSetLayout descriptorSetLayout) noexcept;
		void queue_deletion(VkCommandPool commandPool) noexcept;
		void queue_deletion(VkQueryPool queryPool) noexcept;
		void queue_deletion(VkPipeline pipeline) noexcept;
		void queue_deletion(VkPipelineLayout pipelineLayout) noexcept;
		void queue_deletion(VkRenderPass renderPass) noexcept;

	private:
		[[nodiscard]] DeletionData& queue_item_deletion() noexcept;
		void clear_epoch(size_t epoch) noexcept;

		static constexpr size_t numEpochs = 2;
				
		LogicalDeviceWrapper* ptr_device;
		Allocator* ptr_allocator;

		size_t m_currentEpoch{ 0 };
		std::array<bool, numEpochs> m_deletionEmpty{};
		std::array<DeletionData, numEpochs> m_deletionQueues{};
	};
}
