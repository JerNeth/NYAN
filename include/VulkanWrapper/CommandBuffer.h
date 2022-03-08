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
	struct DescriptorSet {
		uint32_t set;

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
		
	
		CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle, Type type = Type::Generic, uint32_t threadIdx = 0, bool tiny = false);
		void begin_context();
		void begin_graphics();
		void begin_compute();
		bool flush_graphics();
		bool flush_compute();
		bool flush_ray();
		bool flush_graphics_pipeline();
		bool flush_compute_pipeline();
		bool flush_ray_pipeline();
		void flush_descriptor_sets(VkPipelineBindPoint bindPoint);
		void flush_descriptor_set(uint32_t set, VkPipelineBindPoint bindPoint);
		void flush_bindless_descriptor_sets(uint32_t firstSet, uint32_t setCount, VkPipelineBindPoint bindPoint);
		void rebind_descriptor_set(uint32_t set, VkPipelineBindPoint bindPoint);
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
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
		void dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ);
		void trace_rays(uint32_t width, uint32_t height, uint32_t depth);

		void push_constants(const void* data, VkDeviceSize offset, VkDeviceSize range);
		void bind_index_buffer(IndexState indexState);
		void init_viewport_and_scissor(const RenderpassCreateInfo& info);
		void begin_render_pass(const RenderpassCreateInfo& renderpassInfo, VkSubpassContents contents= VK_SUBPASS_CONTENTS_INLINE);
		void end_render_pass();
		void submit_secondary_command_buffer(const CommandBuffer& secondary);
		void set_vertex_attribute(uint32_t location, uint32_t binding, VkFormat format);
		void set_scissor(VkRect2D scissor);
		void set_viewport(VkViewport viewport);
		VkRect2D get_scissor() const;
		void bind_program(Program* program);
		void next_subpass(VkSubpassContents subpass);
		bool swapchain_touched() const noexcept;
		void touch_swapchain() noexcept;
		void bind_storage_image(uint32_t set, uint32_t binding, uint32_t arrayIndex, const ImageView& view);
		void bind_input_attachment(uint32_t set, uint32_t startBinding);
		void bind_acceleration_structure(uint32_t set, uint32_t binding, uint32_t arrayIndex, const AccelerationStructure& accelerationStructure);
		void bind_texture(uint32_t set, uint32_t binding, uint32_t arrayIndex, const ImageView& view, const Sampler* sampler);
		void bind_texture(uint32_t set, uint32_t binding, uint32_t arrayIndex, const ImageView& view, DefaultSampler sampler);
		void bind_texture(uint32_t set, uint32_t binding, uint32_t arrayIndex, const ImageView& view);
		void bind_sampler(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Sampler* sampler);
		void bind_sampler(uint32_t set, uint32_t binding, uint32_t arrayIndex, DefaultSampler sampler);
		void bind_texture(uint32_t set, uint32_t binding, uint32_t arrayIndex, VkImageView floatView, VkImageView integerView, VkImageLayout layout, Utility::UID bindingID);
		void bind_uniform_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize size);
		void bind_uniform_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer);
		void bind_storage_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize size);
		void bind_storage_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer);
		void bind_vertex_buffer(uint32_t binding, const Buffer& buffer, VkDeviceSize offset, VkVertexInputRate inputRate);
		void bind_bindless_sets(uint32_t firstSet, VkDescriptorSet* sets, uint32_t setCount);
		VkCommandBuffer get_handle() const noexcept;
		void end();
		void begin_region(const char* name, const float* color = nullptr);
		void end_region();
		Type get_type() const noexcept;
		void set_depth_test(VkBool32 enabled) noexcept;
		void set_depth_write(VkBool32 enabled) noexcept;
		void set_depth_compare(VkCompareOp compare) noexcept;
		void set_cull_mode(VkCullModeFlags cullMode) noexcept;
		void set_front_face(VkFrontFace frontFace) noexcept;
		void set_topology(VkPrimitiveTopology topology) noexcept;
		void set_blend_enable(VkBool32 enabled) noexcept;
		void set_src_color_blend(VkBlendFactor blendFactor) noexcept;
		void set_dst_color_blend(VkBlendFactor blendFactor) noexcept;
		void set_src_alpha_blend(VkBlendFactor blendFactor) noexcept;
		void set_dst_alpha_blend(VkBlendFactor blendFactor) noexcept;
		void reset_pipeline_state() noexcept;
		void set_polygon_mode(VkPolygonMode polygon_mode) noexcept;
		void disable_depth() noexcept;
		void enable_alpha() noexcept;

		void set_dynamic_vertex_input(bool state) noexcept;
		void set_dynamic_vertex_input_binding_stride(bool state) noexcept;

	private:
		/// *******************************************************************
		/// Private functions
		/// *******************************************************************
		void bind_vertex_buffers() noexcept;
		void invalidate_dynamic_state() noexcept;
		void update_dynamic_state() noexcept;
		void update_push_constants() noexcept;
		void update_dynamic_vertex_state() noexcept;
		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		LogicalDevice& r_device;
		VkCommandBuffer m_vkHandle;
		Type m_type;
		uint32_t m_threadIdx = 0;
		bool m_tiny = false;
		bool m_swapchainTouched = false;
		bool m_isSecondary = false;
		bool m_isCompute = true;
		struct Data {
			VkSubpassContents currentContents = VK_SUBPASS_CONTENTS_INLINE;
			IndexState indexState{};
			VertexState vertexState{};
			Framebuffer* currentFramebuffer = nullptr;
			Renderpass* currentRenderpass = nullptr;
			VkPipeline currentPipeline = VK_NULL_HANDLE;
			PipelineCompile pipelineState;
			PipelineLayout* currentPipelineLayout = nullptr;
			VkRect2D scissor{};
			VkViewport viewport{};
			std::array<ImageView*, MAX_ATTACHMENTS + 1> framebufferAttachments;
			Utility::bitset<static_cast<size_t>(InvalidFlags::Size), InvalidFlags> invalidFlags;
			Utility::bitset<MAX_DESCRIPTOR_SETS> bindlessDescriptorSets;
			Utility::bitset<MAX_DESCRIPTOR_SETS> dirtyDescriptorSets;
			Utility::bitset<MAX_DESCRIPTOR_SETS> dirtyDescriptorSetsDynamicOffsets;
			std::array<VkDescriptorSet, MAX_DESCRIPTOR_SETS> allocatedDescriptorSets;
			DynamicState dynamicState;
			ResourceBindings resourceBindings;
			std::vector<ResourceBinding> bindingBuffer;
		};
		std::unique_ptr<Data> m_data;
	};
	using CommandBufferHandle = Utility::ObjectHandle<CommandBuffer ,Utility::Pool<CommandBuffer>>;
}
#endif //VKCOMMANDBUFFER_H