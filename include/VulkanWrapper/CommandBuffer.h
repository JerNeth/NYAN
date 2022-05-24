#ifndef VKCOMMANDBUFFER_H
#define VKCOMMANDBUFFER_H
#pragma once
#include "VulkanIncludes.h"
#include "Pipeline.h"
#include <Util>
namespace vulkan {
	class LogicalDevice;
	class Framebuffer;
	class Renderpass;
	class ImageView;
	class Buffer;
	class Sampler;
	class AccelerationStructure;
	enum class DefaultSampler;
	struct RenderpassCreateInfo;
	struct IndexState {
		VkBuffer buffer;
		VkDeviceSize offset;
		VkIndexType indexType;
		friend bool operator==(const IndexState& lhs, const IndexState& rhs) {
			return lhs.buffer == rhs.buffer &&
				lhs.offset == rhs.offset &&
				lhs.indexType == rhs.indexType;
		}
	};
	struct VertexState {
		std::array<VkBuffer, MAX_VERTEX_BINDINGS> buffers;
		std::array<VkDeviceSize, MAX_VERTEX_BINDINGS> offsets;
		std::bitset<MAX_VERTEX_BINDINGS> dirty{ ~0ull };
		std::bitset<MAX_VERTEX_BINDINGS> active{ 0ull };
		std::pair<const VkBuffer*, const VkDeviceSize*> operator[](size_t idx) const noexcept {
			return { buffers.data() + idx, offsets.data() + idx };
		}
		void update(std::bitset<MAX_VERTEX_BINDINGS> updateMask) {
			dirty &= ~updateMask;
		}
	};
	struct DynamicVertexState {
		uint32_t inputBindingCount = 0;
		uint32_t inputAttributesCount = 0;
		std::array<VkVertexInputAttributeDescription2EXT, MAX_VERTEX_BINDINGS> inputAttributes;
		std::array<VkVertexInputBindingDescription2EXT, MAX_VERTEX_BINDINGS> inputBindings;
	};
	struct ResourceBindings {
		std::array<std::array<std::vector<ResourceBinding>, MAX_BINDINGS>, MAX_DESCRIPTOR_SETS> bindings;
		std::array<std::array<std::vector<VkDeviceSize>, MAX_BINDINGS>, MAX_DESCRIPTOR_SETS> dynamicOffsets;
		std::array<std::array<std::vector<Utility::UID>, MAX_BINDINGS>, MAX_DESCRIPTOR_SETS> bindingIds;
		std::array<std::array<std::vector<Utility::UID>, MAX_BINDINGS>, MAX_DESCRIPTOR_SETS> samplerIds;
		std::array<std::byte, PUSH_CONSTANT_SIZE> pushConstantData{};
	};
	struct DynamicState {
		float depthBias = 0.0f;
		float depthBiasSlope = 0.0f;
		uint32_t frontCompareMask = 0;
		uint32_t frontWriteMask = 0;
		uint32_t frontReference = 0;
		uint32_t backCompareMask = 0;
		uint32_t backWriteMask = 0;
		uint32_t backReference = 0;
		unsigned cull_mode : CULL_MODE_BITS;
		unsigned front_face : FRONT_FACE_BITS;
		unsigned topology : TOPOLOGY_BITS;
		unsigned depth_write : 1;
		unsigned depth_test : 1;
		unsigned depth_compare : COMPARE_OP_BITS;
		unsigned depth_bound_test : 1;
		unsigned stencil_test : 1;
		unsigned depth_bias_enable : 1;
		unsigned primitive_restart : 1;
		unsigned rasterizer_discard : 1;
		unsigned stencil_front_fail : STENCIL_OP_BITS;
		unsigned stencil_front_pass : STENCIL_OP_BITS;
		unsigned stencil_front_depth_fail : STENCIL_OP_BITS;
		unsigned stencil_back_fail : STENCIL_OP_BITS;
		unsigned stencil_back_pass : STENCIL_OP_BITS;
		unsigned stencil_back_depth_fail : STENCIL_OP_BITS;
		unsigned stencil_front_compare_op : COMPARE_OP_BITS;
		unsigned stencil_back_compare_op : COMPARE_OP_BITS;
		std::array< VkDeviceSize, MAX_VERTEX_BINDINGS> vertexStrides;
	};
	class CommandBuffer {
	public:
		enum class Type {
			Generic,
			Compute,
			Transfer
		};
		enum class InvalidFlags {
			Pipeline,
			StaticPipeline,
			StaticVertex,
			DynamicVertex,
			DynamicState,
			PushConstants,
			Viewport,
			Scissor,
			DepthBias,
			Stencil,
			CullMode,
			FrontFace,
			PrimitiveTopology,
			DepthTest,
			DepthWrite,
			DepthCompare,
			DepthBoundsTest,
			StencilTest,
			StencilOp,
			DepthBiasEnable,
			PrimitiveRestart,
			RasterizerDiscard,
			Size
		};
	public:
		
	
		CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle, Type type = Type::Generic, uint32_t threadIdx = 0);


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
			const VkMemoryBarrier * globals, uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier* bufferBarriers,
			uint32_t imageBarrierCounts, const VkImageMemoryBarrier *imageBarriers);
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
		Type get_type() const noexcept;
	private:
		/// *******************************************************************
		/// Private functions
		/// *******************************************************************
		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		LogicalDevice& r_device;
		VkCommandBuffer m_vkHandle;
		Type m_type;
		uint32_t m_threadIdx = 0;
		bool m_swapchainTouched = false;
		bool m_isSecondary = false;
	};
	using CommandBufferHandle = Utility::ObjectHandle<CommandBuffer ,Utility::Pool<CommandBuffer>>;
}
#endif //VKCOMMANDBUFFER_H