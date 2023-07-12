#pragma once
#include "volk.h"
#include "vk_mem_alloc.h"

namespace vulkan
{
	class LogicalDeviceWrapper
	{
	public:
		LogicalDeviceWrapper(VkDevice device, const VkAllocationCallbacks* allocator);
		~LogicalDeviceWrapper();
#if defined(VK_VERSION_1_0)
		VkResult vkAllocateCommandBuffers(
			const VkCommandBufferAllocateInfo*			pAllocateInfo,
			VkCommandBuffer*							pCommandBuffers) const noexcept;
		VkResult vkAllocateDescriptorSets(
			const VkDescriptorSetAllocateInfo*			pAllocateInfo,
			VkDescriptorSet*							pDescriptorSets) const noexcept;
		VkResult vkAllocateMemory(
			const VkMemoryAllocateInfo*					pAllocateInfo,
			const VkAllocationCallbacks*				pAllocator,
			VkDeviceMemory*								pMemory) const noexcept;
		VkResult vkBindBufferMemory(
			VkBuffer									buffer,
			VkDeviceMemory								memory,
			VkDeviceSize								memoryOffset) const noexcept;
		VkResult vkBindImageMemory(
			VkImage										image,
			VkDeviceMemory								memory,
			VkDeviceSize								memoryOffset) const noexcept;
		void  vkCmdBeginQuery(
			VkCommandBuffer                             commandBuffer,
			VkQueryPool                                 queryPool,
			uint32_t                                    query,
			VkQueryControlFlags                         flags) const noexcept;
		void vkCmdBeginRenderPass(
			VkCommandBuffer                             commandBuffer,
			const VkRenderPassBeginInfo* pRenderPassBegin,
			VkSubpassContents                           contents) const noexcept;
		void vkCmdBindDescriptorSets(
			VkCommandBuffer                             commandBuffer,
			VkPipelineBindPoint                         pipelineBindPoint,
			VkPipelineLayout                            layout,
			uint32_t                                    firstSet,
			uint32_t                                    descriptorSetCount,
			const VkDescriptorSet* pDescriptorSets,
			uint32_t                                    dynamicOffsetCount,
			const uint32_t* pDynamicOffsets) const noexcept;
		void vkCmdBindIndexBuffer(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    buffer,
			VkDeviceSize                                offset,
			VkIndexType                                 indexType) const noexcept;
		void vkCmdBindPipeline(
			VkCommandBuffer                             commandBuffer,
			VkPipelineBindPoint                         pipelineBindPoint,
			VkPipeline                                  pipeline) const noexcept;
		void vkCmdBindVertexBuffers(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    firstBinding,
			uint32_t                                    bindingCount,
			const VkBuffer* pBuffers,
			const VkDeviceSize* pOffsets) const noexcept;
		void vkCmdBlitImage(
			VkCommandBuffer                             commandBuffer,
			VkImage                                     srcImage,
			VkImageLayout                               srcImageLayout,
			VkImage                                     dstImage,
			VkImageLayout                               dstImageLayout,
			uint32_t                                    regionCount,
			const VkImageBlit* pRegions,
			VkFilter                                    filter) const noexcept;
		void vkCmdClearAttachments(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    attachmentCount,
			const VkClearAttachment* pAttachments,
			uint32_t                                    rectCount,
			const VkClearRect* pRects) const noexcept;
		void vkCmdClearColorImage(
			VkCommandBuffer                             commandBuffer,
			VkImage                                     image,
			VkImageLayout                               imageLayout,
			const VkClearColorValue* pColor,
			uint32_t                                    rangeCount,
			const VkImageSubresourceRange* pRanges) const noexcept;
		void vkCmdClearDepthStencilImage(
			VkCommandBuffer                             commandBuffer,
			VkImage                                     image,
			VkImageLayout                               imageLayout,
			const VkClearDepthStencilValue* pDepthStencil,
			uint32_t                                    rangeCount,
			const VkImageSubresourceRange* pRanges) const noexcept;
		void vkCmdCopyBuffer(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    srcBuffer,
			VkBuffer                                    dstBuffer,
			uint32_t                                    regionCount,
			const VkBufferCopy* pRegions) const noexcept;
		void vkCmdCopyBufferToImage(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    srcBuffer,
			VkImage                                     dstImage,
			VkImageLayout                               dstImageLayout,
			uint32_t                                    regionCount,
			const VkBufferImageCopy* pRegions) const noexcept;
		void vkCmdCopyImage(
			VkCommandBuffer                             commandBuffer,
			VkImage                                     srcImage,
			VkImageLayout                               srcImageLayout,
			VkImage                                     dstImage,
			VkImageLayout                               dstImageLayout,
			uint32_t                                    regionCount,
			const VkImageCopy* pRegions) const noexcept;
		void vkCmdCopyImageToBuffer(
			VkCommandBuffer                             commandBuffer,
			VkImage                                     srcImage,
			VkImageLayout                               srcImageLayout,
			VkBuffer                                    dstBuffer,
			uint32_t                                    regionCount,
			const VkBufferImageCopy* pRegions) const noexcept;
		void vkCmdCopyQueryPoolResults(
			VkCommandBuffer                             commandBuffer,
			VkQueryPool                                 queryPool,
			uint32_t                                    firstQuery,
			uint32_t                                    queryCount,
			VkBuffer                                    dstBuffer,
			VkDeviceSize                                dstOffset,
			VkDeviceSize                                stride,
			VkQueryResultFlags                          flags) const noexcept;
		void vkCmdDispatch(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    groupCountX,
			uint32_t                                    groupCountY,
			uint32_t                                    groupCountZ) const noexcept;
		void vkCmdDispatchIndirect(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    buffer,
			VkDeviceSize                                offset) const noexcept;
		void vkCmdDraw(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    vertexCount,
			uint32_t                                    instanceCount,
			uint32_t                                    firstVertex,
			uint32_t                                    firstInstance) const noexcept;
		void vkCmdDrawIndexed(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    indexCount,
			uint32_t                                    instanceCount,
			uint32_t                                    firstIndex,
			int32_t                                     vertexOffset,
			uint32_t                                    firstInstance) const noexcept;
		void vkCmdDrawIndexedIndirect(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    buffer,
			VkDeviceSize                                offset,
			uint32_t                                    drawCount,
			uint32_t                                    stride) const noexcept;
		void vkCmdDrawIndirect(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    buffer,
			VkDeviceSize                                offset,
			uint32_t                                    drawCount,
			uint32_t                                    stride) const noexcept;
		void vkCmdEndQuery(
			VkCommandBuffer                             commandBuffer,
			VkQueryPool                                 queryPool,
			uint32_t                                    query) const noexcept;
		void vkCmdEndRenderPass(
			VkCommandBuffer                             commandBuffer) const noexcept;
		void vkCmdExecuteCommands(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    commandBufferCount,
			const VkCommandBuffer* pCommandBuffers) const noexcept;
		void vkCmdFillBuffer(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    dstBuffer,
			VkDeviceSize                                dstOffset,
			VkDeviceSize                                size,
			uint32_t                                    data) const noexcept;
		void vkCmdNextSubpass(
			VkCommandBuffer                             commandBuffer,
			VkSubpassContents                           contents) const noexcept;
		void vkCmdPipelineBarrier(
			VkCommandBuffer                             commandBuffer,
			VkPipelineStageFlags                        srcStageMask,
			VkPipelineStageFlags                        dstStageMask,
			VkDependencyFlags                           dependencyFlags,
			uint32_t                                    memoryBarrierCount,
			const VkMemoryBarrier*						pMemoryBarriers,
			uint32_t                                    bufferMemoryBarrierCount,
			const VkBufferMemoryBarrier*				pBufferMemoryBarriers,
			uint32_t                                    imageMemoryBarrierCount,
			const VkImageMemoryBarrier*					pImageMemoryBarriers) const noexcept;
		void vkCmdPushConstants(
			VkCommandBuffer                             commandBuffer,
			VkPipelineLayout                            layout,
			VkShaderStageFlags                          stageFlags,
			uint32_t                                    offset,
			uint32_t                                    size,
			const void* pValues) const noexcept;
		void vkCmdResetEvent(
			VkCommandBuffer                             commandBuffer,
			VkEvent                                     event,
			VkPipelineStageFlags                        stageMask) const noexcept;
		void vkCmdResetQueryPool(
			VkCommandBuffer                             commandBuffer,
			VkQueryPool                                 queryPool,
			uint32_t                                    firstQuery,
			uint32_t                                    queryCount) const noexcept;
		void vkCmdResolveImage(
			VkCommandBuffer                             commandBuffer,
			VkImage                                     srcImage,
			VkImageLayout                               srcImageLayout,
			VkImage                                     dstImage,
			VkImageLayout                               dstImageLayout,
			uint32_t                                    regionCount,
			const VkImageResolve* pRegions) const noexcept;
		void vkCmdSetBlendConstants(
			VkCommandBuffer                             commandBuffer,
			const float                                 blendConstants[4]) const noexcept;
		void vkCmdSetDepthBias(
			VkCommandBuffer                             commandBuffer,
			float                                       depthBiasConstantFactor,
			float                                       depthBiasClamp,
			float                                       depthBiasSlopeFactor) const noexcept;
		void vkCmdSetDepthBounds(
			VkCommandBuffer                             commandBuffer,
			float                                       minDepthBounds,
			float                                       maxDepthBounds) const noexcept;
		void vkCmdSetEvent(
			VkCommandBuffer                             commandBuffer,
			VkEvent                                     event,
			VkPipelineStageFlags                        stageMask) const noexcept;
		void vkCmdSetLineWidth(
			VkCommandBuffer                             commandBuffer,
			float                                       lineWidth) const noexcept;
		void vkCmdSetScissor(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    firstScissor,
			uint32_t                                    scissorCount,
			const VkRect2D* pScissors) const noexcept;
		void vkCmdSetStencilCompareMask(
			VkCommandBuffer                             commandBuffer,
			VkStencilFaceFlags                          faceMask,
			uint32_t                                    compareMask) const noexcept;
		void vkCmdSetStencilReference(
			VkCommandBuffer                             commandBuffer,
			VkStencilFaceFlags                          faceMask,
			uint32_t                                    reference) const noexcept;
		void vkCmdSetStencilWriteMask(
			VkCommandBuffer                             commandBuffer,
			VkStencilFaceFlags                          faceMask,
			uint32_t                                    writeMask) const noexcept;
		void vkCmdSetViewport(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    firstViewport,
			uint32_t                                    viewportCount,
			const VkViewport* pViewports) const noexcept;
		void vkCmdUpdateBuffer(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    dstBuffer,
			VkDeviceSize                                dstOffset,
			VkDeviceSize                                dataSize,
			const void* pData) const noexcept;
		void vkCmdWaitEvents(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    eventCount,
			const VkEvent* pEvents,
			VkPipelineStageFlags                        srcStageMask,
			VkPipelineStageFlags                        dstStageMask,
			uint32_t                                    memoryBarrierCount,
			const VkMemoryBarrier* pMemoryBarriers,
			uint32_t                                    bufferMemoryBarrierCount,
			const VkBufferMemoryBarrier* pBufferMemoryBarriers,
			uint32_t                                    imageMemoryBarrierCount,
			const VkImageMemoryBarrier* pImageMemoryBarriers) const noexcept;
		void vkCmdWriteTimestamp(
			VkCommandBuffer                             commandBuffer,
			VkPipelineStageFlagBits                     pipelineStage,
			VkQueryPool                                 queryPool,
			uint32_t                                    query) const noexcept;
		VkResult vkCreateBuffer(
			const VkBufferCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkBuffer* pBuffer) const noexcept;
		VkResult vkCreateBufferView(
			const VkBufferViewCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkBufferView* pView) const noexcept;
		VkResult vkCreateCommandPool(
			const VkCommandPoolCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkCommandPool* pCommandPool) const noexcept;
		VkResult vkCreateComputePipelines(
			VkPipelineCache                             pipelineCache,
			uint32_t                                    createInfoCount,
			const VkComputePipelineCreateInfo* pCreateInfos,
			const VkAllocationCallbacks* pAllocator,
			VkPipeline* pPipelines) const noexcept;
		VkResult vkCreateDescriptorPool(
			const VkDescriptorPoolCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDescriptorPool* pDescriptorPool) const noexcept;
		VkResult vkCreateDescriptorSetLayout(
			const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDescriptorSetLayout* pSetLayout) const noexcept;
		VkResult vkCreateEvent(
			const VkEventCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkEvent* pEvent) const noexcept;
		VkResult vkCreateFence(
			const VkFenceCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkFence* pFence) const noexcept;
		VkResult vkCreateFramebuffer(
			const VkFramebufferCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkFramebuffer* pFramebuffer) const noexcept;
		VkResult vkCreateGraphicsPipelines(
			VkPipelineCache                             pipelineCache,
			uint32_t                                    createInfoCount,
			const VkGraphicsPipelineCreateInfo* pCreateInfos,
			const VkAllocationCallbacks* pAllocator,
			VkPipeline* pPipelines) const noexcept;
		VkResult vkCreateImage(
			const VkImageCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkImage* pImage) const noexcept;
		VkResult vkCreateImageView(
			const VkImageViewCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkImageView* pView) const noexcept;
		VkResult vkCreatePipelineCache(
			const VkPipelineCacheCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkPipelineCache* pPipelineCache) const noexcept;
		VkResult vkCreatePipelineLayout(
			const VkPipelineLayoutCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkPipelineLayout* pPipelineLayout) const noexcept;
		VkResult vkCreateQueryPool(
			const VkQueryPoolCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkQueryPool* pQueryPool) const noexcept;
		VkResult vkCreateRenderPass(
			const VkRenderPassCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkRenderPass* pRenderPass) const noexcept;
		VkResult vkCreateSampler(
			const VkSamplerCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkSampler* pSampler) const noexcept;
		VkResult vkCreateSemaphore(
			const VkSemaphoreCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkSemaphore* pSemaphore) const noexcept;
		VkResult vkCreateShaderModule(
			const VkShaderModuleCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkShaderModule* pShaderModule) const noexcept;
		void vkDestroyBuffer(
			VkBuffer                                    buffer,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyBufferView(
			VkBufferView                                bufferView,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyCommandPool(
			VkCommandPool                               commandPool,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyDescriptorPool(
			VkDescriptorPool                            descriptorPool,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyDescriptorSetLayout(
			VkDescriptorSetLayout                       descriptorSetLayout,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyDevice(
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyEvent(
			VkEvent                                     event,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyFence(
			VkFence                                     fence,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyFramebuffer(
			VkFramebuffer                               framebuffer,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyImage(
			VkImage                                     image,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyImageView(
			VkImageView                                 imageView,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyPipeline(
			VkPipeline                                  pipeline,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyPipelineCache(
			VkPipelineCache                             pipelineCache,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyPipelineLayout(
			VkPipelineLayout                            pipelineLayout,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyQueryPool(
			VkQueryPool                                 queryPool,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyRenderPass(
			VkRenderPass                                renderPass,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroySampler(
			VkSampler                                   sampler,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroySemaphore(
			VkSemaphore                                 semaphore,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroyShaderModule(
			VkShaderModule                              shaderModule,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		VkResult vkDeviceWaitIdle() const noexcept;
		VkResult vkEndCommandBuffer(
			VkCommandBuffer                             commandBuffer) const noexcept;
		VkResult vkFlushMappedMemoryRanges(
			uint32_t                                    memoryRangeCount,
			const VkMappedMemoryRange* pMemoryRanges) const noexcept;
		void vkFreeCommandBuffers(
			VkCommandPool                               commandPool,
			uint32_t                                    commandBufferCount,
			const VkCommandBuffer* pCommandBuffers) const noexcept;
		VkResult vkFreeDescriptorSets(
			VkDescriptorPool                            descriptorPool,
			uint32_t                                    descriptorSetCount,
			const VkDescriptorSet* pDescriptorSets) const noexcept;
		void vkFreeMemory(
			VkDeviceMemory                              memory,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkGetBufferMemoryRequirements(
			VkBuffer                                    buffer,
			VkMemoryRequirements* pMemoryRequirements) const noexcept;
		void vkGetDeviceMemoryCommitment(
			VkDeviceMemory                              memory,
			VkDeviceSize* pCommittedMemoryInBytes) const noexcept;
		void vkGetDeviceQueue(
			uint32_t                                    queueFamilyIndex,
			uint32_t                                    queueIndex,
			VkQueue* pQueue) const noexcept;
		VkResult vkGetEventStatus(
			VkEvent                                     event) const noexcept;
		VkResult vkGetFenceStatus(
			VkFence                                     fence) const noexcept;
		void  vkGetImageMemoryRequirements(
			VkImage                                     image,
			VkMemoryRequirements* pMemoryRequirements) const noexcept;
		void vkGetImageSparseMemoryRequirements(
			VkImage                                     image,
			uint32_t* pSparseMemoryRequirementCount,
			VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const noexcept;
		void  vkGetImageSubresourceLayout(
			VkImage                                     image,
			const VkImageSubresource* pSubresource,
			VkSubresourceLayout* pLayout) const noexcept;
		VkResult vkGetPipelineCacheData(
			VkPipelineCache                             pipelineCache,
			size_t* pDataSize,
			void* pData) const noexcept;
		VkResult vkGetQueryPoolResults(
			VkQueryPool                                 queryPool,
			uint32_t                                    firstQuery,
			uint32_t                                    queryCount,
			size_t                                      dataSize,
			void* pData,
			VkDeviceSize                                stride,
			VkQueryResultFlags                          flags) const noexcept;
		void vkGetRenderAreaGranularity(
			VkRenderPass                                renderPass,
			VkExtent2D* pGranularity) const noexcept;
		VkResult vkInvalidateMappedMemoryRanges(
			uint32_t                                    memoryRangeCount,
			const VkMappedMemoryRange* pMemoryRanges) const noexcept;
		VkResult vkMapMemory(
			VkDeviceMemory                              memory,
			VkDeviceSize                                offset,
			VkDeviceSize                                size,
			VkMemoryMapFlags                            flags,
			void** ppData) const noexcept;
		VkResult vkMergePipelineCaches(
			VkPipelineCache                             dstCache,
			uint32_t                                    srcCacheCount,
			const VkPipelineCache* pSrcCaches) const noexcept;
		VkResult vkQueueBindSparse(
			VkQueue                                     queue,
			uint32_t                                    bindInfoCount,
			const VkBindSparseInfo* pBindInfo,
			VkFence                                     fence) const noexcept;
		VkResult vkQueueSubmit(
			VkQueue                                     queue,
			uint32_t                                    submitCount,
			const VkSubmitInfo* pSubmits,
			VkFence                                     fence) const noexcept;
		VkResult vkQueueWaitIdle(
			VkQueue                                     queue) const noexcept;
		VkResult vkResetCommandBuffer(
			VkCommandBuffer                             commandBuffer,
			VkCommandBufferResetFlags                   flags) const noexcept;
		VkResult vkResetCommandPool(
			VkCommandPool                               commandPool,
			VkCommandPoolResetFlags                     flags) const noexcept;
		VkResult vkResetDescriptorPool(
			VkDescriptorPool                            descriptorPool,
			VkDescriptorPoolResetFlags                  flags) const noexcept;
		VkResult vkResetEvent(
			VkEvent                                     event) const noexcept;
		VkResult vkResetFences(
			uint32_t                                    fenceCount,
			const VkFence* pFences) const noexcept;
		VkResult vkSetEvent(
			VkEvent                                     event) const noexcept;
		void  vkUnmapMemory(
			VkDeviceMemory                              memory) const noexcept;
		void  vkUpdateDescriptorSets(
			uint32_t                                    descriptorWriteCount,
			const VkWriteDescriptorSet* pDescriptorWrites,
			uint32_t                                    descriptorCopyCount,
			const VkCopyDescriptorSet* pDescriptorCopies) const noexcept;
		VkResult vkWaitForFences(
			uint32_t                                    fenceCount,
			const VkFence* pFences,
			VkBool32                                    waitAll,
			uint64_t                                    timeout) const noexcept;
#endif
#if defined(VK_VERSION_1_1)
		VkResult vkBindBufferMemory2(
			uint32_t                                    bindInfoCount,
			const VkBindBufferMemoryInfo* pBindInfos) const noexcept;
		VkResult vkBindImageMemory2(
			uint32_t                                    bindInfoCount,
			const VkBindImageMemoryInfo* pBindInfos) const noexcept;
		void vkCmdDispatchBase(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    baseGroupX,
			uint32_t                                    baseGroupY,
			uint32_t                                    baseGroupZ,
			uint32_t                                    groupCountX,
			uint32_t                                    groupCountY,
			uint32_t                                    groupCountZ) const noexcept;
		void vkCmdSetDeviceMask(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    deviceMask) const noexcept;
		VkResult vkCreateDescriptorUpdateTemplate(
			const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const noexcept;
		VkResult vkCreateSamplerYcbcrConversion(
			const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkSamplerYcbcrConversion* pYcbcrConversion) const noexcept;
		void  vkDestroyDescriptorUpdateTemplate(
			VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkDestroySamplerYcbcrConversion(
			VkSamplerYcbcrConversion                    ycbcrConversion,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void vkGetBufferMemoryRequirements2(
			const VkBufferMemoryRequirementsInfo2* pInfo,
			VkMemoryRequirements2* pMemoryRequirements) const noexcept;
		void  vkGetDescriptorSetLayoutSupport(
			const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
			VkDescriptorSetLayoutSupport* pSupport) const noexcept;
		void  vkGetDeviceGroupPeerMemoryFeatures(
			uint32_t                                    heapIndex,
			uint32_t                                    localDeviceIndex,
			uint32_t                                    remoteDeviceIndex,
			VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const noexcept;
		void  vkGetDeviceQueue2(
			const VkDeviceQueueInfo2* pQueueInfo,
			VkQueue* pQueue) const noexcept;
		void vkGetImageMemoryRequirements2(
			const VkImageMemoryRequirementsInfo2* pInfo,
			VkMemoryRequirements2* pMemoryRequirements) const noexcept;
		void vkGetImageSparseMemoryRequirements2(
			const VkImageSparseMemoryRequirementsInfo2* pInfo,
			uint32_t* pSparseMemoryRequirementCount,
			VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const noexcept;
		void  vkTrimCommandPool(
			VkCommandPool                               commandPool,
			VkCommandPoolTrimFlags                      flags) const noexcept;
		void  vkUpdateDescriptorSetWithTemplate(
			VkDescriptorSet                             descriptorSet,
			VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
			const void* pData) const noexcept;
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_2)
		void  vkCmdBeginRenderPass2(
			VkCommandBuffer                             commandBuffer,
			const VkRenderPassBeginInfo* pRenderPassBegin,
			const VkSubpassBeginInfo* pSubpassBeginInfo) const noexcept;
		void  vkCmdDrawIndexedIndirectCount(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    buffer,
			VkDeviceSize                                offset,
			VkBuffer                                    countBuffer,
			VkDeviceSize                                countBufferOffset,
			uint32_t                                    maxDrawCount,
			uint32_t                                    stride) const noexcept;
		void  vkCmdDrawIndirectCount(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    buffer,
			VkDeviceSize                                offset,
			VkBuffer                                    countBuffer,
			VkDeviceSize                                countBufferOffset,
			uint32_t                                    maxDrawCount,
			uint32_t                                    stride) const noexcept;
		void  vkCmdEndRenderPass2(
			VkCommandBuffer                             commandBuffer,
			const VkSubpassEndInfo* pSubpassEndInfo) const noexcept;
		void  vkCmdNextSubpass2(
			VkCommandBuffer                             commandBuffer,
			const VkSubpassBeginInfo* pSubpassBeginInfo,
			const VkSubpassEndInfo* pSubpassEndInfo) const noexcept;
		VkResult  vkCreateRenderPass2(
			const VkRenderPassCreateInfo2* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkRenderPass* pRenderPass) const noexcept;
		VkDeviceAddress  vkGetBufferDeviceAddress(
			const VkBufferDeviceAddressInfo* pInfo) const noexcept;
		uint64_t  vkGetBufferOpaqueCaptureAddress(
			const VkBufferDeviceAddressInfo* pInfo) const noexcept;
		uint64_t  vkGetDeviceMemoryOpaqueCaptureAddress(
			const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const noexcept;
		VkResult  vkGetSemaphoreCounterValue(
			VkSemaphore                                 semaphore,
			uint64_t* pValue) const noexcept;
		void  vkResetQueryPool(
			VkQueryPool                                 queryPool,
			uint32_t                                    firstQuery,
			uint32_t                                    queryCount) const noexcept;
		VkResult  vkSignalSemaphore(
			const VkSemaphoreSignalInfo* pSignalInfo) const noexcept;
		VkResult  vkWaitSemaphores(
			const VkSemaphoreWaitInfo* pWaitInfo,
			uint64_t                                    timeout) const noexcept;
#endif /* defined(VK_VERSION_1_2) */
#if defined(VK_VERSION_1_3)
		void  vkCmdBeginRendering(
			VkCommandBuffer                             commandBuffer,
			const VkRenderingInfo* pRenderingInfo) const noexcept;
		void  vkCmdBindVertexBuffers2(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    firstBinding,
			uint32_t                                    bindingCount,
			const VkBuffer* pBuffers,
			const VkDeviceSize* pOffsets,
			const VkDeviceSize* pSizes,
			const VkDeviceSize* pStrides) const noexcept;
		void  vkCmdBlitImage2(
			VkCommandBuffer                             commandBuffer,
			const VkBlitImageInfo2* pBlitImageInfo) const noexcept;
		void  vkCmdCopyBuffer2(
			VkCommandBuffer                             commandBuffer,
			const VkCopyBufferInfo2* pCopyBufferInfo) const noexcept;
		void  vkCmdCopyBufferToImage2(
			VkCommandBuffer                             commandBuffer,
			const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const noexcept;
		void  vkCmdCopyImage2(
			VkCommandBuffer                             commandBuffer,
			const VkCopyImageInfo2* pCopyImageInfo) const noexcept;
		void  vkCmdCopyImageToBuffer2(
			VkCommandBuffer                             commandBuffer,
			const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const noexcept;
		void  vkCmdEndRendering(
			VkCommandBuffer                             commandBuffer) const noexcept;
		void  vkCmdPipelineBarrier2(
			VkCommandBuffer                             commandBuffer,
			const VkDependencyInfo* pDependencyInfo) const noexcept;
		void  vkCmdResetEvent2(
			VkCommandBuffer                             commandBuffer,
			VkEvent                                     event,
			VkPipelineStageFlags2                       stageMask) const noexcept;
		void  vkCmdResolveImage2(
			VkCommandBuffer                             commandBuffer,
			const VkResolveImageInfo2* pResolveImageInfo) const noexcept;
		void  vkCmdSetCullMode(
			VkCommandBuffer                             commandBuffer,
			VkCullModeFlags                             cullMode) const noexcept;
		void  vkCmdSetDepthBiasEnable(
			VkCommandBuffer                             commandBuffer,
			VkBool32                                    depthBiasEnable) const noexcept;
		void  vkCmdSetDepthBoundsTestEnable(
			VkCommandBuffer                             commandBuffer,
			VkBool32                                    depthBoundsTestEnable) const noexcept;
		void  vkCmdSetDepthCompareOp(
			VkCommandBuffer                             commandBuffer,
			VkCompareOp                                 depthCompareOp) const noexcept;
		void  vkCmdSetDepthTestEnable(
			VkCommandBuffer                             commandBuffer,
			VkBool32                                    depthTestEnable) const noexcept;
		void  vkCmdSetDepthWriteEnable(
			VkCommandBuffer                             commandBuffer,
			VkBool32                                    depthWriteEnable) const noexcept;
		void  vkCmdSetEvent2(
			VkCommandBuffer                             commandBuffer,
			VkEvent                                     event,
			const VkDependencyInfo* pDependencyInfo) const noexcept;
		void  vkCmdSetFrontFace(
			VkCommandBuffer                             commandBuffer,
			VkFrontFace                                 frontFace) const noexcept;
		void  vkCmdSetPrimitiveRestartEnable(
			VkCommandBuffer                             commandBuffer,
			VkBool32                                    primitiveRestartEnable) const noexcept;
		void  vkCmdSetPrimitiveTopology(
			VkCommandBuffer                             commandBuffer,
			VkPrimitiveTopology                         primitiveTopology) const noexcept;
		void  vkCmdSetRasterizerDiscardEnable(
			VkCommandBuffer                             commandBuffer,
			VkBool32                                    rasterizerDiscardEnable) const noexcept;
		void  vkCmdSetScissorWithCount(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    scissorCount,
			const VkRect2D* pScissors) const noexcept;
		void  vkCmdSetStencilOp(
			VkCommandBuffer                             commandBuffer,
			VkStencilFaceFlags                          faceMask,
			VkStencilOp                                 failOp,
			VkStencilOp                                 passOp,
			VkStencilOp                                 depthFailOp,
			VkCompareOp                                 compareOp) const noexcept;
		void  vkCmdSetStencilTestEnable(
			VkCommandBuffer                             commandBuffer,
			VkBool32                                    stencilTestEnable) const noexcept;
		void  vkCmdSetViewportWithCount(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    viewportCount,
			const VkViewport* pViewports) const noexcept;
		void  vkCmdWaitEvents2(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    eventCount,
			const VkEvent* pEvents,
			const VkDependencyInfo* pDependencyInfos) const noexcept;
		void  vkCmdWriteTimestamp2(
			VkCommandBuffer                             commandBuffer,
			VkPipelineStageFlags2                       stage,
			VkQueryPool                                 queryPool,
			uint32_t                                    query) const noexcept;
		VkResult   vkCreatePrivateDataSlot(
			const VkPrivateDataSlotCreateInfo* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkPrivateDataSlot* pPrivateDataSlot) const noexcept;
		void  vkDestroyPrivateDataSlot(
			VkPrivateDataSlot                           privateDataSlot,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void  vkGetDeviceBufferMemoryRequirements(
			const VkDeviceBufferMemoryRequirements* pInfo,
			VkMemoryRequirements2* pMemoryRequirements) const noexcept;
		void  vkGetDeviceImageMemoryRequirements(
			const VkDeviceImageMemoryRequirements* pInfo,
			VkMemoryRequirements2* pMemoryRequirements) const noexcept;
		void  vkGetDeviceImageSparseMemoryRequirements(
			const VkDeviceImageMemoryRequirements* pInfo,
			uint32_t* pSparseMemoryRequirementCount,
			VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const noexcept;
		void  vkGetPrivateData(
			VkObjectType                                objectType,
			uint64_t                                    objectHandle,
			VkPrivateDataSlot                           privateDataSlot,
			uint64_t* pData) const noexcept;
		VkResult   vkQueueSubmit2(
			VkQueue                                     queue,
			uint32_t                                    submitCount,
			const VkSubmitInfo2* pSubmits,
			VkFence                                     fence) const noexcept;
		VkResult vkSetPrivateData(
			VkObjectType                                objectType,
			uint64_t                                    objectHandle,
			VkPrivateDataSlot                           privateDataSlot,
			uint64_t                                    data) const noexcept;
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_EXT_full_screen_exclusive)
		VkResult vkAcquireFullScreenExclusiveModeEXT(
			VkSwapchainKHR                              swapchain) const noexcept;
		VkResult vkReleaseFullScreenExclusiveModeEXT(
			VkSwapchainKHR                              swapchain) const noexcept;
#endif /* defined(VK_EXT_full_screen_exclusive) */
#if defined(VK_EXT_mesh_shader)
		void  vkCmdDrawMeshTasksEXT(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    groupCountX,
			uint32_t                                    groupCountY,
			uint32_t                                    groupCountZ) const noexcept;
		void  vkCmdDrawMeshTasksIndirectCountEXT(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    buffer,
			VkDeviceSize                                offset,
			VkBuffer                                    countBuffer,
			VkDeviceSize                                countBufferOffset,
			uint32_t                                    maxDrawCount,
			uint32_t                                    stride) const noexcept;
		void  vkCmdDrawMeshTasksIndirectEXT(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    buffer,
			VkDeviceSize                                offset,
			uint32_t                                    drawCount,
			uint32_t                                    stride) const noexcept;
#endif /* defined(VK_EXT_mesh_shader) */

#if defined(VK_EXT_opacity_micromap)
		VkResult  vkBuildMicromapsEXT(
			VkDeferredOperationKHR                      deferredOperation,
			uint32_t                                    infoCount,
			const VkMicromapBuildInfoEXT* pInfos) const noexcept;
		void  vkCmdBuildMicromapsEXT(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    infoCount,
			const VkMicromapBuildInfoEXT* pInfos) const noexcept;
		void  vkCmdCopyMemoryToMicromapEXT(
			VkCommandBuffer                             commandBuffer,
			const VkCopyMemoryToMicromapInfoEXT* pInfo) const noexcept;
		void  vkCmdCopyMicromapEXT(
			VkCommandBuffer                             commandBuffer,
			const VkCopyMicromapInfoEXT* pInfo) const noexcept;
		void  vkCmdCopyMicromapToMemoryEXT(
			VkCommandBuffer                             commandBuffer,
			const VkCopyMicromapToMemoryInfoEXT* pInfo) const noexcept;
		void  vkCmdWriteMicromapsPropertiesEXT(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    micromapCount,
			const VkMicromapEXT* pMicromaps,
			VkQueryType                                 queryType,
			VkQueryPool                                 queryPool,
			uint32_t                                    firstQuery) const noexcept;
		VkResult  vkCopyMemoryToMicromapEXT(
			VkDeferredOperationKHR                      deferredOperation,
			const VkCopyMemoryToMicromapInfoEXT* pInfo) const noexcept;
		VkResult  vkCopyMicromapEXT(
			VkDeferredOperationKHR                      deferredOperation,
			const VkCopyMicromapInfoEXT* pInfo) const noexcept;
		VkResult  vkCopyMicromapToMemoryEXT(
			VkDeferredOperationKHR                      deferredOperation,
			const VkCopyMicromapToMemoryInfoEXT* pInfo) const noexcept;
		VkResult  vkCreateMicromapEXT(
			const VkMicromapCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkMicromapEXT* pMicromap) const noexcept;
		void  vkDestroyMicromapEXT(
			VkMicromapEXT                               micromap,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void  vkGetDeviceMicromapCompatibilityEXT(
			const VkMicromapVersionInfoEXT* pVersionInfo,
			VkAccelerationStructureCompatibilityKHR* pCompatibility) const noexcept;
		void  vkGetMicromapBuildSizesEXT(
			VkAccelerationStructureBuildTypeKHR         buildType,
			const VkMicromapBuildInfoEXT* pBuildInfo,
			VkMicromapBuildSizesInfoEXT* pSizeInfo) const noexcept;
		VkResult  vkWriteMicromapsPropertiesEXT(
			uint32_t                                    micromapCount,
			const VkMicromapEXT* pMicromaps,
			VkQueryType                                 queryType,
			size_t                                      dataSize,
			void* pData,
			size_t                                      stride) const noexcept;
#endif /* defined(VK_EXT_opacity_micromap) */
#if defined(VK_EXT_swapchain_maintenance1)
		VkResult  vkReleaseSwapchainImagesEXT(
			const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo) const noexcept;
#endif /* defined(VK_EXT_swapchain_maintenance1) */
#if defined(VK_KHR_acceleration_structure)
		VkResult  vkBuildAccelerationStructuresKHR(
			VkDeferredOperationKHR                      deferredOperation,
			uint32_t                                    infoCount,
			const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
			const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const noexcept;
		void  vkCmdBuildAccelerationStructuresIndirectKHR(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    infoCount,
			const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
			const VkDeviceAddress* pIndirectDeviceAddresses,
			const uint32_t* pIndirectStrides,
			const uint32_t* const* ppMaxPrimitiveCounts) const noexcept;
		void  vkCmdBuildAccelerationStructuresKHR(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    infoCount,
			const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
			const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const noexcept;
		void  vkCmdCopyAccelerationStructureKHR(
			VkCommandBuffer                             commandBuffer,
			const VkCopyAccelerationStructureInfoKHR* pInfo) const noexcept;
		void  vkCmdCopyAccelerationStructureToMemoryKHR(
			VkCommandBuffer                             commandBuffer,
			const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const noexcept;
		void  vkCmdCopyMemoryToAccelerationStructureKHR(
			VkCommandBuffer                             commandBuffer,
			const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const noexcept;
		void  vkCmdWriteAccelerationStructuresPropertiesKHR(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    accelerationStructureCount,
			const VkAccelerationStructureKHR* pAccelerationStructures,
			VkQueryType                                 queryType,
			VkQueryPool                                 queryPool,
			uint32_t                                    firstQuery) const noexcept;
		VkResult  vkCopyAccelerationStructureKHR(
			VkDeferredOperationKHR                      deferredOperation,
			const VkCopyAccelerationStructureInfoKHR* pInfo) const noexcept;
		VkResult  vkCopyAccelerationStructureToMemoryKHR(
			VkDeferredOperationKHR                      deferredOperation,
			const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const noexcept;
		VkResult  vkCopyMemoryToAccelerationStructureKHR(
			VkDeferredOperationKHR                      deferredOperation,
			const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const noexcept;
		VkResult  vkCreateAccelerationStructureKHR(
			const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkAccelerationStructureKHR* pAccelerationStructure) const noexcept;
		void  vkDestroyAccelerationStructureKHR(
			VkAccelerationStructureKHR                  accelerationStructure,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		void  vkGetAccelerationStructureBuildSizesKHR(
			VkAccelerationStructureBuildTypeKHR         buildType,
			const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
			const uint32_t* pMaxPrimitiveCounts,
			VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) const noexcept;
		VkDeviceAddress  vkGetAccelerationStructureDeviceAddressKHR(
			const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const noexcept;
		void  vkGetDeviceAccelerationStructureCompatibilityKHR(
			const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
			VkAccelerationStructureCompatibilityKHR* pCompatibility) const noexcept;
		VkResult  vkWriteAccelerationStructuresPropertiesKHR(
			uint32_t                                    accelerationStructureCount,
			const VkAccelerationStructureKHR* pAccelerationStructures,
			VkQueryType                                 queryType,
			size_t                                      dataSize,
			void* pData,
			size_t                                      stride) const noexcept;
#endif /* defined(VK_KHR_acceleration_structure) */
#if defined(VK_KHR_deferred_host_operations)
		VkResult  vkCreateDeferredOperationKHR(
			const VkAllocationCallbacks* pAllocator,
			VkDeferredOperationKHR* pDeferredOperation) const noexcept;
		VkResult  vkDeferredOperationJoinKHR(
			VkDeferredOperationKHR                      operation) const noexcept;
		void  vkDestroyDeferredOperationKHR(
			VkDeferredOperationKHR                      operation,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		uint32_t  vkGetDeferredOperationMaxConcurrencyKHR(
			VkDeferredOperationKHR                      operation) const noexcept;
		VkResult  vkGetDeferredOperationResultKHR(
			VkDeferredOperationKHR                      operation) const noexcept;
#endif /* defined(VK_KHR_deferred_host_operations) */
#if defined(VK_KHR_ray_tracing_pipeline)
		void  vkCmdSetRayTracingPipelineStackSizeKHR(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    pipelineStackSize) const noexcept;
		void  vkCmdTraceRaysIndirectKHR(
			VkCommandBuffer                             commandBuffer,
			const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
			VkDeviceAddress                             indirectDeviceAddress) const noexcept;
		void  vkCmdTraceRaysKHR(
			VkCommandBuffer                             commandBuffer,
			const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
			uint32_t                                    width,
			uint32_t                                    height,
			uint32_t                                    depth) const noexcept;
		VkResult  vkCreateRayTracingPipelinesKHR(
			VkDeferredOperationKHR                      deferredOperation,
			VkPipelineCache                             pipelineCache,
			uint32_t                                    createInfoCount,
			const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
			const VkAllocationCallbacks* pAllocator,
			VkPipeline* pPipelines) const noexcept;
		VkResult  vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(
			VkPipeline                                  pipeline,
			uint32_t                                    firstGroup,
			uint32_t                                    groupCount,
			size_t                                      dataSize,
			void* pData) const noexcept;
		VkResult  vkGetRayTracingShaderGroupHandlesKHR(
			VkPipeline                                  pipeline,
			uint32_t                                    firstGroup,
			uint32_t                                    groupCount,
			size_t                                      dataSize,
			void* pData) const noexcept;
		VkDeviceSize  vkGetRayTracingShaderGroupStackSizeKHR(
			VkPipeline                                  pipeline,
			uint32_t                                    group,
			VkShaderGroupShaderKHR                      groupShader) const noexcept;
#endif /* defined(VK_KHR_ray_tracing_pipeline) */
#if defined(VK_KHR_swapchain)
		VkResult  vkAcquireNextImageKHR(
			VkSwapchainKHR                              swapchain,
			uint64_t                                    timeout,
			VkSemaphore                                 semaphore,
			VkFence                                     fence,
			uint32_t* pImageIndex) const noexcept;
		VkResult vkCreateSwapchainKHR(
			const VkSwapchainCreateInfoKHR* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkSwapchainKHR* pSwapchain) const noexcept;
		void  vkDestroySwapchainKHR(
			VkSwapchainKHR                              swapchain,
			const VkAllocationCallbacks* pAllocator) const noexcept;
		VkResult  vkGetSwapchainImagesKHR(
			VkSwapchainKHR                              swapchain,
			uint32_t* pSwapchainImageCount,
			VkImage* pSwapchainImages) const noexcept;
		VkResult  vkQueuePresentKHR(
			VkQueue                                     queue,
			const VkPresentInfoKHR* pPresentInfo) const noexcept;
#endif /* defined(VK_KHR_swapchain) */
		VkResult vmaCreateAllocator(
			VmaAllocatorCreateFlags flags,
			VkPhysicalDevice physicalDevice, 
			const VkAllocationCallbacks* pAllocationCallbacks,
			VkInstance instance,
			uint32_t apiVersion,
			VmaAllocator *pAllocator)const noexcept;

		VkDevice get_handle() const noexcept;
	private:
		VkDevice m_handle {VK_NULL_HANDLE};
		VolkDeviceTable m_table{};
		const VkAllocationCallbacks* m_allocator{nullptr};
	};
};