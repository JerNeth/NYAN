#ifndef VKCOMMANDBUFFER_H
#define VKCOMMANDBUFFER_H
#pragma once
#include "VulkanIncludes.h"
#include "Pipeline.h"
#include <Util>
#include "VulkanForwards.h"
namespace vulkan {
	class CommandBuffer {
	public:
		
	
		CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle, CommandBufferType type = CommandBufferType::Generic, uint32_t threadIdx = 0);


		//void begin_rendering()
		//void bind_pipeline(pipelineIdentifier); -> also binds (actually used) input attachments according to renderpass
		//PipelineReference current_pipeline(bindPoint);
		void begin_rendering(const VkRenderingInfo& info);
		void end_rendering();
		GraphicsPipelineBind bind_graphics_pipeline(PipelineId pipelineIdentifier);
		ComputePipelineBind bind_compute_pipeline(PipelineId pipelineIdentifier);
		RaytracingPipelineBind bind_raytracing_pipeline(PipelineId pipelineIdentifier);

		void copy_buffer(const Buffer& dst, const Buffer& src, VkDeviceSize dstOffset, VkDeviceSize srcOffset, VkDeviceSize size);
		void copy_buffer(const Buffer& dst, const Buffer& src, const VkBufferCopy* copies, uint32_t copyCount);
		void copy_buffer(const Buffer& dst, const Buffer& src);
		void blit_image(const Image& dst, const Image& src, const VkOffset3D &dstOffset, const VkOffset3D &dstExtent,
						const VkOffset3D &srcOffset, const VkOffset3D &srcExtent, uint32_t dstLevel, uint32_t srcLevel,
						uint32_t dstLayer, uint32_t srcLayer, uint32_t layerCount, VkFilter filter);
		void generate_mips(const Image& image);
		void copy_buffer_to_image(const Image& image, const Buffer& buffer, uint32_t blitCounts, const VkBufferImageCopy* blits);
		void copy_acceleration_structure(const AccelerationStructure& src, const AccelerationStructure& dst, bool compact);
		void mip_barrier(const Image& image, VkImageLayout layout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, bool needBarrier);
		void barrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, uint32_t barrierCount,
			const VkMemoryBarrier* globals, uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier* bufferBarriers,
			uint32_t imageBarrierCounts, const VkImageMemoryBarrier* imageBarriers);
		void barrier2(VkDependencyFlags dependencyFlags, uint32_t barrierCount, const VkMemoryBarrier2* globals,
			uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers,
			uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers);
		void reset_event2(VkEvent event, VkPipelineStageFlags2 stages);
		void set_event2(VkEvent event, VkDependencyFlags dependencyFlags, uint32_t barrierCount, const VkMemoryBarrier2* globals,
			uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers,
			uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers);
		void wait_event2(VkEvent event, VkDependencyFlags dependencyFlags, uint32_t barrierCount, const VkMemoryBarrier2* globals,
			uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers,
			uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers);
		void wait_events2(uint32_t eventCount, const VkEvent* event, const VkDependencyInfo* dependencyInfo);
		void barrier(VkPipelineStageFlags srcStages, VkAccessFlags srcAccess, VkPipelineStageFlags dstStages, VkAccessFlags dstAccess);
		void image_barrier(const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkPipelineStageFlags srcStages, VkAccessFlags srcAccessFlags,
			VkPipelineStageFlags dstStages,VkAccessFlags dstAccessFlags);
		bool swapchain_touched() const noexcept;
		void touch_swapchain() noexcept;
		VkCommandBuffer get_handle() const noexcept;
		operator VkCommandBuffer() const noexcept;
		void end();
		void begin_region(const char* name, const float* color = nullptr);
		void end_region();
		CommandBufferType get_type() const noexcept;
	private:
		/// *******************************************************************
		/// Private functions
		/// *******************************************************************
		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		LogicalDevice& r_device;
		VkCommandBuffer m_vkHandle;
		CommandBufferType m_type;
		uint32_t m_threadIdx = 0;
		bool m_swapchainTouched = false;
		bool m_isSecondary = false;
	};
	using CommandBufferHandle = Utility::ObjectHandle<CommandBuffer ,Utility::Pool<CommandBuffer>>;
}
#endif //VKCOMMANDBUFFER_H