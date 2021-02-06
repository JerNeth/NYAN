#ifndef VKCOMMANDBUFFER_H
#define VKCOMMANDBUFFER_H
#pragma once
#include "VulkanIncludes.h"
#include "Pipeline.h"
#include "Utility.h"
namespace Vulkan {
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
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
		void bind_uniform_buffer(uint32_t set, uint32_t binding, VkDeviceSize offset, VkDeviceSize size); //TODO: Add Buffer to bind
		void bind_texture(uint32_t set, uint32_t binding); // TODO: Add Sampler and Image
		void bind_index_buffer(IndexState indexState);
		void init_viewport_and_scissor(const RenderpassCreateInfo& info);
		void begin_render_pass(const RenderpassCreateInfo& renderpassInfo, VkSubpassContents contents= VK_SUBPASS_CONTENTS_INLINE);
		void end_render_pass();
		void submit_secondary_command_buffer(const CommandBuffer& secondary);
		void bind_program(Program* program);
		void next_subpass(VkSubpassContents subpass);
		bool swapchain_touched() const noexcept;
		void touch_swapchain() noexcept;
		void bind_sampler(uint32_t set, uint32_t binding, const Sampler* sampler);
		void bind_sampler(uint32_t set, uint32_t binding, DefaultSampler sampler);
		void bind_texture(uint32_t set, uint32_t binding, VkImageView floatView, VkImageView integerView, VkImageLayout layout, Utility::UID bindingID);
		VkCommandBuffer get_handle() const noexcept;
		void end();
		Type get_type() const noexcept {
			return m_type;
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
	using CommandBufferHandle = Utility::PoolHandle<CommandBuffer>;
}
#endif //VKCOMMANDBUFFER_H