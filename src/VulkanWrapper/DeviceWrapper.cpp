#include "VulkanWrapper/DeviceWrapper.hpp"

vulkan::LogicalDeviceWrapper::LogicalDeviceWrapper(VkDevice device, const VkAllocationCallbacks* allocator) :
	m_handle(device),
	m_allocator(allocator)
{
	volkLoadDeviceTable(&m_table, m_handle);
}

vulkan::LogicalDeviceWrapper::~LogicalDeviceWrapper()
{
	if (m_handle != VK_NULL_HANDLE) {
		vkDestroyDevice(m_allocator);
		m_handle = VK_NULL_HANDLE;
	}
}

#if defined(VK_VERSION_1_0)
VkResult vulkan::LogicalDeviceWrapper::vkAllocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const noexcept
{
	return m_table.vkAllocateCommandBuffers(m_handle, pAllocateInfo, pCommandBuffers);
}

VkResult vulkan::LogicalDeviceWrapper::vkAllocateDescriptorSets(const VkDescriptorSetAllocateInfo* pAllocateInfo,
	VkDescriptorSet* pDescriptorSets) const noexcept
{
	return m_table.vkAllocateDescriptorSets(m_handle, pAllocateInfo, pDescriptorSets);
}

VkResult vulkan::LogicalDeviceWrapper::vkAllocateMemory(const VkMemoryAllocateInfo* pAllocateInfo,
	const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const noexcept
{
	return m_table.vkAllocateMemory(m_handle, pAllocateInfo, pAllocator, pMemory);
}

VkResult vulkan::LogicalDeviceWrapper::vkBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory,
	VkDeviceSize memoryOffset) const noexcept
{
	return m_table.vkBindBufferMemory(m_handle, buffer, memory, memoryOffset);
}

VkResult vulkan::LogicalDeviceWrapper::vkBindImageMemory(VkImage image, VkDeviceMemory memory,
	VkDeviceSize memoryOffset) const noexcept
{
	return m_table.vkBindImageMemory(m_handle, image, memory, memoryOffset);
}

void vulkan::LogicalDeviceWrapper::vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
	VkQueryControlFlags flags) const noexcept
{
	m_table.vkCmdBeginQuery(commandBuffer, queryPool, query, flags);
}

