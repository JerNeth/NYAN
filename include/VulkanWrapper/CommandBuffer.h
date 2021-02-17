#ifndef VKCOMMANDBUFFER_H
#define VKCOMMANDBUFFER_H
#pragma once
#include "VulkanIncludes.h"
#include "Pipeline.h"
#include "Utility.h"
namespace vulkan {
	class LogicalDevice;
	class Framebuffer;
	class Renderpass;
	class ImageView;
	class Buffer;
	class Sampler;
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
		std::pair<const VkBuffer*, const VkDeviceSize*> operator[](size_t idx) const noexcept{
			return {buffers.data() + idx, offsets.data() +idx};
		}
		void update(std::bitset<MAX_VERTEX_BINDINGS> updateMask) {
			dirty &= ~updateMask;
		}
	};
	struct ResourceBindings {
		std::array<std::array<ResourceBinding, MAX_BINDINGS>, MAX_DESCRIPTOR_SETS> bindings;
		std::array<std::array<Utility::UID, MAX_BINDINGS>, MAX_DESCRIPTOR_SETS> bindingIds;
		std::array<std::array<Utility::UID, MAX_BINDINGS>, MAX_DESCRIPTOR_SETS> samplerIds;
		std::array<std::byte, PUSH_CONSTANT_SIZE> pushConstantData;
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
			Size
		};
	public:
		
	
		CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle, Type type = Type::Generic, uint32_t threadIdx = 0);
		void begin_context();
		void begin_graphics();
		void begin_compute();
		bool flush_graphics();
		bool flush_graphics_pipeline();
		void flush_descriptor_sets();
		void flush_descriptor_set(uint32_t set);
		void rebind_descriptor_set(uint32_t set);
		void copy_buffer(const Buffer& dst, const Buffer& src, VkDeviceSize dstOffset, VkDeviceSize srcOffset, VkDeviceSize size);
		void copy_buffer(const Buffer& dst, const Buffer& src, const VkBufferCopy* copies, uint32_t copyCount);
		void copy_buffer(const Buffer& dst, const Buffer& src);
		void blit_image(const Image& dst, const Image& src, const VkOffset3D &dstOffset, const VkOffset3D &dstExtent,
						const VkOffset3D &srcOffset, const VkOffset3D &srcExtent, uint32_t dstLevel, uint32_t srcLevel,
						uint32_t dstLayer, uint32_t srcLayer, uint32_t layerCount, VkFilter filter);
		void generate_mips(const Image& image);
		void copy_buffer_to_image(const Image& image, const Buffer& buffer, uint32_t blitCounts, const VkBufferImageCopy *blits);
		void mip_barrier(const Image& image, VkImageLayout layout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, bool needBarrier);
		void barrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, uint32_t barrierCount,
			const VkMemoryBarrier * globals, uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier* bufferBarriers,
			uint32_t imageBarrierCounts, const VkImageMemoryBarrier *imageBarriers);
		void barrier(VkPipelineStageFlags srcStages, VkAccessFlags srcAccess, VkPipelineStageFlags dstStages, VkAccessFlags dstAccess);
		void image_barrier(const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkPipelineStageFlags srcStages, VkAccessFlags srcAccessFlags,
			VkPipelineStageFlags dstStages,VkAccessFlags dstAccessFlags);
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
		 //TODO: Add Buffer to bind
		void push_constants(const void* data, VkDeviceSize offset, VkDeviceSize range);
		void bind_index_buffer(IndexState indexState);
		void init_viewport_and_scissor(const RenderpassCreateInfo& info);
		void begin_render_pass(const RenderpassCreateInfo& renderpassInfo, VkSubpassContents contents= VK_SUBPASS_CONTENTS_INLINE);
		void end_render_pass();
		void submit_secondary_command_buffer(const CommandBuffer& secondary);
		void set_vertex_attribute(uint32_t location, uint32_t binding, VkFormat format);
		void set_scissor(VkRect2D scissor);
		VkRect2D get_scissor() const;
		void bind_program(Program* program);
		void next_subpass(VkSubpassContents subpass);
		bool swapchain_touched() const noexcept;
		void touch_swapchain() noexcept;
		void bind_input_attachment(uint32_t set, uint32_t startBinding);
		void bind_texture(uint32_t set, uint32_t binding, const ImageView& view, const Sampler* sampler);
		void bind_texture(uint32_t set, uint32_t binding, const ImageView& view, DefaultSampler sampler);
		void bind_sampler(uint32_t set, uint32_t binding, const Sampler* sampler);
		void bind_sampler(uint32_t set, uint32_t binding, DefaultSampler sampler);
		void bind_texture(uint32_t set, uint32_t binding, VkImageView floatView, VkImageView integerView, VkImageLayout layout, Utility::UID bindingID);
		void bind_uniform_buffer(uint32_t set, uint32_t binding, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize size);
		void bind_uniform_buffer(uint32_t set, uint32_t binding, const Buffer& buffer);
		void bind_vertex_buffer(uint32_t binding, const Buffer& buffer, VkDeviceSize offset, VkVertexInputRate inputRate, VkDeviceSize vertexStride = 0);
		VkCommandBuffer get_handle() const noexcept;
		void end();
		void begin_region(const char* name, const float* color = nullptr);
		void end_region();
		Type get_type() const noexcept {
			return m_type;
		}
		void set_depth_test(VkBool32 enabled) noexcept {
			if (m_pipelineState.state.dynamic_depth_test) {
				if (m_dynamicState.depth_test != static_cast<unsigned>(enabled)) {
					m_dynamicState.depth_test = static_cast<unsigned>(enabled);
					m_invalidFlags.set(InvalidFlags::DepthTest);
				}
			}
			else {
				if (m_pipelineState.state.depth_test != static_cast<unsigned>(enabled)) {
					m_invalidFlags.set(InvalidFlags::StaticPipeline);
						m_pipelineState.state.depth_test = static_cast<unsigned>(enabled);
				}
			}
		}
		void set_depth_write(VkBool32 enabled) noexcept {
			if (m_pipelineState.state.dynamic_depth_write) {
				if (m_dynamicState.depth_write != static_cast<unsigned>(enabled)) {
					m_dynamicState.depth_write = static_cast<unsigned>(enabled);
					m_invalidFlags.set(InvalidFlags::DepthWrite);
				}
			}
			else {
				if (m_pipelineState.state.depth_write != enabled) {
					m_invalidFlags.set(InvalidFlags::StaticPipeline);
					m_pipelineState.state.depth_write = enabled;
				}
			}
		}
		void set_depth_compare(VkCompareOp compare) noexcept {
			if (m_pipelineState.state.dynamic_depth_compare) {
				if (m_dynamicState.depth_compare != static_cast<unsigned>(compare)) {
					m_dynamicState.depth_compare = static_cast<unsigned>(compare);
					m_invalidFlags.set(InvalidFlags::DepthCompare);
				}
			}
			else {
				if (m_pipelineState.state.depth_compare != static_cast<unsigned>(compare)) {
					m_invalidFlags.set(InvalidFlags::StaticPipeline);
						m_pipelineState.state.depth_compare = static_cast<unsigned>(compare);
				}
			}
		}
		void set_cull_mode(VkCullModeFlags cullMode) noexcept {
			if (m_pipelineState.state.dynamic_cull_mode) {
				if (m_dynamicState.cull_mode != static_cast<unsigned>(cullMode)) {
					m_dynamicState.cull_mode = static_cast<unsigned>(cullMode);
					m_invalidFlags.set(InvalidFlags::CullMode);
				}
			}
			else {
				if (m_pipelineState.state.cull_mode != static_cast<unsigned>(cullMode)) {
					m_invalidFlags.set(InvalidFlags::StaticPipeline);
						m_pipelineState.state.cull_mode = static_cast<unsigned>(cullMode);
				}
			}
		}
		void set_front_face(VkFrontFace frontFace) noexcept {
			if (m_pipelineState.state.dynamic_front_face) {
				if (m_dynamicState.front_face != static_cast<unsigned>(frontFace)) {
					m_dynamicState.front_face = static_cast<unsigned>(frontFace);
					m_invalidFlags.set(InvalidFlags::FrontFace);
				}
			}
			else {
				if (m_pipelineState.state.front_face != static_cast<unsigned>(frontFace)) {
					m_invalidFlags.set(InvalidFlags::StaticPipeline);
						m_pipelineState.state.front_face = static_cast<unsigned>(frontFace);
				}
			}
		}
		void set_topology(VkPrimitiveTopology topology) noexcept {
			if (m_pipelineState.state.dynamic_primitive_topology) {
				if (m_dynamicState.topology != static_cast<unsigned>(topology)) {
					m_dynamicState.topology = static_cast<unsigned>(topology);
					m_invalidFlags.set(InvalidFlags::FrontFace);
				}
			}
			else {
				if (m_pipelineState.state.topology != static_cast<unsigned>(topology)) {
					m_invalidFlags.set(InvalidFlags::StaticPipeline);
						m_pipelineState.state.topology = static_cast<unsigned>(topology);
				}
			}
		}
		void set_blend_enable(VkBool32 enabled) noexcept {
			if (m_pipelineState.state.blend_enable != static_cast<unsigned>(enabled)) {
				m_invalidFlags.set(InvalidFlags::StaticPipeline);
					m_pipelineState.state.blend_enable = static_cast<unsigned>(enabled);
			}
		}
		void set_src_color_blend(VkBlendFactor blendFactor) noexcept {
			if (m_pipelineState.state.src_color_blend != static_cast<unsigned>(blendFactor)) {
				m_invalidFlags.set(InvalidFlags::StaticPipeline);
					m_pipelineState.state.src_color_blend = static_cast<unsigned>(blendFactor);
			}
		}
		void set_dst_color_blend(VkBlendFactor blendFactor) noexcept {
			if (m_pipelineState.state.dst_color_blend != static_cast<unsigned>(blendFactor)) {
				m_invalidFlags.set(InvalidFlags::StaticPipeline);
					m_pipelineState.state.dst_color_blend = static_cast<unsigned>(blendFactor);
			}
		}
		void set_src_alpha_blend(VkBlendFactor blendFactor) noexcept {
			if (m_pipelineState.state.src_alpha_blend != static_cast<unsigned>(blendFactor)) {
				m_invalidFlags.set(InvalidFlags::StaticPipeline);
					m_pipelineState.state.src_alpha_blend = static_cast<unsigned>(blendFactor);
			}
		}
		void set_dst_alpha_blend(VkBlendFactor blendFactor) noexcept {
			if (m_pipelineState.state.dst_alpha_blend != static_cast<unsigned>(blendFactor)) {
				m_invalidFlags.set(InvalidFlags::StaticPipeline);
					m_pipelineState.state.dst_alpha_blend = static_cast<unsigned>(blendFactor);
			}
		}
		void reset_pipeline_state() noexcept {
			if (m_pipelineState.state != defaultPipelineState) {
				m_pipelineState.state = defaultPipelineState;
				m_invalidFlags.set(InvalidFlags::StaticPipeline);
			}
		}
		void set_polygon_mode(VkPolygonMode polygon_mode) noexcept {
			if (m_pipelineState.state.polygon_mode != static_cast<unsigned>(polygon_mode)) {
				m_invalidFlags.set(InvalidFlags::StaticPipeline);
				m_pipelineState.state.polygon_mode = static_cast<unsigned>(polygon_mode);
			}
		}
		void disable_depth() noexcept {
			set_depth_test(VK_FALSE);
			set_depth_write(VK_FALSE);
		}
		void enable_alpha() noexcept {
			set_blend_enable(VK_TRUE);
			set_src_color_blend(VK_BLEND_FACTOR_SRC_ALPHA);
			set_dst_color_blend(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
			set_src_alpha_blend(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
			set_dst_alpha_blend(VK_BLEND_FACTOR_ZERO);
		}
	private:
		/// *******************************************************************
		/// Private functions
		/// *******************************************************************
		void bind_vertex_buffers() noexcept;
		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		LogicalDevice& r_device;
		VkCommandBuffer m_vkHandle;
		bool m_isSecondary = false;
		bool m_swapchainTouched = false;
		bool m_isCompute = true;
		VkSubpassContents m_currentContents = VK_SUBPASS_CONTENTS_INLINE;
		uint32_t m_threadIdx = 0;
		IndexState m_indexState{};
		VertexState m_vertexState{};
		Framebuffer* m_currentFramebuffer = nullptr;
		Renderpass* m_currentRenderpass = nullptr;
		VkPipeline m_currentPipeline = VK_NULL_HANDLE;
		PipelineCompile m_pipelineState;
		PipelineLayout* m_currentPipelineLayout = nullptr;
		VkRect2D m_scissor{};
		VkViewport m_viewport{};
		std::array<ImageView*, MAX_ATTACHMENTS + 1> m_framebufferAttachments;
		Utility::bitset<static_cast<size_t>(InvalidFlags::Size), InvalidFlags> m_invalidFlags;
		Utility::bitset<MAX_DESCRIPTOR_SETS> m_dirtyDescriptorSets;
		Utility::bitset<MAX_DESCRIPTOR_SETS> m_dirtyDescriptorSetsDynamicOffsets;
		std::array<VkDescriptorSet, MAX_DESCRIPTOR_SETS> m_allocatedDescriptorSets;
		DynamicState m_dynamicState;
		ResourceBindings m_resourceBindings;
		Type m_type;
	};
	using CommandBufferHandle = Utility::ObjectHandle<CommandBuffer ,Utility::Pool<CommandBuffer>>;
}
#endif //VKCOMMANDBUFFER_H