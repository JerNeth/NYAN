module;

#include <cassert>

#include "volk.h"
#include "vk_mem_alloc.h"

export module NYANVulkanWrapper:LogicalDeviceWrapper;

export namespace nyan::vulkan::wrapper
{
	class LogicalDeviceWrapper
	{
	public:
		explicit LogicalDeviceWrapper(VkDevice device, const VkAllocationCallbacks* allocator = nullptr) noexcept;
		~LogicalDeviceWrapper() noexcept;

		LogicalDeviceWrapper(LogicalDeviceWrapper&) = delete;
		LogicalDeviceWrapper(LogicalDeviceWrapper&& other) noexcept;
		LogicalDeviceWrapper& operator=(LogicalDeviceWrapper&) = delete;
		LogicalDeviceWrapper& operator=(LogicalDeviceWrapper&& other) noexcept;

		[[nodiscard]] VkDevice get_handle() const noexcept
		{
			return m_handle;
		}

		[[nodiscard]] const VkAllocationCallbacks* get_allocator_callbacks() const noexcept
		{
			return m_allocatorCallbacks;
		}

#if defined(VK_VERSION_1_0)

		VkResult vkAllocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const noexcept
		{
			return m_table.vkAllocateCommandBuffers(m_handle, pAllocateInfo, pCommandBuffers);
		}

		VkResult vkAllocateDescriptorSets(const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) const noexcept
		{
			return m_table.vkAllocateDescriptorSets(m_handle, pAllocateInfo, pDescriptorSets);
		}

		VkResult vkAllocateMemory(const VkMemoryAllocateInfo* pAllocateInfo, VkDeviceMemory* pMemory) const noexcept
		{
			return m_table.vkAllocateMemory(m_handle, pAllocateInfo, m_allocatorCallbacks, pMemory);
		}

		VkResult vkBindBufferMemory(const VkBuffer buffer, const VkDeviceMemory memory, const VkDeviceSize memoryOffset) const noexcept
		{
			return m_table.vkBindBufferMemory(m_handle, buffer, memory, memoryOffset);
		}

		VkResult vkBindImageMemory(const VkImage image, const VkDeviceMemory memory, const VkDeviceSize memoryOffset) const noexcept
		{
			return m_table.vkBindImageMemory(m_handle, image, memory, memoryOffset);
		}

		void vkCmdBeginQuery(const VkCommandBuffer commandBuffer, const VkQueryPool queryPool, const uint32_t query, const VkQueryControlFlags flags) const noexcept
		{
			m_table.vkCmdBeginQuery(commandBuffer, queryPool, query, flags);
		}

		void vkCmdBeginRenderPass(const VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, const VkSubpassContents contents) const noexcept
		{
			m_table.vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
		}

		void vkCmdBindDescriptorSets(const VkCommandBuffer commandBuffer, const VkPipelineBindPoint pipelineBindPoint, const VkPipelineLayout layout, const uint32_t firstSet, const uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, const uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) const noexcept
		{
			m_table.vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
		}

		void vkCmdBindIndexBuffer(const VkCommandBuffer commandBuffer, const VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType) const noexcept
		{
			m_table.vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
		}

		void vkCmdBindPipeline(const VkCommandBuffer commandBuffer, const VkPipelineBindPoint pipelineBindPoint, const VkPipeline pipeline) const noexcept
		{
			m_table.vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
		}

