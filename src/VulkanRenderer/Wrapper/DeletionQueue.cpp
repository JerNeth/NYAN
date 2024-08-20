module;

#include <cassert>
#include <utility>

module NYANVulkanWrapper;

using namespace nyan::vulkan::wrapper;

DeletionQueue::DeletionQueue(LogicalDeviceWrapper& device, Allocator& allocator) noexcept :
	r_device(device),
	r_allocator(allocator)
{
}

DeletionQueue::DeletionQueue(DeletionQueue&& other) noexcept :
	r_device(other.r_device),
	r_allocator(other.r_allocator),
	m_currentEpoch(other.m_currentEpoch),
	m_deletionEmpty(std::move(other.m_deletionEmpty)),
	m_deletionQueues(std::move(other.m_deletionQueues))
{
	for (size_t epoch = 0; epoch < numEpochs; ++epoch)
		m_deletionEmpty[epoch] = true;
}

DeletionQueue& DeletionQueue::operator=(DeletionQueue&& other) noexcept
{
	if (this != std::addressof(other))
	{
		std::swap(m_currentEpoch, other.m_currentEpoch);
		std::swap(m_deletionQueues, other.m_deletionQueues);
		std::swap(m_deletionEmpty, other.m_deletionEmpty);
	}
	return *this;
}

DeletionQueue::~DeletionQueue()
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
	queue_item_deletion().imageViews.push_back(imageView);
}

void DeletionQueue::queue_buffer_view_deletion(const VkBufferView bufferView) noexcept
{
	queue_item_deletion().bufferViews.push_back(bufferView);
}

void DeletionQueue::queue_sampler_deletion(const VkSampler sampler) noexcept
{
	queue_item_deletion().samplers.push_back(sampler);
}
void DeletionQueue::queue_image_deletion(const VkImage image) noexcept
{
	queue_item_deletion().images.push_back(image);
}
void DeletionQueue::queue_buffer_deletion(const VkBuffer buffer) noexcept
{
	queue_item_deletion().buffers.push_back(buffer);
}
void DeletionQueue::queue_acceleration_structure_deletion(const VkAccelerationStructureKHR accelerationStructure) noexcept
{
	queue_item_deletion().accelerationStructures.push_back(accelerationStructure);
}
void DeletionQueue::queue_framebuffer_deletion(const VkFramebuffer framebuffer) noexcept
{
	queue_item_deletion().framebuffers.push_back(framebuffer);
}
void DeletionQueue::queue_semaphore_deletion(const VkSemaphore semaphore) noexcept
{
	queue_item_deletion().semaphores.push_back(semaphore);
}
void DeletionQueue::queue_allocation_deletion(const VmaAllocation allocation) noexcept
{
	queue_item_deletion().allocations.push_back(allocation);
}
void DeletionQueue::queue_descriptor_pool_deletion(const VkDescriptorPool descriptorPool) noexcept
{
	queue_item_deletion().descriptorPools.push_back(descriptorPool);
}

void DeletionQueue::queue_descriptor_set_layout_deletion(const VkDescriptorSetLayout descriptorSetLayout) noexcept
{
	queue_item_deletion().descriptorSetLayouts.push_back(descriptorSetLayout);
}

void DeletionQueue::queue_command_pool_deletion(const VkCommandPool commandPool) noexcept
{
	queue_item_deletion().commandPools.push_back(commandPool);
}
void DeletionQueue::queue_query_pool_deletion(const VkQueryPool queryPool) noexcept
{
	queue_item_deletion().queryPools.push_back(queryPool);
}
void DeletionQueue::queue_pipeline_deletion(VkPipeline pipeline) noexcept
{
	queue_item_deletion().pipelines.push_back(pipeline);
}
void DeletionQueue::queue_pipeline_layout_deletion(VkPipelineLayout pipelineLayout) noexcept
{
	queue_item_deletion().pipelineLayouts.push_back(pipelineLayout);
}
DeletionQueue::DeletionData& nyan::vulkan::wrapper::DeletionQueue::queue_item_deletion() noexcept
{
	assert(m_currentEpoch < numEpochs);
	// ReSharper disable once CppUseStructuredBinding
	m_deletionEmpty[m_currentEpoch] = false;
	return m_deletionQueues[m_currentEpoch];
}
void DeletionQueue::clear_epoch(const size_t epoch) noexcept
{
	assert(epoch < numEpochs);
	if (epoch >= numEpochs)
		return;
	if (!m_deletionEmpty[epoch])
	{
		m_deletionEmpty[epoch] = true;

		// ReSharper disable once CppUseStructuredBinding
		auto& currentQueue = m_deletionQueues[epoch];

		for (const auto imageView : currentQueue.imageViews)
			r_device.vkDestroyImageView(imageView);
		currentQueue.imageViews.clear();

		for (const auto bufferView : currentQueue.bufferViews)
			r_device.vkDestroyBufferView(bufferView);
		currentQueue.bufferViews.clear();

		for (const auto sampler : currentQueue.samplers)
			r_device.vkDestroySampler(sampler);
		currentQueue.samplers.clear();

		for (const auto image : currentQueue.images)
			r_device.vkDestroyImage(image);
		currentQueue.images.clear();

		for (const auto buffer : currentQueue.buffers)
			r_device.vkDestroyBuffer(buffer);
		currentQueue.buffers.clear();

		for (const auto accelerationStructure : currentQueue.accelerationStructures)
			r_device.vkDestroyAccelerationStructureKHR(accelerationStructure);
		currentQueue.accelerationStructures.clear();

		for (const auto framebuffer : currentQueue.framebuffers)
			r_device.vkDestroyFramebuffer(framebuffer);
		currentQueue.framebuffers.clear();

		for (const auto semaphore : currentQueue.semaphores)
			r_device.vkDestroySemaphore(semaphore);
		currentQueue.semaphores.clear();

		for (const auto allocation : currentQueue.allocations)
			r_allocator.free_allocation(allocation);
		currentQueue.allocations.clear();

		for (const auto pool : currentQueue.descriptorPools)
			r_device.vkDestroyDescriptorPool(pool);
		currentQueue.descriptorPools.clear();

		for (const auto setLayout : currentQueue.descriptorSetLayouts)
			r_device.vkDestroyDescriptorSetLayout(setLayout);
		currentQueue.descriptorSetLayouts.clear();

		for (const auto pool : currentQueue.commandPools)
			r_device.vkDestroyCommandPool(pool);
		currentQueue.commandPools.clear();

		for (const auto pool : currentQueue.queryPools)
			r_device.vkDestroyQueryPool(pool);
		currentQueue.queryPools.clear();

		for (const auto pipeline : currentQueue.pipelines)
			r_device.vkDestroyPipeline(pipeline);
		currentQueue.pipelines.clear();

		for (const auto pipelineLayout : currentQueue.pipelineLayouts)
			r_device.vkDestroyPipelineLayout(pipelineLayout);
		currentQueue.pipelineLayouts.clear();
	}
}