void vulkan::LogicalDeviceWrapper::vkCmdBeginRenderPass(VkCommandBuffer commandBuffer,
	const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) const noexcept
{
	m_table.vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

void vulkan::LogicalDeviceWrapper::vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
	VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
	const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) const noexcept
{
	m_table.vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void vulkan::LogicalDeviceWrapper::vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer,
	VkDeviceSize offset, VkIndexType indexType) const noexcept
{
	m_table.vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void vulkan::LogicalDeviceWrapper::vkCmdBindPipeline(VkCommandBuffer commandBuffer,
	VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const noexcept
{
	m_table.vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

void vulkan::LogicalDeviceWrapper::vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
	uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const noexcept
{
	m_table.vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

void vulkan::LogicalDeviceWrapper::vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
	VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
	const VkImageBlit* pRegions, VkFilter filter) const noexcept
{
	m_table.vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void vulkan::LogicalDeviceWrapper::vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
	const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) const noexcept
{
	m_table.vkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void vulkan::LogicalDeviceWrapper::vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
	VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount,
	const VkImageSubresourceRange* pRanges) const noexcept
{
	m_table.vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

void vulkan::LogicalDeviceWrapper::vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
	VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
	const VkImageSubresourceRange* pRanges) const noexcept
{
	m_table.vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
	VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) const noexcept
{
	m_table.vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
	VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
	const VkBufferImageCopy* pRegions) const noexcept
{
	m_table.vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
	VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
	const VkImageCopy* pRegions) const noexcept
{
	m_table.vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
	VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount,
	const VkBufferImageCopy* pRegions) const noexcept
{
	m_table.vkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
	uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
	VkQueryResultFlags flags) const noexcept
{
	m_table.vkCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

void vulkan::LogicalDeviceWrapper::vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX,
	uint32_t groupCountY, uint32_t groupCountZ) const noexcept
{
	m_table.vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void vulkan::LogicalDeviceWrapper::vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
	VkDeviceSize offset) const noexcept
{
	m_table.vkCmdDispatchIndirect(commandBuffer, buffer, offset);
}

void vulkan::LogicalDeviceWrapper::vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount,
	uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const noexcept
{
	m_table.vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void vulkan::LogicalDeviceWrapper::vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
	uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const noexcept
{
	m_table.vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void vulkan::LogicalDeviceWrapper::vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
	VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const noexcept
{
	m_table.vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void vulkan::LogicalDeviceWrapper::vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
	VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const noexcept
{
	m_table.vkCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void vulkan::LogicalDeviceWrapper::vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
	uint32_t query) const noexcept
{
	m_table.vkCmdEndQuery(commandBuffer, queryPool, query);
}

void vulkan::LogicalDeviceWrapper::vkCmdEndRenderPass(VkCommandBuffer commandBuffer) const noexcept
{
	m_table.vkCmdEndRenderPass(commandBuffer);
}

void vulkan::LogicalDeviceWrapper::vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
	const VkCommandBuffer* pCommandBuffers) const noexcept
{
	m_table.vkCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

void vulkan::LogicalDeviceWrapper::vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
	VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) const noexcept
{
	m_table.vkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

void vulkan::LogicalDeviceWrapper::vkCmdNextSubpass(VkCommandBuffer commandBuffer,
	VkSubpassContents contents) const noexcept
{
	m_table.vkCmdNextSubpass(commandBuffer, contents);
}

void vulkan::LogicalDeviceWrapper::vkCmdPipelineBarrier(VkCommandBuffer commandBuffer,
	VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
	uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
	const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
	const VkImageMemoryBarrier* pImageMemoryBarriers) const noexcept
{
	m_table.vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
		pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void vulkan::LogicalDeviceWrapper::vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
	VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) const noexcept
{
	m_table.vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

void vulkan::LogicalDeviceWrapper::vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
	VkPipelineStageFlags stageMask) const noexcept
{
	m_table.vkCmdResetEvent(commandBuffer, event, stageMask);
}

void vulkan::LogicalDeviceWrapper::vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
	uint32_t firstQuery, uint32_t queryCount) const noexcept
{
	m_table.vkCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

void vulkan::LogicalDeviceWrapper::vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
	VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
	const VkImageResolve* pRegions) const noexcept
{
	m_table.vkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetBlendConstants(VkCommandBuffer commandBuffer,
	const float blendConstants[4]) const noexcept
{
	m_table.vkCmdSetBlendConstants(commandBuffer, blendConstants);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
	float depthBiasClamp, float depthBiasSlopeFactor) const noexcept
{
	m_table.vkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
	float maxDepthBounds) const noexcept
{
	m_table.vkCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
	VkPipelineStageFlags stageMask) const noexcept
{
	m_table.vkCmdSetEvent(commandBuffer, event, stageMask);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const noexcept
{
	m_table.vkCmdSetLineWidth(commandBuffer, lineWidth);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
	uint32_t scissorCount, const VkRect2D* pScissors) const noexcept
{
	m_table.vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer,
	VkStencilFaceFlags faceMask, uint32_t compareMask) const noexcept
{
	m_table.vkCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
	uint32_t reference) const noexcept
{
	m_table.vkCmdSetStencilReference(commandBuffer, faceMask, reference);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
	uint32_t writeMask) const noexcept
{
	m_table.vkCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
	uint32_t viewportCount, const VkViewport* pViewports) const noexcept
{
	m_table.vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

void vulkan::LogicalDeviceWrapper::vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
	VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) const noexcept
{
	m_table.vkCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

void vulkan::LogicalDeviceWrapper::vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount,
	const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
	uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
	const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
	const VkImageMemoryBarrier* pImageMemoryBarriers) const noexcept
{
	m_table.vkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
		pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void vulkan::LogicalDeviceWrapper::vkCmdWriteTimestamp(VkCommandBuffer commandBuffer,
	VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) const noexcept
{
	m_table.vkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateBuffer(const VkBufferCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const noexcept
{
	return m_table.vkCreateBuffer(m_handle, pCreateInfo, pAllocator, pBuffer);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateBufferView(const VkBufferViewCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkBufferView* pView) const noexcept
{
	return m_table.vkCreateBufferView(m_handle, pCreateInfo, pAllocator, pView);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateCommandPool(const VkCommandPoolCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const noexcept
{
	return m_table.vkCreateCommandPool(m_handle, pCreateInfo, pAllocator, pCommandPool);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateComputePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount,
	const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
	VkPipeline* pPipelines) const noexcept
{
	return m_table.vkCreateComputePipelines(m_handle, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateDescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) const noexcept
{
	return m_table.vkCreateDescriptorPool(m_handle, pCreateInfo, pAllocator, pDescriptorPool);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const noexcept
{
	return m_table.vkCreateDescriptorSetLayout(m_handle, pCreateInfo, pAllocator, pSetLayout);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateEvent(const VkEventCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) const noexcept
{
	return m_table.vkCreateEvent(m_handle, pCreateInfo, pAllocator, pEvent);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateFence(const VkFenceCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkFence* pFence) const noexcept
{
	return m_table.vkCreateFence(m_handle, pCreateInfo, pAllocator, pFence);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateFramebuffer(const VkFramebufferCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const noexcept
{
	return m_table.vkCreateFramebuffer(m_handle, pCreateInfo, pAllocator, pFramebuffer);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateGraphicsPipelines(VkPipelineCache pipelineCache,
	uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator,
	VkPipeline* pPipelines) const noexcept
{
	return m_table.vkCreateGraphicsPipelines(m_handle, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateImage(const VkImageCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkImage* pImage) const noexcept
{
	return m_table.vkCreateImage(m_handle, pCreateInfo, pAllocator, pImage);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateImageView(const VkImageViewCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkImageView* pView) const noexcept
{
	return m_table.vkCreateImageView(m_handle, pCreateInfo, pAllocator, pView);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreatePipelineCache(const VkPipelineCacheCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const noexcept
{
	return m_table.vkCreatePipelineCache(m_handle, pCreateInfo, pAllocator, pPipelineCache);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreatePipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const noexcept
{
	return m_table.vkCreatePipelineLayout(m_handle, pCreateInfo, pAllocator, pPipelineLayout);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateQueryPool(const VkQueryPoolCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const noexcept
{
	return m_table.vkCreateQueryPool(m_handle, pCreateInfo, pAllocator, pQueryPool);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateRenderPass(const VkRenderPassCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const noexcept
{
	return m_table.vkCreateRenderPass(m_handle, pCreateInfo, pAllocator, pRenderPass);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateSampler(const VkSamplerCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const noexcept
{
	return m_table.vkCreateSampler(m_handle, pCreateInfo, pAllocator, pSampler);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateSemaphore(const VkSemaphoreCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const noexcept
{
	return m_table.vkCreateSemaphore(m_handle, pCreateInfo, pAllocator, pSemaphore);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateShaderModule(const VkShaderModuleCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const noexcept
{
	return m_table.vkCreateShaderModule(m_handle, pCreateInfo, pAllocator, pShaderModule);
}

void vulkan::LogicalDeviceWrapper::vkDestroyBuffer(VkBuffer buffer,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyBuffer(m_handle, buffer, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyBufferView(VkBufferView bufferView,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyBufferView(m_handle, bufferView, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyCommandPool(VkCommandPool commandPool,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyCommandPool(m_handle, commandPool, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyDescriptorPool(VkDescriptorPool descriptorPool,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyDescriptorPool(m_handle, descriptorPool, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyDescriptorSetLayout(m_handle, descriptorSetLayout, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyDevice(const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyDevice(m_handle, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyEvent(VkEvent event, const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyEvent(m_handle, event, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyFence(VkFence fence, const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyFence(m_handle, fence, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyFramebuffer(VkFramebuffer framebuffer,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyFramebuffer(m_handle, framebuffer, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyImage(VkImage image, const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyImage(m_handle, image, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyImageView(VkImageView imageView,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyImageView(m_handle, imageView, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyPipeline(VkPipeline pipeline,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyPipeline(m_handle, pipeline, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyPipelineCache(VkPipelineCache pipelineCache,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyPipelineCache(m_handle, pipelineCache, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyPipelineLayout(VkPipelineLayout pipelineLayout,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyPipelineLayout(m_handle, pipelineLayout, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyQueryPool(VkQueryPool queryPool,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyQueryPool(m_handle, queryPool, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyRenderPass(VkRenderPass renderPass,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyRenderPass(m_handle, renderPass, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroySampler(VkSampler sampler,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroySampler(m_handle, sampler, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroySemaphore(VkSemaphore semaphore,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroySemaphore(m_handle, semaphore, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroyShaderModule(VkShaderModule shaderModule,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyShaderModule(m_handle, shaderModule, pAllocator);
}

VkResult vulkan::LogicalDeviceWrapper::vkDeviceWaitIdle() const noexcept
{
	return m_table.vkDeviceWaitIdle(m_handle);
}

VkResult vulkan::LogicalDeviceWrapper::vkEndCommandBuffer(VkCommandBuffer commandBuffer) const noexcept
{
	return m_table.vkEndCommandBuffer( commandBuffer);
}

VkResult vulkan::LogicalDeviceWrapper::vkFlushMappedMemoryRanges(uint32_t memoryRangeCount,
	const VkMappedMemoryRange* pMemoryRanges) const noexcept
{
	return m_table.vkFlushMappedMemoryRanges(m_handle, memoryRangeCount, pMemoryRanges);
}

void vulkan::LogicalDeviceWrapper::vkFreeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount,
	const VkCommandBuffer* pCommandBuffers) const noexcept
{
	m_table.vkFreeCommandBuffers(m_handle, commandPool, commandBufferCount, pCommandBuffers);
}

VkResult vulkan::LogicalDeviceWrapper::vkFreeDescriptorSets(VkDescriptorPool descriptorPool,
	uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) const noexcept
{
	return m_table.vkFreeDescriptorSets(m_handle, descriptorPool, descriptorSetCount, pDescriptorSets);
}

void vulkan::LogicalDeviceWrapper::vkFreeMemory(VkDeviceMemory memory,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkFreeMemory(m_handle, memory, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkGetBufferMemoryRequirements(VkBuffer buffer,
	VkMemoryRequirements* pMemoryRequirements) const noexcept
{
	m_table.vkGetBufferMemoryRequirements(m_handle, buffer, pMemoryRequirements);
}

void vulkan::LogicalDeviceWrapper::vkGetDeviceMemoryCommitment(VkDeviceMemory memory,
	VkDeviceSize* pCommittedMemoryInBytes) const noexcept
{
	m_table.vkGetDeviceMemoryCommitment(m_handle, memory, pCommittedMemoryInBytes);
}

void vulkan::LogicalDeviceWrapper::vkGetDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex,
	VkQueue* pQueue) const noexcept
{
	m_table.vkGetDeviceQueue(m_handle, queueFamilyIndex, queueIndex, pQueue);
}

VkResult vulkan::LogicalDeviceWrapper::vkGetEventStatus(VkEvent event) const noexcept
{
	return m_table.vkGetEventStatus(m_handle, event);
}

VkResult vulkan::LogicalDeviceWrapper::vkGetFenceStatus(VkFence fence) const noexcept
{
	return m_table.vkGetFenceStatus(m_handle, fence);
}

void vulkan::LogicalDeviceWrapper::vkGetImageMemoryRequirements(VkImage image,
	VkMemoryRequirements* pMemoryRequirements) const noexcept
{
	m_table.vkGetImageMemoryRequirements(m_handle, image, pMemoryRequirements);
}

void vulkan::LogicalDeviceWrapper::vkGetImageSparseMemoryRequirements(VkImage image,
	uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const noexcept
{
	m_table.vkGetImageSparseMemoryRequirements(m_handle, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void vulkan::LogicalDeviceWrapper::vkGetImageSubresourceLayout(VkImage image,
	const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const noexcept
{
	m_table.vkGetImageSubresourceLayout(m_handle, image, pSubresource, pLayout);
}

VkResult vulkan::LogicalDeviceWrapper::vkGetPipelineCacheData(VkPipelineCache pipelineCache, size_t* pDataSize,
	void* pData) const noexcept
{
	return m_table.vkGetPipelineCacheData(m_handle, pipelineCache, pDataSize, pData);
}

VkResult vulkan::LogicalDeviceWrapper::vkGetQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery,
	uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) const noexcept
{
	return m_table.vkGetQueryPoolResults(m_handle, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

void vulkan::LogicalDeviceWrapper::vkGetRenderAreaGranularity(VkRenderPass renderPass,
	VkExtent2D* pGranularity) const noexcept
{
	m_table.vkGetRenderAreaGranularity(m_handle, renderPass, pGranularity);
}

VkResult vulkan::LogicalDeviceWrapper::vkInvalidateMappedMemoryRanges(uint32_t memoryRangeCount,
	const VkMappedMemoryRange* pMemoryRanges) const noexcept
{
	return m_table.vkInvalidateMappedMemoryRanges(m_handle, memoryRangeCount, pMemoryRanges);
}

VkResult vulkan::LogicalDeviceWrapper::vkMapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
	VkMemoryMapFlags flags, void** ppData) const noexcept
{
	return m_table.vkMapMemory(m_handle, memory, offset, size, flags, ppData);
}

VkResult vulkan::LogicalDeviceWrapper::vkMergePipelineCaches(VkPipelineCache dstCache, uint32_t srcCacheCount,
	const VkPipelineCache* pSrcCaches) const noexcept
{
	return m_table.vkMergePipelineCaches(m_handle, dstCache, srcCacheCount, pSrcCaches);
}

VkResult vulkan::LogicalDeviceWrapper::vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount,
	const VkBindSparseInfo* pBindInfo, VkFence fence) const noexcept
{
	return m_table.vkQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

VkResult vulkan::LogicalDeviceWrapper::vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
	VkFence fence) const noexcept
{
	return m_table.vkQueueSubmit(queue, submitCount, pSubmits, fence);
}

VkResult vulkan::LogicalDeviceWrapper::vkQueueWaitIdle(VkQueue queue) const noexcept
{
	return m_table.vkQueueWaitIdle(queue);
}

VkResult vulkan::LogicalDeviceWrapper::vkResetCommandBuffer(VkCommandBuffer commandBuffer,
	VkCommandBufferResetFlags flags) const noexcept
{
	return m_table.vkResetCommandBuffer(commandBuffer, flags);
}

VkResult vulkan::LogicalDeviceWrapper::vkResetCommandPool(VkCommandPool commandPool,
	VkCommandPoolResetFlags flags) const noexcept
{
	return m_table.vkResetCommandPool(m_handle, commandPool, flags);
}

VkResult vulkan::LogicalDeviceWrapper::vkResetDescriptorPool(VkDescriptorPool descriptorPool,
	VkDescriptorPoolResetFlags flags) const noexcept
{
	return m_table.vkResetDescriptorPool(m_handle, descriptorPool, flags);
}

VkResult vulkan::LogicalDeviceWrapper::vkResetEvent(VkEvent event) const noexcept
{
	return m_table.vkResetEvent(m_handle, event);
}

VkResult vulkan::LogicalDeviceWrapper::vkResetFences(uint32_t fenceCount, const VkFence* pFences) const noexcept
{
	return m_table.vkResetFences(m_handle, fenceCount, pFences);
}

VkResult vulkan::LogicalDeviceWrapper::vkSetEvent(VkEvent event) const noexcept
{
	return m_table.vkSetEvent(m_handle, event);
}

void vulkan::LogicalDeviceWrapper::vkUnmapMemory(VkDeviceMemory memory) const noexcept
{
	m_table.vkUnmapMemory(m_handle, memory);
}

void vulkan::LogicalDeviceWrapper::vkUpdateDescriptorSets(uint32_t descriptorWriteCount,
	const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
	const VkCopyDescriptorSet* pDescriptorCopies) const noexcept
{
	m_table.vkUpdateDescriptorSets(m_handle, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

VkResult vulkan::LogicalDeviceWrapper::vkWaitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
	uint64_t timeout) const noexcept
{
	return m_table.vkWaitForFences(m_handle, fenceCount, pFences, waitAll, timeout);
}

#endif

#if defined(VK_VERSION_1_1)

VkResult vulkan::LogicalDeviceWrapper::vkBindBufferMemory2(uint32_t bindInfoCount,
	const VkBindBufferMemoryInfo* pBindInfos) const noexcept
{
	return m_table.vkBindBufferMemory2(m_handle, bindInfoCount, pBindInfos);
}

VkResult vulkan::LogicalDeviceWrapper::vkBindImageMemory2(uint32_t bindInfoCount,
	const VkBindImageMemoryInfo* pBindInfos) const noexcept
{
	return m_table.vkBindImageMemory2(m_handle, bindInfoCount, pBindInfos);
}

void vulkan::LogicalDeviceWrapper::vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
	uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
	uint32_t groupCountZ) const noexcept
{
	m_table.vkCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) const noexcept
{
	m_table.vkCmdSetDeviceMask(commandBuffer, deviceMask);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateDescriptorUpdateTemplate(
	const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
	VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const noexcept
{
	return m_table.vkCreateDescriptorUpdateTemplate(m_handle, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateSamplerYcbcrConversion(
	const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
	VkSamplerYcbcrConversion* pYcbcrConversion) const noexcept
{
	return m_table.vkCreateSamplerYcbcrConversion(m_handle, pCreateInfo, pAllocator, pYcbcrConversion);
}

void vulkan::LogicalDeviceWrapper::vkDestroyDescriptorUpdateTemplate(
	VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyDescriptorUpdateTemplate(m_handle, descriptorUpdateTemplate, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkDestroySamplerYcbcrConversion(VkSamplerYcbcrConversion ycbcrConversion,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroySamplerYcbcrConversion(m_handle, ycbcrConversion, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkGetBufferMemoryRequirements2(const VkBufferMemoryRequirementsInfo2* pInfo,
	VkMemoryRequirements2* pMemoryRequirements) const noexcept
{
	m_table.vkGetBufferMemoryRequirements2(m_handle, pInfo, pMemoryRequirements);
}

void vulkan::LogicalDeviceWrapper::vkGetDescriptorSetLayoutSupport(const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
	VkDescriptorSetLayoutSupport* pSupport) const noexcept
{
	m_table.vkGetDescriptorSetLayoutSupport(m_handle, pCreateInfo, pSupport);
}

void vulkan::LogicalDeviceWrapper::vkGetDeviceGroupPeerMemoryFeatures(uint32_t heapIndex, uint32_t localDeviceIndex,
	uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const noexcept
{
	m_table.vkGetDeviceGroupPeerMemoryFeatures(m_handle, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

void vulkan::LogicalDeviceWrapper::vkGetDeviceQueue2(const VkDeviceQueueInfo2* pQueueInfo,
	VkQueue* pQueue) const noexcept
{
	m_table.vkGetDeviceQueue2(m_handle, pQueueInfo, pQueue);
}

void vulkan::LogicalDeviceWrapper::vkGetImageMemoryRequirements2(const VkImageMemoryRequirementsInfo2* pInfo,
	VkMemoryRequirements2* pMemoryRequirements) const noexcept
{
	m_table.vkGetImageMemoryRequirements2(m_handle, pInfo, pMemoryRequirements);
}

void vulkan::LogicalDeviceWrapper::vkGetImageSparseMemoryRequirements2(
	const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const noexcept
{
	m_table.vkGetImageSparseMemoryRequirements2(m_handle, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void vulkan::LogicalDeviceWrapper::vkTrimCommandPool(VkCommandPool commandPool,
	VkCommandPoolTrimFlags flags) const noexcept
{
	m_table.vkTrimCommandPool(m_handle, commandPool, flags);
}

void vulkan::LogicalDeviceWrapper::vkUpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet,
	VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) const noexcept
{
	m_table.vkUpdateDescriptorSetWithTemplate(m_handle, descriptorSet, descriptorUpdateTemplate, pData);
}
#endif
#if defined(VK_VERSION_1_2)
void vulkan::LogicalDeviceWrapper::vkCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
	const VkRenderPassBeginInfo* pRenderPassBegin, const VkSubpassBeginInfo* pSubpassBeginInfo) const noexcept
{
	m_table.vkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
	VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
	uint32_t stride) const noexcept
{
	m_table.vkCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void vulkan::LogicalDeviceWrapper::vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
	VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
	uint32_t stride) const noexcept
{
	m_table.vkCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void vulkan::LogicalDeviceWrapper::vkCmdEndRenderPass2(VkCommandBuffer commandBuffer,
	const VkSubpassEndInfo* pSubpassEndInfo) const noexcept
{
	m_table.vkCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdNextSubpass2(VkCommandBuffer commandBuffer,
	const VkSubpassBeginInfo* pSubpassBeginInfo, const VkSubpassEndInfo* pSubpassEndInfo) const noexcept
{
	m_table.vkCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateRenderPass2(const VkRenderPassCreateInfo2* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const noexcept
{
	return m_table.vkCreateRenderPass2(m_handle, pCreateInfo, pAllocator, pRenderPass);
}

VkDeviceAddress vulkan::LogicalDeviceWrapper::vkGetBufferDeviceAddress(
	const VkBufferDeviceAddressInfo* pInfo) const noexcept
{
	return m_table.vkGetBufferDeviceAddress(m_handle, pInfo);
}

uint64_t vulkan::LogicalDeviceWrapper::vkGetBufferOpaqueCaptureAddress(
	const VkBufferDeviceAddressInfo* pInfo) const noexcept
{
	return m_table.vkGetBufferOpaqueCaptureAddress(m_handle, pInfo);
}

uint64_t vulkan::LogicalDeviceWrapper::vkGetDeviceMemoryOpaqueCaptureAddress(
	const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const noexcept
{
	return m_table.vkGetDeviceMemoryOpaqueCaptureAddress(m_handle, pInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkGetSemaphoreCounterValue(VkSemaphore semaphore,
	uint64_t* pValue) const noexcept
{
	return m_table.vkGetSemaphoreCounterValue(m_handle, semaphore, pValue);
}

void vulkan::LogicalDeviceWrapper::vkResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery,
	uint32_t queryCount) const noexcept
{
	m_table.vkResetQueryPool(m_handle, queryPool, firstQuery, queryCount);
}

VkResult vulkan::LogicalDeviceWrapper::vkSignalSemaphore(const VkSemaphoreSignalInfo* pSignalInfo) const noexcept
{
	return m_table.vkSignalSemaphore(m_handle, pSignalInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkWaitSemaphores(const VkSemaphoreWaitInfo* pWaitInfo,
	uint64_t timeout) const noexcept
{
	return m_table.vkWaitSemaphores(m_handle, pWaitInfo, timeout);
}

#endif /* defined(VK_VERSION_1_2) */
#if defined(VK_VERSION_1_3)
void vulkan::LogicalDeviceWrapper::vkCmdBeginRendering(VkCommandBuffer commandBuffer,
	const VkRenderingInfo* pRenderingInfo) const noexcept
{
	m_table.vkCmdBeginRendering(commandBuffer, pRenderingInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
	uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
	const VkDeviceSize* pStrides) const noexcept
{
	m_table.vkCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

void vulkan::LogicalDeviceWrapper::vkCmdBlitImage2(VkCommandBuffer commandBuffer,
	const VkBlitImageInfo2* pBlitImageInfo) const noexcept
{
	m_table.vkCmdBlitImage2(commandBuffer, pBlitImageInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyBuffer2(VkCommandBuffer commandBuffer,
	const VkCopyBufferInfo2* pCopyBufferInfo) const noexcept
{
	m_table.vkCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
	const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const noexcept
{
	m_table.vkCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyImage2(VkCommandBuffer commandBuffer,
	const VkCopyImageInfo2* pCopyImageInfo) const noexcept
{
	m_table.vkCmdCopyImage2(commandBuffer, pCopyImageInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
	const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const noexcept
{
	m_table.vkCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdEndRendering(VkCommandBuffer commandBuffer) const noexcept
{
	m_table.vkCmdEndRendering(commandBuffer);
}

void vulkan::LogicalDeviceWrapper::vkCmdPipelineBarrier2(VkCommandBuffer commandBuffer,
	const VkDependencyInfo* pDependencyInfo) const noexcept
{
	m_table.vkCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
	VkPipelineStageFlags2 stageMask) const noexcept
{
	m_table.vkCmdResetEvent2(commandBuffer, event, stageMask);
}

void vulkan::LogicalDeviceWrapper::vkCmdResolveImage2(VkCommandBuffer commandBuffer,
	const VkResolveImageInfo2* pResolveImageInfo) const noexcept
{
	m_table.vkCmdResolveImage2(commandBuffer, pResolveImageInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetCullMode(VkCommandBuffer commandBuffer,
	VkCullModeFlags cullMode) const noexcept
{
	m_table.vkCmdSetCullMode(commandBuffer, cullMode);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer,
	VkBool32 depthBiasEnable) const noexcept
{
	m_table.vkCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer,
	VkBool32 depthBoundsTestEnable) const noexcept
{
	m_table.vkCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetDepthCompareOp(VkCommandBuffer commandBuffer,
	VkCompareOp depthCompareOp) const noexcept
{
	m_table.vkCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetDepthTestEnable(VkCommandBuffer commandBuffer,
	VkBool32 depthTestEnable) const noexcept
{
	m_table.vkCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer,
	VkBool32 depthWriteEnable) const noexcept
{
	m_table.vkCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
	const VkDependencyInfo* pDependencyInfo) const noexcept
{
	m_table.vkCmdSetEvent2(commandBuffer, event, pDependencyInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetFrontFace(VkCommandBuffer commandBuffer,
	VkFrontFace frontFace) const noexcept
{
	m_table.vkCmdSetFrontFace(commandBuffer, frontFace);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer,
	VkBool32 primitiveRestartEnable) const noexcept
{
	m_table.vkCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
	VkPrimitiveTopology primitiveTopology) const noexcept
{
	m_table.vkCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
	VkBool32 rasterizerDiscardEnable) const noexcept
{
	m_table.vkCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
	const VkRect2D* pScissors) const noexcept
{
	m_table.vkCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
	VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const noexcept
{
	m_table.vkCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetStencilTestEnable(VkCommandBuffer commandBuffer,
	VkBool32 stencilTestEnable) const noexcept
{
	m_table.vkCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
}

void vulkan::LogicalDeviceWrapper::vkCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
	const VkViewport* pViewports) const noexcept
{
	m_table.vkCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
}

void vulkan::LogicalDeviceWrapper::vkCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount,
	const VkEvent* pEvents, const VkDependencyInfo* pDependencyInfos) const noexcept
{
	m_table.vkCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

void vulkan::LogicalDeviceWrapper::vkCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage,
	VkQueryPool queryPool, uint32_t query) const noexcept
{
	m_table.vkCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreatePrivateDataSlot(const VkPrivateDataSlotCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) const noexcept
{
	return m_table.vkCreatePrivateDataSlot(m_handle, pCreateInfo, pAllocator, pPrivateDataSlot);
}

void vulkan::LogicalDeviceWrapper::vkDestroyPrivateDataSlot(VkPrivateDataSlot privateDataSlot,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyPrivateDataSlot(m_handle, privateDataSlot, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkGetDeviceBufferMemoryRequirements(const VkDeviceBufferMemoryRequirements* pInfo,
	VkMemoryRequirements2* pMemoryRequirements) const noexcept
{
	m_table.vkGetDeviceBufferMemoryRequirements(m_handle, pInfo, pMemoryRequirements);
}

void vulkan::LogicalDeviceWrapper::vkGetDeviceImageMemoryRequirements(const VkDeviceImageMemoryRequirements* pInfo,
	VkMemoryRequirements2* pMemoryRequirements) const noexcept
{
	m_table.vkGetDeviceImageMemoryRequirements(m_handle, pInfo, pMemoryRequirements);
}

void vulkan::LogicalDeviceWrapper::vkGetDeviceImageSparseMemoryRequirements(
	const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount,
	VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const noexcept
{
	m_table.vkGetDeviceImageSparseMemoryRequirements(m_handle, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void vulkan::LogicalDeviceWrapper::vkGetPrivateData(VkObjectType objectType, uint64_t objectHandle,
	VkPrivateDataSlot privateDataSlot, uint64_t* pData) const noexcept
{
	m_table.vkGetPrivateData(m_handle, objectType, objectHandle, privateDataSlot, pData);
}

VkResult vulkan::LogicalDeviceWrapper::vkQueueSubmit2(VkQueue queue, uint32_t submitCount,
	const VkSubmitInfo2* pSubmits, VkFence fence) const noexcept
{
	return m_table.vkQueueSubmit2(queue, submitCount, pSubmits, fence);
}

VkResult vulkan::LogicalDeviceWrapper::vkSetPrivateData(VkObjectType objectType, uint64_t objectHandle,
	VkPrivateDataSlot privateDataSlot, uint64_t data) const noexcept
{
	return m_table.vkSetPrivateData(m_handle, objectType, objectHandle, privateDataSlot, data);
}

#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_EXT_full_screen_exclusive)
VkResult vulkan::LogicalDeviceWrapper::vkAcquireFullScreenExclusiveModeEXT(VkSwapchainKHR swapchain) const noexcept
{
	return m_table.vkAcquireFullScreenExclusiveModeEXT(m_handle, swapchain);
}

VkResult vulkan::LogicalDeviceWrapper::vkReleaseFullScreenExclusiveModeEXT(VkSwapchainKHR swapchain) const noexcept
{
	return m_table.vkReleaseFullScreenExclusiveModeEXT(m_handle, swapchain);
}

#endif /* defined(VK_EXT_full_screen_exclusive) */
#if defined(VK_EXT_mesh_shader)
void vulkan::LogicalDeviceWrapper::vkCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX,
	uint32_t groupCountY, uint32_t groupCountZ) const noexcept
{
	m_table.vkCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void vulkan::LogicalDeviceWrapper::vkCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
	VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
	uint32_t stride) const noexcept
{
	m_table.vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void vulkan::LogicalDeviceWrapper::vkCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
	VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const noexcept
{
	m_table.vkCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
}

#endif /* defined(VK_EXT_mesh_shader) */

#if defined(VK_EXT_opacity_micromap)
VkResult vulkan::LogicalDeviceWrapper::vkBuildMicromapsEXT(VkDeferredOperationKHR deferredOperation,
	uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos) const noexcept
{
	return m_table.vkBuildMicromapsEXT(m_handle, deferredOperation, infoCount, pInfos);
}

void vulkan::LogicalDeviceWrapper::vkCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
	const VkMicromapBuildInfoEXT* pInfos) const noexcept
{
	m_table.vkCmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer,
	const VkCopyMemoryToMicromapInfoEXT* pInfo) const noexcept
{
	m_table.vkCmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyMicromapEXT(VkCommandBuffer commandBuffer,
	const VkCopyMicromapInfoEXT* pInfo) const noexcept
{
	m_table.vkCmdCopyMicromapEXT(commandBuffer, pInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer,
	const VkCopyMicromapToMemoryInfoEXT* pInfo) const noexcept
{
	m_table.vkCmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer,
	uint32_t micromapCount, const VkMicromapEXT* pMicromaps, VkQueryType queryType, VkQueryPool queryPool,
	uint32_t firstQuery) const noexcept
{
	m_table.vkCmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
}

VkResult vulkan::LogicalDeviceWrapper::vkCopyMemoryToMicromapEXT(VkDeferredOperationKHR deferredOperation,
	const VkCopyMemoryToMicromapInfoEXT* pInfo) const noexcept
{
	return m_table.vkCopyMemoryToMicromapEXT(m_handle, deferredOperation, pInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkCopyMicromapEXT(VkDeferredOperationKHR deferredOperation,
	const VkCopyMicromapInfoEXT* pInfo) const noexcept
{
	return m_table.vkCopyMicromapEXT(m_handle, deferredOperation, pInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkCopyMicromapToMemoryEXT(VkDeferredOperationKHR deferredOperation,
	const VkCopyMicromapToMemoryInfoEXT* pInfo) const noexcept
{
	return m_table.vkCopyMicromapToMemoryEXT(m_handle, deferredOperation, pInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateMicromapEXT(const VkMicromapCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap) const noexcept
{
	return m_table.vkCreateMicromapEXT(m_handle, pCreateInfo, pAllocator, pMicromap);
}

void vulkan::LogicalDeviceWrapper::vkDestroyMicromapEXT(VkMicromapEXT micromap,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyMicromapEXT(m_handle, micromap, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkGetDeviceMicromapCompatibilityEXT(const VkMicromapVersionInfoEXT* pVersionInfo,
	VkAccelerationStructureCompatibilityKHR* pCompatibility) const noexcept
{
	m_table.vkGetDeviceMicromapCompatibilityEXT(m_handle, pVersionInfo, pCompatibility);
}

void vulkan::LogicalDeviceWrapper::vkGetMicromapBuildSizesEXT(VkAccelerationStructureBuildTypeKHR buildType,
	const VkMicromapBuildInfoEXT* pBuildInfo, VkMicromapBuildSizesInfoEXT* pSizeInfo) const noexcept
{
	m_table.vkGetMicromapBuildSizesEXT(m_handle, buildType, pBuildInfo, pSizeInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkWriteMicromapsPropertiesEXT(uint32_t micromapCount,
	const VkMicromapEXT* pMicromaps, VkQueryType queryType, size_t dataSize, void* pData, size_t stride) const noexcept
{
	return m_table.vkWriteMicromapsPropertiesEXT(m_handle, micromapCount, pMicromaps, queryType, dataSize, pData, stride);
}

#endif /* defined(VK_EXT_opacity_micromap) */
#if defined(VK_EXT_swapchain_maintenance1)
VkResult vulkan::LogicalDeviceWrapper::vkReleaseSwapchainImagesEXT(
	const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) const noexcept
{
	return m_table.vkReleaseSwapchainImagesEXT(m_handle, pReleaseInfo);
}

#endif /* defined(VK_EXT_swapchain_maintenance1) */
#if defined(VK_KHR_acceleration_structure)
VkResult vulkan::LogicalDeviceWrapper::vkBuildAccelerationStructuresKHR(VkDeferredOperationKHR deferredOperation,
	uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
	const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const noexcept
{
	return m_table.vkBuildAccelerationStructuresKHR(m_handle, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
}

void vulkan::LogicalDeviceWrapper::vkCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer,
	uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
	const VkDeviceAddress* pIndirectDeviceAddresses, const uint32_t* pIndirectStrides,
	const uint32_t* const* ppMaxPrimitiveCounts) const noexcept
{
	m_table.vkCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}

void vulkan::LogicalDeviceWrapper::vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer,
	uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
	const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const noexcept
{
	m_table.vkCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
	const VkCopyAccelerationStructureInfoKHR* pInfo) const noexcept
{
	m_table.vkCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
	const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const noexcept
{
	m_table.vkCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
	const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const noexcept
{
	m_table.vkCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
}

void vulkan::LogicalDeviceWrapper::vkCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
	uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures,
	VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const noexcept
{
	m_table.vkCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

VkResult vulkan::LogicalDeviceWrapper::vkCopyAccelerationStructureKHR(VkDeferredOperationKHR deferredOperation,
	const VkCopyAccelerationStructureInfoKHR* pInfo) const noexcept
{
	return m_table.vkCopyAccelerationStructureKHR(m_handle, deferredOperation, pInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkCopyAccelerationStructureToMemoryKHR(VkDeferredOperationKHR deferredOperation,
	const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const noexcept
{
	return m_table.vkCopyAccelerationStructureToMemoryKHR(m_handle, deferredOperation, pInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkCopyMemoryToAccelerationStructureKHR(VkDeferredOperationKHR deferredOperation,
	const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const noexcept
{
	return m_table.vkCopyMemoryToAccelerationStructureKHR(m_handle, deferredOperation, pInfo);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateAccelerationStructureKHR(
	const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
	VkAccelerationStructureKHR* pAccelerationStructure) const noexcept
{
	return m_table.vkCreateAccelerationStructureKHR(m_handle, pCreateInfo, pAllocator, pAccelerationStructure);
}

void vulkan::LogicalDeviceWrapper::vkDestroyAccelerationStructureKHR(VkAccelerationStructureKHR accelerationStructure,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyAccelerationStructureKHR(m_handle, accelerationStructure, pAllocator);
}

void vulkan::LogicalDeviceWrapper::vkGetAccelerationStructureBuildSizesKHR(
	VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
	const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) const noexcept
{
	m_table.vkGetAccelerationStructureBuildSizesKHR(m_handle, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

VkDeviceAddress vulkan::LogicalDeviceWrapper::vkGetAccelerationStructureDeviceAddressKHR(
	const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const noexcept
{
	return m_table.vkGetAccelerationStructureDeviceAddressKHR(m_handle, pInfo);
}

void vulkan::LogicalDeviceWrapper::vkGetDeviceAccelerationStructureCompatibilityKHR(
	const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
	VkAccelerationStructureCompatibilityKHR* pCompatibility) const noexcept
{
	m_table.vkGetDeviceAccelerationStructureCompatibilityKHR(m_handle, pVersionInfo, pCompatibility);
}

VkResult vulkan::LogicalDeviceWrapper::vkWriteAccelerationStructuresPropertiesKHR(uint32_t accelerationStructureCount,
	const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, size_t dataSize, void* pData,
	size_t stride) const noexcept
{
	return m_table.vkWriteAccelerationStructuresPropertiesKHR(m_handle, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
}

#endif /* defined(VK_KHR_acceleration_structure) */
#if defined(VK_KHR_deferred_host_operations)
VkResult vulkan::LogicalDeviceWrapper::vkCreateDeferredOperationKHR(const VkAllocationCallbacks* pAllocator,
	VkDeferredOperationKHR* pDeferredOperation) const noexcept
{
	return m_table.vkCreateDeferredOperationKHR(m_handle, pAllocator, pDeferredOperation);
}

VkResult vulkan::LogicalDeviceWrapper::vkDeferredOperationJoinKHR(VkDeferredOperationKHR operation) const noexcept
{
	return m_table.vkDeferredOperationJoinKHR(m_handle, operation);
}

void vulkan::LogicalDeviceWrapper::vkDestroyDeferredOperationKHR(VkDeferredOperationKHR operation,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroyDeferredOperationKHR(m_handle, operation, pAllocator);
}

uint32_t vulkan::LogicalDeviceWrapper::vkGetDeferredOperationMaxConcurrencyKHR(
	VkDeferredOperationKHR operation) const noexcept
{
	return m_table.vkGetDeferredOperationMaxConcurrencyKHR(m_handle, operation);
}

VkResult vulkan::LogicalDeviceWrapper::vkGetDeferredOperationResultKHR(VkDeferredOperationKHR operation) const noexcept
{
	return m_table.vkGetDeferredOperationResultKHR(m_handle, operation);
}

#endif /* defined(VK_KHR_deferred_host_operations) */
#if defined(VK_KHR_ray_tracing_pipeline)
void vulkan::LogicalDeviceWrapper::vkCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer,
	uint32_t pipelineStackSize) const noexcept
{
	m_table.vkCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}

void vulkan::LogicalDeviceWrapper::vkCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
	const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
	const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
	const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
	const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
	VkDeviceAddress indirectDeviceAddress) const noexcept
{
	m_table.vkCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}

void vulkan::LogicalDeviceWrapper::vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
	const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
	const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
	const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
	const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height,
	uint32_t depth) const noexcept
{
	m_table.vkCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable,
		width, height, depth);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateRayTracingPipelinesKHR(VkDeferredOperationKHR deferredOperation,
	VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
	const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const noexcept
{
	return m_table.vkCreateRayTracingPipelinesKHR(m_handle, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
		pAllocator, pPipelines);
}

VkResult vulkan::LogicalDeviceWrapper::vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkPipeline pipeline,
	uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const noexcept
{
	return m_table.vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_handle, pipeline, firstGroup, groupCount, dataSize, pData);
}

VkResult vulkan::LogicalDeviceWrapper::vkGetRayTracingShaderGroupHandlesKHR(VkPipeline pipeline, uint32_t firstGroup,
	uint32_t groupCount, size_t dataSize, void* pData) const noexcept
{
	return m_table.vkGetRayTracingShaderGroupHandlesKHR(m_handle, pipeline, firstGroup, groupCount, dataSize, pData);
}

VkDeviceSize vulkan::LogicalDeviceWrapper::vkGetRayTracingShaderGroupStackSizeKHR(VkPipeline pipeline, uint32_t group,
	VkShaderGroupShaderKHR groupShader) const noexcept
{
	return m_table.vkGetRayTracingShaderGroupStackSizeKHR(m_handle, pipeline, group, groupShader);
}

#endif /* defined(VK_KHR_ray_tracing_pipeline) */
#if defined(VK_KHR_swapchain)
VkResult vulkan::LogicalDeviceWrapper::vkAcquireNextImageKHR(VkSwapchainKHR swapchain, uint64_t timeout,
	VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) const noexcept
{
	return m_table.vkAcquireNextImageKHR(m_handle, swapchain, timeout, semaphore, fence, pImageIndex);
}

VkResult vulkan::LogicalDeviceWrapper::vkCreateSwapchainKHR(const VkSwapchainCreateInfoKHR* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const noexcept
{
	return m_table.vkCreateSwapchainKHR(m_handle, pCreateInfo, pAllocator, pSwapchain);
}

void vulkan::LogicalDeviceWrapper::vkDestroySwapchainKHR(VkSwapchainKHR swapchain,
	const VkAllocationCallbacks* pAllocator) const noexcept
{
	m_table.vkDestroySwapchainKHR(m_handle, swapchain, pAllocator);
}

VkResult vulkan::LogicalDeviceWrapper::vkGetSwapchainImagesKHR(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
	VkImage* pSwapchainImages) const noexcept
{
	return m_table.vkGetSwapchainImagesKHR(m_handle, swapchain, pSwapchainImageCount, pSwapchainImages);
}

VkResult vulkan::LogicalDeviceWrapper::vkQueuePresentKHR(VkQueue queue,
	const VkPresentInfoKHR* pPresentInfo) const noexcept
{
	return m_table.vkQueuePresentKHR(queue, pPresentInfo);
}

#endif /* defined(VK_KHR_swapchain) */

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

VkDevice vulkan::LogicalDeviceWrapper::get_handle() const noexcept
{
	return m_handle;
}
