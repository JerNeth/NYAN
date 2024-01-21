#ifndef VKCOMMANDBUFFER_H
#define VKCOMMANDBUFFER_H
#pragma once
#include <expected>
#include <span>

#include <Util>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"
#include "VulkanWrapper/VulkanError.hpp"
#include "VulkanWrapper/VulkanObject.h"
namespace vulkan {
	class CommandBuffer : public VulkanObject<VkCommandBuffer> {
	public:
		CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle, CommandBufferType type = CommandBufferType::Generic) noexcept;


		//void begin_rendering()
		//void bind_pipeline(pipelineIdentifier); -> also binds (actually used) input attachments according to renderpass
		//PipelineReference current_pipeline(bindPoint);
		void begin_rendering(const VkRenderingInfo& info) noexcept;
		void end_rendering() noexcept;
		[[nodiscard]] GraphicsPipelineBind bind_graphics_pipeline(PipelineId pipelineIdentifier) noexcept;
		[[nodiscard]] ComputePipelineBind bind_compute_pipeline(PipelineId pipelineIdentifier) noexcept;
		[[nodiscard]] RaytracingPipelineBind bind_raytracing_pipeline(PipelineId pipelineIdentifier) noexcept;

		void write_timestamp(VkPipelineStageFlags2 stage, TimestampQueryPool& queryPool, uint32_t query) noexcept;

		void copy_buffer(const Buffer& dst, const Buffer& src, VkDeviceSize dstOffset, VkDeviceSize srcOffset, VkDeviceSize size) noexcept;
		void copy_buffer(const Buffer& dst, const Buffer& src, const VkBufferCopy* copies, uint32_t copyCount) noexcept;
		void copy_buffer(const Buffer& dst, const Buffer& src) noexcept;
		void fill_buffer(const Buffer& dst, uint32_t data) noexcept;
		void blit_image(const Image& dst, const Image& src, const VkOffset3D &dstOffset, const VkOffset3D &dstExtent,
						const VkOffset3D &srcOffset, const VkOffset3D &srcExtent, uint32_t dstLevel, uint32_t srcLevel,
						uint32_t dstLayer, uint32_t srcLayer, uint32_t layerCount, VkFilter filter) noexcept;
		void copy_image(const Image& src, const Image& dst, VkImageLayout srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, uint32_t mipLevel = 0) noexcept;
		void copy_image_to_buffer(const Image& image, const Buffer& buffer, const std::span<const VkBufferImageCopy> regions, VkImageLayout srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) noexcept;

		void generate_mips(const Image& image) noexcept;
		void copy_buffer_to_image(const Image& image, const Buffer& buffer, uint32_t blitCounts, const VkBufferImageCopy* blits) noexcept;
		void copy_acceleration_structure(const AccelerationStructure& src, const AccelerationStructure& dst, bool compact) noexcept;
		void mip_barrier(const Image& image, VkImageLayout layout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, bool needBarrier) noexcept;
		void barrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, uint32_t barrierCount = 0,
			const VkMemoryBarrier* globals = nullptr, uint32_t bufferBarrierCounts = 0, const VkBufferMemoryBarrier* bufferBarriers = nullptr,
			uint32_t imageBarrierCounts = 0, const VkImageMemoryBarrier* imageBarriers = nullptr) noexcept;
		void barrier2(const VkMemoryBarrier2& global, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void barrier2(const VkBufferMemoryBarrier2& bufferBarrier, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void barrier2(const VkImageMemoryBarrier2& imageBarrier, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void barrier2(uint32_t barrierCount = 0, const VkMemoryBarrier2* globals = nullptr,
			uint32_t bufferBarrierCounts = 0, const VkBufferMemoryBarrier2* bufferBarriers = nullptr,
			uint32_t imageBarrierCounts = 0, const VkImageMemoryBarrier2* imageBarriers = nullptr, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void barrier2(uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void barrier2(uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void reset_event2(VkEvent event, VkPipelineStageFlags2 stages) noexcept;
		void set_event2(VkEvent event, uint32_t barrierCount = 0, const VkMemoryBarrier2* globals = nullptr,
			uint32_t bufferBarrierCounts = 0, const VkBufferMemoryBarrier2* bufferBarriers = nullptr,
			uint32_t imageBarrierCounts = 0, const VkImageMemoryBarrier2* imageBarriers = nullptr, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void set_event2(VkEvent event, uint32_t barrierCount, const VkMemoryBarrier2* globals, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void set_event2(VkEvent event, uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void set_event2(VkEvent event, uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void wait_event2(VkEvent event, uint32_t barrierCount = 0, const VkMemoryBarrier2* globals = nullptr,
			uint32_t bufferBarrierCounts = 0, const VkBufferMemoryBarrier2* bufferBarriers = nullptr,
			uint32_t imageBarrierCounts = 0, const VkImageMemoryBarrier2* imageBarriers = nullptr, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void wait_event2(VkEvent event, uint32_t barrierCount, const VkMemoryBarrier2* globals, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void wait_event2(VkEvent event,	uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void wait_event2(VkEvent event, uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers, VkDependencyFlags dependencyFlags = VkDependencyFlags{ 0 }) noexcept;
		void wait_events2(uint32_t eventCount, const VkEvent* event, const VkDependencyInfo* dependencyInfo) noexcept;
		void barrier(VkPipelineStageFlags srcStages, VkAccessFlags srcAccess, VkPipelineStageFlags dstStages, VkAccessFlags dstAccess) noexcept;
		void image_barrier(const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkPipelineStageFlags srcStages, VkAccessFlags srcAccessFlags,
			VkPipelineStageFlags dstStages,VkAccessFlags dstAccessFlags) noexcept;
		void clear_color_image(const Image& image, VkImageLayout layout, const VkClearColorValue* clearColor) noexcept;
		void clear_depth_image(const Image& image, VkImageLayout layout, const VkClearDepthStencilValue* clearColor) noexcept;
		bool swapchain_touched() const noexcept;
		void touch_swapchain() noexcept;
		[[nodiscard]] std::expected<void, vulkan::Error> end() noexcept;
		void begin_region(const char* name, const float* color = nullptr) noexcept;
		void end_region() noexcept;
		[[nodiscard]] CommandBufferType get_type() const noexcept;
	private:
		/// *******************************************************************
		/// Private functions
		/// *******************************************************************
		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		CommandBufferType m_type;
		bool m_swapchainTouched = false;
		bool m_isSecondary = false;
	};
	using CommandBufferHandle = Utility::ObjectHandle<CommandBuffer ,Utility::Pool<CommandBuffer>>;
}
#endif //VKCOMMANDBUFFER_H