		void vkCmdBindVertexBuffers(const VkCommandBuffer commandBuffer, const uint32_t firstBinding, const uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const noexcept
		{
			m_table.vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
		}

		void vkCmdBlitImage(const VkCommandBuffer commandBuffer, const VkImage srcImage, const VkImageLayout srcImageLayout, const VkImage dstImage, const VkImageLayout dstImageLayout, const uint32_t regionCount, const VkImageBlit* pRegions, const VkFilter filter) const noexcept
		{
			m_table.vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
		}

		void vkCmdClearAttachments(const VkCommandBuffer commandBuffer, const uint32_t attachmentCount, const VkClearAttachment* pAttachments, const uint32_t rectCount, const VkClearRect* pRects) const noexcept
		{
			m_table.vkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
		}

		void vkCmdClearColorImage(const VkCommandBuffer commandBuffer, const VkImage image, const VkImageLayout imageLayout, const VkClearColorValue* pColor, const uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const noexcept
		{
			m_table.vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
		}

		void vkCmdClearDepthStencilImage(const VkCommandBuffer commandBuffer, const VkImage image, const VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, const uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const noexcept
		{
			m_table.vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
		}

		void vkCmdCopyBuffer(const VkCommandBuffer commandBuffer, const VkBuffer srcBuffer, const VkBuffer dstBuffer, const uint32_t regionCount, const VkBufferCopy* pRegions) const noexcept
		{
			m_table.vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
		}

		void vkCmdCopyBufferToImage(const VkCommandBuffer commandBuffer, const VkBuffer srcBuffer, const VkImage dstImage, const VkImageLayout dstImageLayout, const uint32_t regionCount, const VkBufferImageCopy* pRegions) const noexcept
		{
			m_table.vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
		}

		void vkCmdCopyImage(const VkCommandBuffer commandBuffer, const VkImage srcImage, const VkImageLayout srcImageLayout, const VkImage dstImage, const VkImageLayout dstImageLayout, const uint32_t regionCount, const VkImageCopy* pRegions) const noexcept
		{
			m_table.vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
		}

		void vkCmdCopyImageToBuffer(const VkCommandBuffer commandBuffer, const VkImage srcImage, const VkImageLayout srcImageLayout, const VkBuffer dstBuffer, const uint32_t regionCount, const VkBufferImageCopy* pRegions) const noexcept
		{
			m_table.vkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
		}

		void vkCmdCopyQueryPoolResults(const VkCommandBuffer commandBuffer, const VkQueryPool queryPool, const uint32_t firstQuery, const uint32_t queryCount, const VkBuffer dstBuffer, const VkDeviceSize dstOffset, const VkDeviceSize stride, const VkQueryResultFlags flags) const noexcept
		{
			m_table.vkCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
		}

		void vkCmdDispatch(const VkCommandBuffer commandBuffer, const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) const noexcept
		{
			m_table.vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
		}

		void vkCmdDispatchIndirect(const VkCommandBuffer commandBuffer, const VkBuffer buffer, const VkDeviceSize offset) const noexcept
		{
			m_table.vkCmdDispatchIndirect(commandBuffer, buffer, offset);
		}

		void vkCmdDraw(const VkCommandBuffer commandBuffer, const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) const noexcept
		{
			m_table.vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
		}

		void vkCmdDrawIndexed(const VkCommandBuffer commandBuffer, const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) const noexcept
		{
			m_table.vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}

		void vkCmdDrawIndexedIndirect(const VkCommandBuffer commandBuffer, const VkBuffer buffer, const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride) const noexcept
		{
			m_table.vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
		}

		void vkCmdDrawIndirect(const VkCommandBuffer commandBuffer, const VkBuffer buffer, const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride) const noexcept
		{
			m_table.vkCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
		}

		void vkCmdEndQuery(const VkCommandBuffer commandBuffer, const VkQueryPool queryPool, const uint32_t query) const noexcept
		{
			m_table.vkCmdEndQuery(commandBuffer, queryPool, query);
		}

		void vkCmdEndRenderPass(const VkCommandBuffer commandBuffer) const noexcept
		{
			m_table.vkCmdEndRenderPass(commandBuffer);
		}

		void vkCmdExecuteCommands(const VkCommandBuffer commandBuffer, const uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const noexcept
		{
			m_table.vkCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
		}

		void vkCmdFillBuffer(const VkCommandBuffer commandBuffer, const VkBuffer dstBuffer, const VkDeviceSize dstOffset, const VkDeviceSize size, const uint32_t data) const noexcept
		{
			m_table.vkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
		}

		void vkCmdNextSubpass(const VkCommandBuffer commandBuffer, const VkSubpassContents contents) const noexcept
		{
			m_table.vkCmdNextSubpass(commandBuffer, contents);
		}

		void vkCmdPipelineBarrier(const VkCommandBuffer commandBuffer, const VkPipelineStageFlags srcStageMask, const VkPipelineStageFlags dstStageMask, const VkDependencyFlags dependencyFlags, const uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, const uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, const uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const noexcept
		{
			m_table.vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
			                             pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
		}

		void vkCmdPushConstants(const VkCommandBuffer commandBuffer, const VkPipelineLayout layout, const VkShaderStageFlags stageFlags, const uint32_t offset, const uint32_t size, const void* pValues) const noexcept
		{
			m_table.vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
		}

		void vkCmdResetEvent(
			const VkCommandBuffer                             commandBuffer,
			const VkEvent                                     event,
			const VkPipelineStageFlags                        stageMask) const noexcept
		{
			m_table.vkCmdResetEvent(commandBuffer, event, stageMask);
		}
		void vkCmdResetQueryPool(
			const VkCommandBuffer                             commandBuffer,
			const VkQueryPool                                 queryPool,
			const uint32_t                                    firstQuery,
			const uint32_t                                    queryCount) const noexcept
		{
			m_table.vkCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
		}
		void vkCmdResolveImage(
			const VkCommandBuffer                             commandBuffer,
			const VkImage                                     srcImage,
			const VkImageLayout                               srcImageLayout,
			const VkImage                                     dstImage,
			const VkImageLayout                               dstImageLayout,
			const uint32_t                                    regionCount,
			const VkImageResolve* pRegions) const noexcept
		{
			m_table.vkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
		}
		void vkCmdSetBlendConstants(
			const VkCommandBuffer                             commandBuffer,
			const float                                 blendConstants[4]) const noexcept
		{
			m_table.vkCmdSetBlendConstants(commandBuffer, blendConstants);
		}
		void vkCmdSetDepthBias(
			const VkCommandBuffer                             commandBuffer,
			const float                                       depthBiasConstantFactor,
			const float                                       depthBiasClamp,
			const float                                       depthBiasSlopeFactor) const noexcept
		{
			m_table.vkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
		}

		void vkCmdSetDepthBounds(
			const VkCommandBuffer                             commandBuffer,
			const float                                       minDepthBounds,
			const float                                       maxDepthBounds) const noexcept
		{
			m_table.vkCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
		}
		void vkCmdSetEvent(
			const VkCommandBuffer                             commandBuffer,
			const VkEvent                                     event,
			const VkPipelineStageFlags                        stageMask) const noexcept
		{
			m_table.vkCmdSetEvent(commandBuffer, event, stageMask);
		}
		void vkCmdSetLineWidth(
			const VkCommandBuffer                             commandBuffer,
			const float                                       lineWidth) const noexcept
		{
			m_table.vkCmdSetLineWidth(commandBuffer, lineWidth);
		}

		void vkCmdSetScissor(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    firstScissor,
			const uint32_t                                    scissorCount,
			const VkRect2D* pScissors) const noexcept
		{
			m_table.vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
		}
		void vkCmdSetStencilCompareMask(
			const VkCommandBuffer                             commandBuffer,
			const VkStencilFaceFlags                          faceMask,
			const uint32_t                                    compareMask) const noexcept
		{
			m_table.vkCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
		}
		void vkCmdSetStencilReference(
			const VkCommandBuffer                             commandBuffer,
			const VkStencilFaceFlags                          faceMask,
			const uint32_t                                    reference) const noexcept
		{
			m_table.vkCmdSetStencilReference(commandBuffer, faceMask, reference);
		}
		void vkCmdSetStencilWriteMask(
			const VkCommandBuffer                             commandBuffer,
			const VkStencilFaceFlags                          faceMask,
			const uint32_t                                    writeMask) const noexcept
		{
			m_table.vkCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
		}
		void vkCmdSetViewport(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    firstViewport,
			const uint32_t                                    viewportCount,
			const VkViewport* pViewports) const noexcept
		{
			m_table.vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
		}

		void vkCmdUpdateBuffer(
			const VkCommandBuffer                             commandBuffer,
			const VkBuffer                                    dstBuffer,
			const VkDeviceSize                                dstOffset,
			const VkDeviceSize                                dataSize,
			const void* pData) const noexcept
		{
			m_table.vkCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
		}

		void vkCmdWaitEvents(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    eventCount,
			const VkEvent* pEvents,
			const VkPipelineStageFlags                        srcStageMask,
			const VkPipelineStageFlags                        dstStageMask,
			const uint32_t                                    memoryBarrierCount,
			const VkMemoryBarrier* pMemoryBarriers,
			const uint32_t                                    bufferMemoryBarrierCount,
			const VkBufferMemoryBarrier* pBufferMemoryBarriers,
			const uint32_t                                    imageMemoryBarrierCount,
			const VkImageMemoryBarrier* pImageMemoryBarriers) const noexcept
		{
			m_table.vkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
			                        pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
		}
		void vkCmdWriteTimestamp(
			const VkCommandBuffer                             commandBuffer,
			const VkPipelineStageFlagBits                     pipelineStage,
			const VkQueryPool                                 queryPool,
			const uint32_t                                    query) const noexcept
		{
			m_table.vkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
		}
		VkResult vkCreateBuffer(
			const VkBufferCreateInfo* pCreateInfo,
			VkBuffer* pBuffer) const noexcept
		{
			return m_table.vkCreateBuffer(m_handle, pCreateInfo, m_allocatorCallbacks, pBuffer);
		}
		VkResult vkCreateBufferView(
			const VkBufferViewCreateInfo* pCreateInfo,
			VkBufferView* pView) const noexcept
		{
			return m_table.vkCreateBufferView(m_handle, pCreateInfo, m_allocatorCallbacks, pView);
		}
		VkResult vkCreateCommandPool(
			const VkCommandPoolCreateInfo* pCreateInfo,
			VkCommandPool* pCommandPool) const noexcept
		{
			return m_table.vkCreateCommandPool(m_handle, pCreateInfo, m_allocatorCallbacks, pCommandPool);
		}
		VkResult vkCreateComputePipelines(
			const VkPipelineCache                             pipelineCache,
			const uint32_t                                    createInfoCount,
			const VkComputePipelineCreateInfo* pCreateInfos,
			VkPipeline* pPipelines) const noexcept
		{
			return m_table.vkCreateComputePipelines(m_handle, pipelineCache, createInfoCount, pCreateInfos, m_allocatorCallbacks, pPipelines);
		}

		VkResult vkCreateDescriptorPool(
			const VkDescriptorPoolCreateInfo* pCreateInfo,
			VkDescriptorPool* pDescriptorPool) const noexcept
		{
			return m_table.vkCreateDescriptorPool(m_handle, pCreateInfo, m_allocatorCallbacks, pDescriptorPool);
		}

		VkResult vkCreateDescriptorSetLayout(
			const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
			VkDescriptorSetLayout* pSetLayout) const noexcept
		{
			return m_table.vkCreateDescriptorSetLayout(m_handle, pCreateInfo, m_allocatorCallbacks, pSetLayout);
		}
		VkResult vkCreateEvent(
			const VkEventCreateInfo* pCreateInfo,
			VkEvent* pEvent) const noexcept
		{
			return m_table.vkCreateEvent(m_handle, pCreateInfo, m_allocatorCallbacks, pEvent);
		}

		VkResult vkCreateFence(
			const VkFenceCreateInfo* pCreateInfo,
			VkFence* pFence) const noexcept
		{
			return m_table.vkCreateFence(m_handle, pCreateInfo, m_allocatorCallbacks, pFence);
		}
		VkResult vkCreateFramebuffer(
			const VkFramebufferCreateInfo* pCreateInfo,
			VkFramebuffer* pFramebuffer) const noexcept
		{
			return m_table.vkCreateFramebuffer(m_handle, pCreateInfo, m_allocatorCallbacks, pFramebuffer);
		}
		VkResult vkCreateGraphicsPipelines(
			const VkPipelineCache                             pipelineCache,
			const uint32_t                                    createInfoCount,
			const VkGraphicsPipelineCreateInfo* pCreateInfos,
			VkPipeline* pPipelines) const noexcept
		{
			return m_table.vkCreateGraphicsPipelines(m_handle, pipelineCache, createInfoCount, pCreateInfos, m_allocatorCallbacks, pPipelines);
		}

		VkResult vkCreateImage(
			const VkImageCreateInfo* pCreateInfo,
			VkImage* pImage) const noexcept
		{
			return m_table.vkCreateImage(m_handle, pCreateInfo, m_allocatorCallbacks, pImage);
		}
		VkResult vkCreateImageView(
			const VkImageViewCreateInfo* pCreateInfo,
			VkImageView* pView) const noexcept
		{
			return m_table.vkCreateImageView(m_handle, pCreateInfo, m_allocatorCallbacks, pView);
		}
		VkResult vkCreatePipelineCache(
			const VkPipelineCacheCreateInfo* pCreateInfo,
			VkPipelineCache* pPipelineCache) const noexcept
		{
			return m_table.vkCreatePipelineCache(m_handle, pCreateInfo, m_allocatorCallbacks, pPipelineCache);
		}

		VkResult vkCreatePipelineLayout(
			const VkPipelineLayoutCreateInfo* pCreateInfo,
			VkPipelineLayout* pPipelineLayout) const noexcept
		{
			return m_table.vkCreatePipelineLayout(m_handle, pCreateInfo, m_allocatorCallbacks, pPipelineLayout);
		}
		VkResult vkCreateQueryPool(
			const VkQueryPoolCreateInfo* pCreateInfo,
			VkQueryPool* pQueryPool) const noexcept
		{
			return m_table.vkCreateQueryPool(m_handle, pCreateInfo, m_allocatorCallbacks, pQueryPool);
		}
		VkResult vkCreateRenderPass(
			const VkRenderPassCreateInfo* pCreateInfo,
			VkRenderPass* pRenderPass) const noexcept
		{
			return m_table.vkCreateRenderPass(m_handle, pCreateInfo, m_allocatorCallbacks, pRenderPass);
		}

		VkResult vkCreateSampler(
			const VkSamplerCreateInfo* pCreateInfo,
			VkSampler* pSampler) const noexcept
		{
			return m_table.vkCreateSampler(m_handle, pCreateInfo, m_allocatorCallbacks, pSampler);
		}

		VkResult vkCreateSemaphore(
			const VkSemaphoreCreateInfo* pCreateInfo,
			VkSemaphore* pSemaphore) const noexcept
		{
			return m_table.vkCreateSemaphore(m_handle, pCreateInfo, m_allocatorCallbacks, pSemaphore);
		}
		VkResult vkCreateShaderModule(
			const VkShaderModuleCreateInfo* pCreateInfo,
			VkShaderModule* pShaderModule) const noexcept
		{
			return m_table.vkCreateShaderModule(m_handle, pCreateInfo, m_allocatorCallbacks, pShaderModule);
		}
		void vkDestroyBuffer(
			const VkBuffer                                    buffer) const noexcept
		{
			m_table.vkDestroyBuffer(m_handle, buffer, m_allocatorCallbacks);
		}
		void vkDestroyBufferView(
			const VkBufferView                                bufferView) const noexcept
		{
			m_table.vkDestroyBufferView(m_handle, bufferView, m_allocatorCallbacks);
		}
		void vkDestroyCommandPool(
			const VkCommandPool                               commandPool) const noexcept
		{
			m_table.vkDestroyCommandPool(m_handle, commandPool, m_allocatorCallbacks);
		}
		void vkDestroyDescriptorPool(
			const VkDescriptorPool                            descriptorPool) const noexcept
		{
			m_table.vkDestroyDescriptorPool(m_handle, descriptorPool, m_allocatorCallbacks);
		}
		void vkDestroyDescriptorSetLayout(
			const VkDescriptorSetLayout                       descriptorSetLayout) const noexcept
		{
			m_table.vkDestroyDescriptorSetLayout(m_handle, descriptorSetLayout, m_allocatorCallbacks);
		}

		void vkDestroyDevice() const noexcept
		{
			m_table.vkDestroyDevice(m_handle, m_allocatorCallbacks);
		}

		void vkDestroyEvent(
			const VkEvent                                     event) const noexcept
		{
			m_table.vkDestroyEvent(m_handle, event, m_allocatorCallbacks);
		}

		void vkDestroyFence(
			const VkFence                                     fence) const noexcept
		{
			m_table.vkDestroyFence(m_handle, fence, m_allocatorCallbacks);
		}

		void vkDestroyFramebuffer(
			const VkFramebuffer                               framebuffer) const noexcept
		{
			m_table.vkDestroyFramebuffer(m_handle, framebuffer, m_allocatorCallbacks);
		}
		void vkDestroyImage(
			const VkImage                                     image) const noexcept
		{
			m_table.vkDestroyImage(m_handle, image, m_allocatorCallbacks);
		}
		void vkDestroyImageView(
			const VkImageView                                 imageView) const noexcept
		{
			m_table.vkDestroyImageView(m_handle, imageView, m_allocatorCallbacks);
		}
		void vkDestroyPipeline(
			const VkPipeline                                  pipeline) const noexcept
		{
			m_table.vkDestroyPipeline(m_handle, pipeline, m_allocatorCallbacks);
		}
		void vkDestroyPipelineCache(
			const VkPipelineCache                             pipelineCache) const noexcept
		{
			m_table.vkDestroyPipelineCache(m_handle, pipelineCache, m_allocatorCallbacks);
		}
		void vkDestroyPipelineLayout(
			const VkPipelineLayout                            pipelineLayout) const noexcept
		{
			m_table.vkDestroyPipelineLayout(m_handle, pipelineLayout, m_allocatorCallbacks);
		}
		void vkDestroyQueryPool(
			const VkQueryPool                                 queryPool) const noexcept
		{
			m_table.vkDestroyQueryPool(m_handle, queryPool, m_allocatorCallbacks);
		}

		void vkDestroyRenderPass(
			const VkRenderPass                                renderPass) const noexcept
		{
			m_table.vkDestroyRenderPass(m_handle, renderPass, m_allocatorCallbacks);
		}

		void vkDestroySampler(
			const VkSampler                                   sampler) const noexcept
		{
			m_table.vkDestroySampler(m_handle, sampler, m_allocatorCallbacks);
		}
		void vkDestroySemaphore(
			const VkSemaphore                                 semaphore) const noexcept
		{
			m_table.vkDestroySemaphore(m_handle, semaphore, m_allocatorCallbacks);
		}

		void vkDestroyShaderModule(
			const VkShaderModule                              shaderModule) const noexcept
		{
			m_table.vkDestroyShaderModule(m_handle, shaderModule, m_allocatorCallbacks);
		}

		VkResult vkDeviceWaitIdle() const noexcept
		{
			return m_table.vkDeviceWaitIdle(m_handle);
		}
		VkResult vkBeginCommandBuffer(
			const VkCommandBuffer           commandBuffer,
			const VkCommandBufferBeginInfo* pBeginInfo) const noexcept
		{
			return m_table.vkBeginCommandBuffer(commandBuffer, pBeginInfo);
		}

		VkResult vkEndCommandBuffer(
			const VkCommandBuffer                             commandBuffer) const noexcept
		{
			return m_table.vkEndCommandBuffer(commandBuffer);
		}
		VkResult vkFlushMappedMemoryRanges(
			const uint32_t                                    memoryRangeCount,
			const VkMappedMemoryRange* pMemoryRanges) const noexcept
		{
			return m_table.vkFlushMappedMemoryRanges(m_handle, memoryRangeCount, pMemoryRanges);
		}
		void vkFreeCommandBuffers(
			const VkCommandPool                               commandPool,
			const uint32_t                                    commandBufferCount,
			const VkCommandBuffer* pCommandBuffers) const noexcept
		{
			m_table.vkFreeCommandBuffers(m_handle, commandPool, commandBufferCount, pCommandBuffers);
		}
		VkResult vkFreeDescriptorSets(
			const VkDescriptorPool                            descriptorPool,
			const uint32_t                                    descriptorSetCount,
			const VkDescriptorSet* pDescriptorSets) const noexcept
		{
			return m_table.vkFreeDescriptorSets(m_handle, descriptorPool, descriptorSetCount, pDescriptorSets);
		}

		void vkFreeMemory(
			const VkDeviceMemory                              memory) const noexcept
		{
			m_table.vkFreeMemory(m_handle, memory, m_allocatorCallbacks);
		}
		void vkGetBufferMemoryRequirements(
			const VkBuffer                                    buffer,
			VkMemoryRequirements* pMemoryRequirements) const noexcept
		{
			m_table.vkGetBufferMemoryRequirements(m_handle, buffer, pMemoryRequirements);
		}
		void vkGetDeviceMemoryCommitment(
			const VkDeviceMemory                              memory,
			VkDeviceSize* pCommittedMemoryInBytes) const noexcept
		{
			m_table.vkGetDeviceMemoryCommitment(m_handle, memory, pCommittedMemoryInBytes);
		}

		void vkGetDeviceQueue(
			const uint32_t                                    queueFamilyIndex,
			const uint32_t                                    queueIndex,
			VkQueue* pQueue) const noexcept
		{
			m_table.vkGetDeviceQueue(m_handle, queueFamilyIndex, queueIndex, pQueue);
		}
		VkResult vkGetEventStatus(
			const VkEvent                                     event) const noexcept
		{
			return m_table.vkGetEventStatus(m_handle, event);
		}
		VkResult vkGetFenceStatus(
			const VkFence                                     fence) const noexcept
		{
			return m_table.vkGetFenceStatus(m_handle, fence);
		}
		void  vkGetImageMemoryRequirements(
			const VkImage                                     image,
			VkMemoryRequirements* pMemoryRequirements) const noexcept
		{
			m_table.vkGetImageMemoryRequirements(m_handle, image, pMemoryRequirements);
		}
		void vkGetImageSparseMemoryRequirements(
			const VkImage                                     image,
			uint32_t* pSparseMemoryRequirementCount,
			VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const noexcept
		{
			m_table.vkGetImageSparseMemoryRequirements(m_handle, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
		}
		void  vkGetImageSubresourceLayout(
			const VkImage                                     image,
			const VkImageSubresource* pSubresource,
			VkSubresourceLayout* pLayout) const noexcept
		{
			m_table.vkGetImageSubresourceLayout(m_handle, image, pSubresource, pLayout);
		}
		VkResult vkGetPipelineCacheData(
			const VkPipelineCache                             pipelineCache,
			size_t* pDataSize,
			void* pData) const noexcept
		{
			return m_table.vkGetPipelineCacheData(m_handle, pipelineCache, pDataSize, pData);
		}
		VkResult vkGetQueryPoolResults(
			const VkQueryPool                                 queryPool,
			const uint32_t                                    firstQuery,
			const uint32_t                                    queryCount,
			const size_t                                      dataSize,
			void* pData,
			const VkDeviceSize                                stride,
			const VkQueryResultFlags                          flags) const noexcept
		{
			return m_table.vkGetQueryPoolResults(m_handle, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
		}
		void vkGetRenderAreaGranularity(
			const VkRenderPass                                renderPass,
			VkExtent2D* pGranularity) const noexcept
		{
			m_table.vkGetRenderAreaGranularity(m_handle, renderPass, pGranularity);
		}
		VkResult vkInvalidateMappedMemoryRanges(
			const uint32_t                                    memoryRangeCount,
			const VkMappedMemoryRange* pMemoryRanges) const noexcept
		{
			return m_table.vkInvalidateMappedMemoryRanges(m_handle, memoryRangeCount, pMemoryRanges);
		}
		VkResult vkMapMemory(
			const VkDeviceMemory                              memory,
			const VkDeviceSize                                offset,
			const VkDeviceSize                                size,
			const VkMemoryMapFlags                            flags,
			void** ppData) const noexcept
		{
			return m_table.vkMapMemory(m_handle, memory, offset, size, flags, ppData);
		}

		VkResult vkMergePipelineCaches(
			const VkPipelineCache                             dstCache,
			const uint32_t                                    srcCacheCount,
			const VkPipelineCache* pSrcCaches) const noexcept
		{
			return m_table.vkMergePipelineCaches(m_handle, dstCache, srcCacheCount, pSrcCaches);
		}

		VkResult vkQueueBindSparse(
			const VkQueue                                     queue,
			const uint32_t                                    bindInfoCount,
			const VkBindSparseInfo* pBindInfo,
			const VkFence                                     fence) const noexcept
		{
			return m_table.vkQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
		}
		VkResult vkQueueSubmit(
			const VkQueue                                     queue,
			const uint32_t                                    submitCount,
			const VkSubmitInfo* pSubmits,
			const VkFence                                     fence) const noexcept
		{
			return m_table.vkQueueSubmit(queue, submitCount, pSubmits, fence);
		}
		VkResult vkQueueWaitIdle(
			const VkQueue                                     queue) const noexcept
		{
			return m_table.vkQueueWaitIdle(queue);
		}

		VkResult vkResetCommandBuffer(
			const VkCommandBuffer                             commandBuffer,
			const VkCommandBufferResetFlags                   flags) const noexcept
		{
			return m_table.vkResetCommandBuffer(commandBuffer, flags);
		}
		VkResult vkResetCommandPool(
			const VkCommandPool                               commandPool,
			const VkCommandPoolResetFlags                     flags) const noexcept
		{
			return m_table.vkResetCommandPool(m_handle, commandPool, flags);
		}
		VkResult vkResetDescriptorPool(
			const VkDescriptorPool                            descriptorPool,
			const VkDescriptorPoolResetFlags                  flags) const noexcept
		{
			return m_table.vkResetDescriptorPool(m_handle, descriptorPool, flags);
		}

		VkResult vkResetEvent(
			const VkEvent                                     event) const noexcept
		{
			return m_table.vkResetEvent(m_handle, event);
		}
		VkResult vkResetFences(
			const uint32_t                                    fenceCount,
			const VkFence* pFences) const noexcept
		{
			return m_table.vkResetFences(m_handle, fenceCount, pFences);
		}
		VkResult vkSetEvent(
			const VkEvent                                     event) const noexcept
		{
			return m_table.vkSetEvent(m_handle, event);
		}

		void  vkUnmapMemory(
			const VkDeviceMemory                              memory) const noexcept
		{
			m_table.vkUnmapMemory(m_handle, memory);
		}

		void  vkUpdateDescriptorSets(
			const uint32_t                                    descriptorWriteCount,
			const VkWriteDescriptorSet* pDescriptorWrites,
			const uint32_t                                    descriptorCopyCount,
			const VkCopyDescriptorSet* pDescriptorCopies) const noexcept
		{
			m_table.vkUpdateDescriptorSets(m_handle, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
		}

		VkResult vkWaitForFences(
			const uint32_t                                    fenceCount,
			const VkFence* pFences,
			const VkBool32                                    waitAll,
			const uint64_t                                    timeout) const noexcept
		{
			return m_table.vkWaitForFences(m_handle, fenceCount, pFences, waitAll, timeout);
		}

#endif
#if defined(VK_VERSION_1_1)
		VkResult vkBindBufferMemory2(
			const uint32_t                                    bindInfoCount,
			const VkBindBufferMemoryInfo* pBindInfos) const noexcept
		{
			return m_table.vkBindBufferMemory2(m_handle, bindInfoCount, pBindInfos);
		}
		VkResult vkBindImageMemory2(
			const uint32_t                                    bindInfoCount,
			const VkBindImageMemoryInfo* pBindInfos) const noexcept
		{
			return m_table.vkBindImageMemory2(m_handle, bindInfoCount, pBindInfos);
		}
		void vkCmdDispatchBase(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    baseGroupX,
			const uint32_t                                    baseGroupY,
			const uint32_t                                    baseGroupZ,
			const uint32_t                                    groupCountX,
			const uint32_t                                    groupCountY,
			const uint32_t                                    groupCountZ) const noexcept
		{
			m_table.vkCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
		}
		void vkCmdSetDeviceMask(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    deviceMask) const noexcept
		{
			m_table.vkCmdSetDeviceMask(commandBuffer, deviceMask);
		}

		VkResult vkCreateDescriptorUpdateTemplate(
			const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
			VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const noexcept
		{
			return m_table.vkCreateDescriptorUpdateTemplate(m_handle, pCreateInfo, m_allocatorCallbacks, pDescriptorUpdateTemplate);
		}
		VkResult vkCreateSamplerYcbcrConversion(
			const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
			VkSamplerYcbcrConversion* pYcbcrConversion) const noexcept
		{
			return m_table.vkCreateSamplerYcbcrConversion(m_handle, pCreateInfo, m_allocatorCallbacks, pYcbcrConversion);
		}
		void  vkDestroyDescriptorUpdateTemplate(
			const VkDescriptorUpdateTemplate                  descriptorUpdateTemplate) const noexcept
		{
			m_table.vkDestroyDescriptorUpdateTemplate(m_handle, descriptorUpdateTemplate, m_allocatorCallbacks);
		}
		void vkDestroySamplerYcbcrConversion(
			const VkSamplerYcbcrConversion                    ycbcrConversion) const noexcept
		{
			m_table.vkDestroySamplerYcbcrConversion(m_handle, ycbcrConversion, m_allocatorCallbacks);
		}
		void vkGetBufferMemoryRequirements2(
			const VkBufferMemoryRequirementsInfo2* pInfo,
			VkMemoryRequirements2* pMemoryRequirements) const noexcept
		{
			m_table.vkGetBufferMemoryRequirements2(m_handle, pInfo, pMemoryRequirements);
		}
		void  vkGetDescriptorSetLayoutSupport(
			const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
			VkDescriptorSetLayoutSupport* pSupport) const noexcept
		{
			m_table.vkGetDescriptorSetLayoutSupport(m_handle, pCreateInfo, pSupport);
		}
		void  vkGetDeviceGroupPeerMemoryFeatures(
			const uint32_t                                    heapIndex,
			const uint32_t                                    localDeviceIndex,
			const uint32_t                                    remoteDeviceIndex,
			VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const noexcept
		{
			m_table.vkGetDeviceGroupPeerMemoryFeatures(m_handle, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
		}
		void  vkGetDeviceQueue2(
			const VkDeviceQueueInfo2* pQueueInfo,
			VkQueue* pQueue) const noexcept
		{
			m_table.vkGetDeviceQueue2(m_handle, pQueueInfo, pQueue);
		}
		void vkGetImageMemoryRequirements2(
			const VkImageMemoryRequirementsInfo2* pInfo,
			VkMemoryRequirements2* pMemoryRequirements) const noexcept
		{
			m_table.vkGetImageMemoryRequirements2(m_handle, pInfo, pMemoryRequirements);
		}
		void vkGetImageSparseMemoryRequirements2(
			const VkImageSparseMemoryRequirementsInfo2* pInfo,
			uint32_t* pSparseMemoryRequirementCount,
			VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const noexcept
		{
			m_table.vkGetImageSparseMemoryRequirements2(m_handle, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
		}
		void  vkTrimCommandPool(
			const VkCommandPool                               commandPool,
			const VkCommandPoolTrimFlags                      flags) const noexcept
		{
			m_table.vkTrimCommandPool(m_handle, commandPool, flags);
		}
		void  vkUpdateDescriptorSetWithTemplate(
			const VkDescriptorSet                             descriptorSet,
			const VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
			const void* pData) const noexcept
		{
			m_table.vkUpdateDescriptorSetWithTemplate(m_handle, descriptorSet, descriptorUpdateTemplate, pData);
		}
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_2)
		void  vkCmdBeginRenderPass2(
			const VkCommandBuffer                             commandBuffer,
			const VkRenderPassBeginInfo* pRenderPassBegin,
			const VkSubpassBeginInfo* pSubpassBeginInfo) const noexcept
		{
			m_table.vkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
		}

		void  vkCmdDrawIndexedIndirectCount(
			const VkCommandBuffer                             commandBuffer,
			const VkBuffer                                    buffer,
			const VkDeviceSize                                offset,
			const VkBuffer                                    countBuffer,
			const VkDeviceSize                                countBufferOffset,
			const uint32_t                                    maxDrawCount,
			const uint32_t                                    stride) const noexcept
		{
			m_table.vkCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
		}
		void  vkCmdDrawIndirectCount(
			const VkCommandBuffer                             commandBuffer,
			const VkBuffer                                    buffer,
			const VkDeviceSize                                offset,
			const VkBuffer                                    countBuffer,
			const VkDeviceSize                                countBufferOffset,
			const uint32_t                                    maxDrawCount,
			const uint32_t                                    stride) const noexcept
		{
			m_table.vkCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
		}
		void  vkCmdEndRenderPass2(
			const VkCommandBuffer                             commandBuffer,
			const VkSubpassEndInfo* pSubpassEndInfo) const noexcept
		{
			m_table.vkCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
		}
		void  vkCmdNextSubpass2(
			const VkCommandBuffer                             commandBuffer,
			const VkSubpassBeginInfo* pSubpassBeginInfo,
			const VkSubpassEndInfo* pSubpassEndInfo) const noexcept
		{
			m_table.vkCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
		}

		VkResult  vkCreateRenderPass2(
			const VkRenderPassCreateInfo2* pCreateInfo,
			VkRenderPass* pRenderPass) const noexcept
		{
			return m_table.vkCreateRenderPass2(m_handle, pCreateInfo, m_allocatorCallbacks, pRenderPass);
		}
		VkDeviceAddress  vkGetBufferDeviceAddress(
			const VkBufferDeviceAddressInfo* pInfo) const noexcept
		{
			return m_table.vkGetBufferDeviceAddress(m_handle, pInfo);
		}
		uint64_t  vkGetBufferOpaqueCaptureAddress(
			const VkBufferDeviceAddressInfo* pInfo) const noexcept
		{
			return m_table.vkGetBufferOpaqueCaptureAddress(m_handle, pInfo);
		}
		uint64_t  vkGetDeviceMemoryOpaqueCaptureAddress(
			const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const noexcept
		{
			return m_table.vkGetDeviceMemoryOpaqueCaptureAddress(m_handle, pInfo);
		}
		VkResult  vkGetSemaphoreCounterValue(
			const VkSemaphore                                 semaphore,
			uint64_t* pValue) const noexcept
		{
			return m_table.vkGetSemaphoreCounterValue(m_handle, semaphore, pValue);
		}
		void  vkResetQueryPool(
			const VkQueryPool                                 queryPool,
			const uint32_t                                    firstQuery,
			const uint32_t                                    queryCount) const noexcept
		{
			m_table.vkResetQueryPool(m_handle, queryPool, firstQuery, queryCount);
		}
		VkResult  vkSignalSemaphore(
			const VkSemaphoreSignalInfo* pSignalInfo) const noexcept
		{
			return m_table.vkSignalSemaphore(m_handle, pSignalInfo);
		}
		VkResult  vkWaitSemaphores(
			const VkSemaphoreWaitInfo* pWaitInfo,
			const uint64_t                                    timeout) const noexcept
		{
			return m_table.vkWaitSemaphores(m_handle, pWaitInfo, timeout);
		}
#endif /* defined(VK_VERSION_1_2) */
#if defined(VK_VERSION_1_3)
		void  vkCmdBeginRendering(
			const VkCommandBuffer                             commandBuffer,
			const VkRenderingInfo* pRenderingInfo) const noexcept
		{
			m_table.vkCmdBeginRendering(commandBuffer, pRenderingInfo);
		}

		void  vkCmdBindVertexBuffers2(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    firstBinding,
			const uint32_t                                    bindingCount,
			const VkBuffer* pBuffers,
			const VkDeviceSize* pOffsets,
			const VkDeviceSize* pSizes,
			const VkDeviceSize* pStrides) const noexcept
		{
			m_table.vkCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
		}
		void  vkCmdBlitImage2(
			const VkCommandBuffer                             commandBuffer,
			const VkBlitImageInfo2* pBlitImageInfo) const noexcept
		{
			m_table.vkCmdBlitImage2(commandBuffer, pBlitImageInfo);
		}

		void  vkCmdCopyBuffer2(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyBufferInfo2* pCopyBufferInfo) const noexcept
		{
			m_table.vkCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
		}
		void  vkCmdCopyBufferToImage2(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const noexcept
		{
			m_table.vkCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
		}
		void  vkCmdCopyImage2(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyImageInfo2* pCopyImageInfo) const noexcept
		{
			m_table.vkCmdCopyImage2(commandBuffer, pCopyImageInfo);
		}
		void  vkCmdCopyImageToBuffer2(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const noexcept
		{
			m_table.vkCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
		}
		void  vkCmdEndRendering(
			const VkCommandBuffer                             commandBuffer) const noexcept
		{
			m_table.vkCmdEndRendering(commandBuffer);
		}
		void  vkCmdPipelineBarrier2(
			const VkCommandBuffer                             commandBuffer,
			const VkDependencyInfo* pDependencyInfo) const noexcept
		{
			m_table.vkCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
		}
		void  vkCmdResetEvent2(
			const VkCommandBuffer                             commandBuffer,
			const VkEvent                                     event,
			const VkPipelineStageFlags2                       stageMask) const noexcept
		{
			m_table.vkCmdResetEvent2(commandBuffer, event, stageMask);
		}
		void  vkCmdResolveImage2(
			const VkCommandBuffer                             commandBuffer,
			const VkResolveImageInfo2* pResolveImageInfo) const noexcept
		{
			m_table.vkCmdResolveImage2(commandBuffer, pResolveImageInfo);
		}
		void  vkCmdSetCullMode(
			const VkCommandBuffer                             commandBuffer,
			const VkCullModeFlags                             cullMode) const noexcept
		{
			m_table.vkCmdSetCullMode(commandBuffer, cullMode);
		}
		void  vkCmdSetDepthBiasEnable(
			const VkCommandBuffer                             commandBuffer,
			const VkBool32                                    depthBiasEnable) const noexcept
		{
			m_table.vkCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
		}
		void  vkCmdSetDepthBoundsTestEnable(
			const VkCommandBuffer                             commandBuffer,
			const VkBool32                                    depthBoundsTestEnable) const noexcept
		{
			m_table.vkCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
		}
		void  vkCmdSetDepthCompareOp(
			const VkCommandBuffer                             commandBuffer,
			const VkCompareOp                                 depthCompareOp) const noexcept
		{
			m_table.vkCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
		}
		void  vkCmdSetDepthTestEnable(
			const VkCommandBuffer                             commandBuffer,
			const VkBool32                                    depthTestEnable) const noexcept
		{
			m_table.vkCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
		}
		void  vkCmdSetDepthWriteEnable(
			const VkCommandBuffer                             commandBuffer,
			const VkBool32                                    depthWriteEnable) const noexcept
		{
			m_table.vkCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
		}
		void  vkCmdSetEvent2(
			const VkCommandBuffer                             commandBuffer,
			const VkEvent                                     event,
			const VkDependencyInfo* pDependencyInfo) const noexcept
		{
			m_table.vkCmdSetEvent2(commandBuffer, event, pDependencyInfo);
		}
		void  vkCmdSetFrontFace(
			const VkCommandBuffer                             commandBuffer,
			const VkFrontFace                                 frontFace) const noexcept
		{
			m_table.vkCmdSetFrontFace(commandBuffer, frontFace);
		}

		void  vkCmdSetPrimitiveRestartEnable(
			const VkCommandBuffer                             commandBuffer,
			const VkBool32                                    primitiveRestartEnable) const noexcept
		{
			m_table.vkCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
		}

		void  vkCmdSetPrimitiveTopology(
			const VkCommandBuffer                             commandBuffer,
			const VkPrimitiveTopology                         primitiveTopology) const noexcept
		{
			m_table.vkCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
		}
		void  vkCmdSetRasterizerDiscardEnable(
			const VkCommandBuffer                             commandBuffer,
			const VkBool32                                    rasterizerDiscardEnable) const noexcept
		{
			m_table.vkCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
		}
		void  vkCmdSetScissorWithCount(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    scissorCount,
			const VkRect2D* pScissors) const noexcept
		{
			m_table.vkCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
		}
		void  vkCmdSetStencilOp(
			const VkCommandBuffer                             commandBuffer,
			const VkStencilFaceFlags                          faceMask,
			const VkStencilOp                                 failOp,
			const VkStencilOp                                 passOp,
			const VkStencilOp                                 depthFailOp,
			const VkCompareOp                                 compareOp) const noexcept
		{
			m_table.vkCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
		}

		void  vkCmdSetStencilTestEnable(
			const VkCommandBuffer                             commandBuffer,
			const VkBool32                                    stencilTestEnable) const noexcept
		{
			m_table.vkCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
		}

		void  vkCmdSetViewportWithCount(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    viewportCount,
			const VkViewport* pViewports) const noexcept
		{
			m_table.vkCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
		}
		void  vkCmdWaitEvents2(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    eventCount,
			const VkEvent* pEvents,
			const VkDependencyInfo* pDependencyInfos) const noexcept
		{
			m_table.vkCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
		}
		void  vkCmdWriteTimestamp2(
			const VkCommandBuffer                             commandBuffer,
			const VkPipelineStageFlags2                       stage,
			const VkQueryPool                                 queryPool,
			const uint32_t                                    query) const noexcept
		{
			m_table.vkCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
		}

		VkResult   vkCreatePrivateDataSlot(
			const VkPrivateDataSlotCreateInfo* pCreateInfo,
			VkPrivateDataSlot* pPrivateDataSlot) const noexcept
		{
			return m_table.vkCreatePrivateDataSlot(m_handle, pCreateInfo, m_allocatorCallbacks, pPrivateDataSlot);
		}

		void  vkDestroyPrivateDataSlot(
			const VkPrivateDataSlot                           privateDataSlot) const noexcept
		{
			m_table.vkDestroyPrivateDataSlot(m_handle, privateDataSlot, m_allocatorCallbacks);
		}
		void  vkGetDeviceBufferMemoryRequirements(
			const VkDeviceBufferMemoryRequirements* pInfo,
			VkMemoryRequirements2* pMemoryRequirements) const noexcept
		{
			m_table.vkGetDeviceBufferMemoryRequirements(m_handle, pInfo, pMemoryRequirements);
		}
		void  vkGetDeviceImageMemoryRequirements(
			const VkDeviceImageMemoryRequirements* pInfo,
			VkMemoryRequirements2* pMemoryRequirements) const noexcept
		{
			m_table.vkGetDeviceImageMemoryRequirements(m_handle, pInfo, pMemoryRequirements);
		}
		void  vkGetDeviceImageSparseMemoryRequirements(
			const VkDeviceImageMemoryRequirements* pInfo,
			uint32_t* pSparseMemoryRequirementCount,
			VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const noexcept
		{
			m_table.vkGetDeviceImageSparseMemoryRequirements(m_handle, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
		}
		void  vkGetPrivateData(
			const VkObjectType                                objectType,
			const uint64_t                                    objectHandle,
			const VkPrivateDataSlot                           privateDataSlot,
			uint64_t* pData) const noexcept
		{
			m_table.vkGetPrivateData(m_handle, objectType, objectHandle, privateDataSlot, pData);
		}
		VkResult   vkQueueSubmit2(
			const VkQueue                                     queue,
			const uint32_t                                    submitCount,
			const VkSubmitInfo2* pSubmits,
			const VkFence                                     fence) const noexcept
		{
			return m_table.vkQueueSubmit2(queue, submitCount, pSubmits, fence);
		}
		VkResult vkSetPrivateData(
			const VkObjectType                                objectType,
			const uint64_t                                    objectHandle,
			const VkPrivateDataSlot                           privateDataSlot,
			const uint64_t                                    data) const noexcept
		{
			return m_table.vkSetPrivateData(m_handle, objectType, objectHandle, privateDataSlot, data);
		}
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_EXT_full_screen_exclusive)
		VkResult vkAcquireFullScreenExclusiveModeEXT(
			const VkSwapchainKHR                              swapchain) const noexcept
		{
			return m_table.vkAcquireFullScreenExclusiveModeEXT(m_handle, swapchain);
		}
		VkResult vkReleaseFullScreenExclusiveModeEXT(
			const VkSwapchainKHR                              swapchain) const noexcept
		{
			return m_table.vkReleaseFullScreenExclusiveModeEXT(m_handle, swapchain);
		}
#endif /* defined(VK_EXT_full_screen_exclusive) */
#if defined(VK_EXT_mesh_shader)
		void  vkCmdDrawMeshTasksEXT(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    groupCountX,
			const uint32_t                                    groupCountY,
			const uint32_t                                    groupCountZ) const noexcept
		{
			m_table.vkCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
		}

		void  vkCmdDrawMeshTasksIndirectCountEXT(
			const VkCommandBuffer                             commandBuffer,
			const VkBuffer                                    buffer,
			const VkDeviceSize                                offset,
			const VkBuffer                                    countBuffer,
			const VkDeviceSize                                countBufferOffset,
			const uint32_t                                    maxDrawCount,
			const uint32_t                                    stride) const noexcept
		{
			m_table.vkCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
		}

		void  vkCmdDrawMeshTasksIndirectEXT(
			const VkCommandBuffer                             commandBuffer,
			const VkBuffer                                    buffer,
			const VkDeviceSize                                offset,
			const uint32_t                                    drawCount,
			const uint32_t                                    stride) const noexcept
		{
			m_table.vkCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
		}

#endif /* defined(VK_EXT_mesh_shader) */

#if defined(VK_EXT_opacity_micromap)
		VkResult  vkBuildMicromapsEXT(
			const VkDeferredOperationKHR                      deferredOperation,
			const uint32_t                                    infoCount,
			const VkMicromapBuildInfoEXT* pInfos) const noexcept
		{
			return m_table.vkBuildMicromapsEXT(m_handle, deferredOperation, infoCount, pInfos);
		}

		void  vkCmdBuildMicromapsEXT(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    infoCount,
			const VkMicromapBuildInfoEXT* pInfos) const noexcept
		{
			m_table.vkCmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
		}
		void  vkCmdCopyMemoryToMicromapEXT(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyMemoryToMicromapInfoEXT* pInfo) const noexcept
		{
			m_table.vkCmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
		}
		void  vkCmdCopyMicromapEXT(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyMicromapInfoEXT* pInfo) const noexcept
		{
			m_table.vkCmdCopyMicromapEXT(commandBuffer, pInfo);
		}
		void  vkCmdCopyMicromapToMemoryEXT(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyMicromapToMemoryInfoEXT* pInfo) const noexcept
		{
			m_table.vkCmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
		}
		void  vkCmdWriteMicromapsPropertiesEXT(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    micromapCount,
			const VkMicromapEXT* pMicromaps,
			const VkQueryType                                 queryType,
			const VkQueryPool                                 queryPool,
			const uint32_t                                    firstQuery) const noexcept
		{
			m_table.vkCmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
		}
		VkResult  vkCopyMemoryToMicromapEXT(
			const VkDeferredOperationKHR                      deferredOperation,
			const VkCopyMemoryToMicromapInfoEXT* pInfo) const noexcept
		{
			return m_table.vkCopyMemoryToMicromapEXT(m_handle, deferredOperation, pInfo);
		}
		VkResult  vkCopyMicromapEXT(
			const VkDeferredOperationKHR                      deferredOperation,
			const VkCopyMicromapInfoEXT* pInfo) const noexcept
		{
			return m_table.vkCopyMicromapEXT(m_handle, deferredOperation, pInfo);
		}
		VkResult  vkCopyMicromapToMemoryEXT(
			const VkDeferredOperationKHR                      deferredOperation,
			const VkCopyMicromapToMemoryInfoEXT* pInfo) const noexcept
		{
			return m_table.vkCopyMicromapToMemoryEXT(m_handle, deferredOperation, pInfo);
		}
		VkResult  vkCreateMicromapEXT(
			const VkMicromapCreateInfoEXT* pCreateInfo,
			VkMicromapEXT* pMicromap) const noexcept
		{
			return m_table.vkCreateMicromapEXT(m_handle, pCreateInfo, m_allocatorCallbacks, pMicromap);
		}

		void  vkDestroyMicromapEXT(
			const VkMicromapEXT                               micromap) const noexcept
		{
			m_table.vkDestroyMicromapEXT(m_handle, micromap, m_allocatorCallbacks);
		}
		void  vkGetDeviceMicromapCompatibilityEXT(
			const VkMicromapVersionInfoEXT* pVersionInfo,
			VkAccelerationStructureCompatibilityKHR* pCompatibility) const noexcept
		{
			m_table.vkGetDeviceMicromapCompatibilityEXT(m_handle, pVersionInfo, pCompatibility);
		}
		void  vkGetMicromapBuildSizesEXT(
			const VkAccelerationStructureBuildTypeKHR         buildType,
			const VkMicromapBuildInfoEXT* pBuildInfo,
			VkMicromapBuildSizesInfoEXT* pSizeInfo) const noexcept
		{
			m_table.vkGetMicromapBuildSizesEXT(m_handle, buildType, pBuildInfo, pSizeInfo);
		}
		VkResult  vkWriteMicromapsPropertiesEXT(
			const uint32_t                                    micromapCount,
			const VkMicromapEXT* pMicromaps,
			const VkQueryType                                 queryType,
			const size_t                                      dataSize,
			void* pData,
			const size_t                                      stride) const noexcept
		{
			return m_table.vkWriteMicromapsPropertiesEXT(m_handle, micromapCount, pMicromaps, queryType, dataSize, pData, stride);
		}
#endif /* defined(VK_EXT_opacity_micromap) */
#if defined(VK_EXT_swapchain_maintenance1)
		VkResult  vkReleaseSwapchainImagesEXT(
			const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) const noexcept
		{
			return m_table.vkReleaseSwapchainImagesEXT(m_handle, pReleaseInfo);
		}
#endif /* defined(VK_EXT_swapchain_maintenance1) */
#if defined(VK_KHR_acceleration_structure)
		VkResult  vkBuildAccelerationStructuresKHR(
			const VkDeferredOperationKHR                      deferredOperation,
			const uint32_t                                    infoCount,
			const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
			const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const noexcept
		{
			return m_table.vkBuildAccelerationStructuresKHR(m_handle, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
		}
		void  vkCmdBuildAccelerationStructuresIndirectKHR(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    infoCount,
			const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
			const VkDeviceAddress* pIndirectDeviceAddresses,
			const uint32_t* pIndirectStrides,
			const uint32_t* const* ppMaxPrimitiveCounts) const noexcept
		{
			m_table.vkCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
		}
		void  vkCmdBuildAccelerationStructuresKHR(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    infoCount,
			const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
			const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const noexcept
		{
			m_table.vkCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
		}
		void  vkCmdCopyAccelerationStructureKHR(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyAccelerationStructureInfoKHR* pInfo) const noexcept
		{
			m_table.vkCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
		}

		void  vkCmdCopyAccelerationStructureToMemoryKHR(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const noexcept
		{
			m_table.vkCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
		}
		void  vkCmdCopyMemoryToAccelerationStructureKHR(
			const VkCommandBuffer                             commandBuffer,
			const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const noexcept
		{
			m_table.vkCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
		}
		void  vkCmdWriteAccelerationStructuresPropertiesKHR(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    accelerationStructureCount,
			const VkAccelerationStructureKHR* pAccelerationStructures,
			const VkQueryType                                 queryType,
			const VkQueryPool                                 queryPool,
			const uint32_t                                    firstQuery) const noexcept
		{
			m_table.vkCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
		}
		VkResult  vkCopyAccelerationStructureKHR(
			const VkDeferredOperationKHR                      deferredOperation,
			const VkCopyAccelerationStructureInfoKHR* pInfo) const noexcept
		{
			return m_table.vkCopyAccelerationStructureKHR(m_handle, deferredOperation, pInfo);
		}

		VkResult  vkCopyAccelerationStructureToMemoryKHR(
			const VkDeferredOperationKHR                      deferredOperation,
			const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const noexcept
		{
			return m_table.vkCopyAccelerationStructureToMemoryKHR(m_handle, deferredOperation, pInfo);
		}
		VkResult  vkCopyMemoryToAccelerationStructureKHR(
			const VkDeferredOperationKHR                      deferredOperation,
			const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const noexcept
		{
			return m_table.vkCopyMemoryToAccelerationStructureKHR(m_handle, deferredOperation, pInfo);
		}
		VkResult  vkCreateAccelerationStructureKHR(
			const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
			VkAccelerationStructureKHR* pAccelerationStructure) const noexcept
		{
			return m_table.vkCreateAccelerationStructureKHR(m_handle, pCreateInfo, m_allocatorCallbacks, pAccelerationStructure);
		}
		void  vkDestroyAccelerationStructureKHR(
			const VkAccelerationStructureKHR                  accelerationStructure) const noexcept
		{
			m_table.vkDestroyAccelerationStructureKHR(m_handle, accelerationStructure, m_allocatorCallbacks);
		}
		void  vkGetAccelerationStructureBuildSizesKHR(
			const VkAccelerationStructureBuildTypeKHR         buildType,
			const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
			const uint32_t* pMaxPrimitiveCounts,
			VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) const noexcept
		{
			m_table.vkGetAccelerationStructureBuildSizesKHR(m_handle, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
		}
		VkDeviceAddress  vkGetAccelerationStructureDeviceAddressKHR(
			const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const noexcept
		{
			return m_table.vkGetAccelerationStructureDeviceAddressKHR(m_handle, pInfo);
		}
		void  vkGetDeviceAccelerationStructureCompatibilityKHR(
			const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
			VkAccelerationStructureCompatibilityKHR* pCompatibility) const noexcept
		{
			m_table.vkGetDeviceAccelerationStructureCompatibilityKHR(m_handle, pVersionInfo, pCompatibility);
		}
		VkResult  vkWriteAccelerationStructuresPropertiesKHR(
			const uint32_t                                    accelerationStructureCount,
			const VkAccelerationStructureKHR* pAccelerationStructures,
			const VkQueryType                                 queryType,
			const size_t                                      dataSize,
			void* pData,
			const size_t                                      stride) const noexcept
		{
			return m_table.vkWriteAccelerationStructuresPropertiesKHR(m_handle, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
		}
#endif /* defined(VK_KHR_acceleration_structure) */
#if defined(VK_KHR_deferred_host_operations)
		VkResult  vkCreateDeferredOperationKHR(
			VkDeferredOperationKHR* pDeferredOperation) const noexcept
		{
			return m_table.vkCreateDeferredOperationKHR(m_handle, m_allocatorCallbacks, pDeferredOperation);
		}
		VkResult  vkDeferredOperationJoinKHR(
			const VkDeferredOperationKHR                      operation) const noexcept
		{
			return m_table.vkDeferredOperationJoinKHR(m_handle, operation);
		}
		void  vkDestroyDeferredOperationKHR(
			const VkDeferredOperationKHR                      operation) const noexcept
		{
			m_table.vkDestroyDeferredOperationKHR(m_handle, operation, m_allocatorCallbacks);
		}

		uint32_t  vkGetDeferredOperationMaxConcurrencyKHR(
			const VkDeferredOperationKHR                      operation) const noexcept
		{
			return m_table.vkGetDeferredOperationMaxConcurrencyKHR(m_handle, operation);
		}
		VkResult  vkGetDeferredOperationResultKHR(
			const VkDeferredOperationKHR                      operation) const noexcept
		{
			return m_table.vkGetDeferredOperationResultKHR(m_handle, operation);
		}

#endif /* defined(VK_KHR_deferred_host_operations) */
#if defined(VK_KHR_ray_tracing_pipeline)
		void  vkCmdSetRayTracingPipelineStackSizeKHR(
			const VkCommandBuffer                             commandBuffer,
			const uint32_t                                    pipelineStackSize) const noexcept
		{
			m_table.vkCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
		}
		void  vkCmdTraceRaysIndirectKHR(
			const VkCommandBuffer                             commandBuffer,
			const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
			const VkDeviceAddress                             indirectDeviceAddress) const noexcept
		{
			m_table.vkCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
		}
		void  vkCmdTraceRaysKHR(
			const VkCommandBuffer                             commandBuffer,
			const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
			const uint32_t                                    width,
			const uint32_t                                    height,
			const uint32_t                                    depth) const noexcept
		{
			m_table.vkCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable,
			                          width, height, depth);
		}
		VkResult  vkCreateRayTracingPipelinesKHR(
			const VkDeferredOperationKHR                      deferredOperation,
			const VkPipelineCache                             pipelineCache,
			const uint32_t                                    createInfoCount,
			const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
			VkPipeline* pPipelines) const noexcept
		{
			return m_table.vkCreateRayTracingPipelinesKHR(m_handle, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
				m_allocatorCallbacks, pPipelines);
		}

		VkResult  vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(
			const VkPipeline                                  pipeline,
			const uint32_t                                    firstGroup,
			const uint32_t                                    groupCount,
			const size_t                                      dataSize,
			void* pData) const noexcept
		{
			return m_table.vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(m_handle, pipeline, firstGroup, groupCount, dataSize, pData);
		}
		VkResult  vkGetRayTracingShaderGroupHandlesKHR(
			const VkPipeline                                  pipeline,
			const uint32_t                                    firstGroup,
			const uint32_t                                    groupCount,
			const size_t                                      dataSize,
			void* pData) const noexcept
		{
			return m_table.vkGetRayTracingShaderGroupHandlesKHR(m_handle, pipeline, firstGroup, groupCount, dataSize, pData);
		}
		VkDeviceSize  vkGetRayTracingShaderGroupStackSizeKHR(
			const VkPipeline                                  pipeline,
			const uint32_t                                    group,
			const VkShaderGroupShaderKHR                      groupShader) const noexcept
		{
			return m_table.vkGetRayTracingShaderGroupStackSizeKHR(m_handle, pipeline, group, groupShader);
		}
#endif /* defined(VK_KHR_ray_tracing_pipeline) */
#if defined(VK_KHR_swapchain)
		VkResult  vkAcquireNextImageKHR(
			const VkSwapchainKHR                              swapchain,
			const uint64_t                                    timeout,
			const VkSemaphore                                 semaphore,
			const VkFence                                     fence,
			uint32_t* pImageIndex) const noexcept
		{
			return m_table.vkAcquireNextImageKHR(m_handle, swapchain, timeout, semaphore, fence, pImageIndex);
		}
		VkResult vkCreateSwapchainKHR(
			const VkSwapchainCreateInfoKHR* pCreateInfo,
			VkSwapchainKHR* pSwapchain) const noexcept
		{
			return m_table.vkCreateSwapchainKHR(m_handle, pCreateInfo, m_allocatorCallbacks, pSwapchain);
		}
		void  vkDestroySwapchainKHR(
			const VkSwapchainKHR                              swapchain) const noexcept
		{
			m_table.vkDestroySwapchainKHR(m_handle, swapchain, m_allocatorCallbacks);
		}
		VkResult  vkGetSwapchainImagesKHR(
			const VkSwapchainKHR                              swapchain,
			uint32_t* pSwapchainImageCount,
			VkImage* pSwapchainImages) const noexcept
		{
			return m_table.vkGetSwapchainImagesKHR(m_handle, swapchain, pSwapchainImageCount, pSwapchainImages);
		}
		VkResult  vkQueuePresentKHR(
			const VkQueue                                     queue,
			const VkPresentInfoKHR* pPresentInfo) const noexcept
		{
			return m_table.vkQueuePresentKHR(queue, pPresentInfo);
		}
#endif /* defined(VK_KHR_swapchain) */
		VkResult vmaCreateAllocator(
			VmaAllocatorCreateFlags flags,
			VkPhysicalDevice physicalDevice,
			VkInstance instance,
			uint32_t apiVersion,
			VmaAllocator* pAllocator)const noexcept;
	private:
		VkDevice m_handle{ VK_NULL_HANDLE };
		VolkDeviceTable m_table{};
		const VkAllocationCallbacks* m_allocatorCallbacks{ nullptr };
	};
}

nyan::vulkan::wrapper::LogicalDeviceWrapper::LogicalDeviceWrapper(const VkDevice device, const VkAllocationCallbacks* allocator) noexcept :
	m_handle(device),
	m_allocatorCallbacks(allocator)
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

nyan::vulkan::wrapper::LogicalDeviceWrapper::~LogicalDeviceWrapper() noexcept
{
	if (m_handle != VK_NULL_HANDLE) {
		this->vkDestroyDevice();
		m_handle = VK_NULL_HANDLE;
	}
}

nyan::vulkan::wrapper::LogicalDeviceWrapper::LogicalDeviceWrapper(LogicalDeviceWrapper&& other) noexcept :
	m_table(other.m_table),
	m_allocatorCallbacks(other.m_allocatorCallbacks)
{
	m_handle = other.m_handle;
	other.m_handle = VK_NULL_HANDLE;
}

nyan::vulkan::wrapper::LogicalDeviceWrapper& nyan::vulkan::wrapper::LogicalDeviceWrapper::operator=(LogicalDeviceWrapper&& other) noexcept
{
	if (this != &other)
	{
		m_handle = other.m_handle;
		other.m_handle = VK_NULL_HANDLE;
		m_allocatorCallbacks = other.m_allocatorCallbacks;
		m_table = other.m_table;
	}
	return *this;
}

VkResult nyan::vulkan::wrapper::LogicalDeviceWrapper::vmaCreateAllocator(VmaAllocatorCreateFlags flags,
	VkPhysicalDevice physicalDevice, VkInstance instance,
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
		.pAllocationCallbacks = m_allocatorCallbacks,
		.pVulkanFunctions = &vulkanFunctions,
		.instance = instance,
		.vulkanApiVersion = apiVersion,
	};
	return ::vmaCreateAllocator(&allocatorInfo, pAllocator);
}
