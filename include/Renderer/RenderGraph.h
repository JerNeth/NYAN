#pragma once
#ifndef RDRENDERGRAPH_H
#define RDRENDERGRAPH_H
#include "VkWrapper.h"
#include <variant>
#include <set>
namespace nyan {

	struct ImageAttachment {
		enum class Size : uint8_t {
			Absolute,
			Swapchain
		};
		VkFormat format = VK_FORMAT_UNDEFINED;
		Size size = Size::Swapchain;
		float width = 1.0f;
		float height = 1.0f;
		Math::vec4 clearColor;
	};
	struct BufferAttachment {
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
	};
	using Attachment = std::variant<ImageAttachment, BufferAttachment>;
	using RenderResourceId = uint32_t;
	constexpr RenderResourceId InvalidResourceId = UINT32_MAX;
	struct RenderResource {
		enum class Type : uint8_t {
			Image,
			Buffer
		};
		enum class UseType : uint32_t {
			Sample,
			ImageLoad,
			Attachment,
			ImageStore,
			BlitSource,
			BlitTarget,
			CopySource,
			CopyTarget,
			Size
		};
		RenderResource() = default;
		RenderResource(RenderResourceId id) : m_id(id){}
		Type m_type = Type::Image;
		RenderResourceId m_id = InvalidResourceId;
		std::set<uint32_t> m_readIn;
		std::set<uint32_t> m_writeToIn;
		std::vector<Utility::bitset<static_cast<size_t>(UseType::Size), UseType>> m_uses;
		Utility::bitset<static_cast<size_t>(UseType::Size), UseType> totalUses;
		Attachment attachment;
		vulkan::Image* handle = nullptr;
	};
	struct Barrier {
		RenderResourceId resourceId = InvalidResourceId;
		VkPipelineStageFlags src = 0;
		VkPipelineStageFlags dst = 0;
		uint16_t bufferBarrierCount = 0;
		uint16_t bufferBarrierOffset = 0;
		uint16_t imageBarrierCount = 0;
		uint16_t imageBarrierOffset = 0;
	};
	
	class Rendergraph;
	using RenderpassId = uint32_t;
	constexpr RenderpassId invalidRenderpassId = UINT32_MAX;
	struct ImageBarrier {
		RenderpassId src = invalidRenderpassId;
		RenderpassId dst = invalidRenderpassId;
		//uint32_t srcFamily = VK_QUEUE_FAMILY_IGNORED;
		//uint32_t dstFamily = VK_QUEUE_FAMILY_IGNORED;
		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		VkAccessFlags srcAccess = 0;
		VkAccessFlags dstAccess = 0;
		VkImageLayout srcLayout = static_cast<VkImageLayout>(0);
		VkImageLayout dstLayout = static_cast<VkImageLayout>(0);
	};
	struct BufferBarrier {
		RenderpassId src = invalidRenderpassId;
		RenderpassId dst = invalidRenderpassId;
		//uint32_t srcFamily = VK_QUEUE_FAMILY_IGNORED;
		//uint32_t dstFamily = VK_QUEUE_FAMILY_IGNORED;
		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;
		VkAccessFlags srcAccess = 0;
		VkAccessFlags dstAccess = 0;
	};
	class Renderpass {
	public:
		struct Read {
			enum class Type {
				ImageColor,
				ImageDepth,
				ImageStencil,
			};
			RenderResourceId id;
			Type type;
			VkImageView view;
			uint32_t binding = InvalidResourceId;
		};
		struct Write {
			enum class Type {
				Graphics,
				Compute
			};
			RenderResourceId id;
			Type type;
			VkImageView view;
			uint32_t binding = InvalidResourceId;
		};
	private:
		friend class Rendergraph;
	public:
		enum class Type :uint8_t{
			Generic,
			AsyncCompute,
			Transfer
		};
		Renderpass(Rendergraph& graph, Type type, uint32_t id, const std::string& name);
		Renderpass(const Renderpass&) = delete;
		Renderpass(Renderpass&&) = default;
		Renderpass& operator=(const Renderpass&) = delete;
		Renderpass& operator=(Renderpass&&) = default;
		void add_read(const std::string& name, Renderpass::Read::Type readType = Read::Type::ImageColor);
		void add_attachment(const std::string& name, ImageAttachment attachment);
		void add_attachment(const std::string& name);
		void add_swapchain_attachment(Math::vec4 clearColor = Math::vec4{0.48f, 0.66f, 0.35f, 1.f});
		void add_depth_attachment(const std::string& name, ImageAttachment attachment);
		void add_depth_attachment(const std::string& name);
		void add_depth_stencil_attachment(const std::string& name, ImageAttachment attachment);
		void add_depth_stencil_attachment(const std::string& name);
		void add_stencil_attachment(const std::string& name, ImageAttachment attachment);
		void add_stencil_attachment(const std::string& name);
		void add_write(const std::string& name, ImageAttachment attachment, Renderpass::Write::Type writeType = Write::Type::Graphics);
		void add_swapchain_write(Math::vec4 clearColor = Math::vec4{ 0.48f, 0.66f, 0.35f, 1.f }, Renderpass::Write::Type writeType = Write::Type::Graphics);
		//void add_read_dependency(const std::string& name, bool storageImage = false);
		void add_renderfunction(const std::function<void(vulkan::CommandBufferHandle&, Renderpass&) > & functor, bool renderpass) {
			m_renderFunctions.push_back(functor);
			m_useRendering.push_back(renderpass);
		}
		void copy(const std::string& source, const std::string& target);
		uint32_t get_id() const noexcept {
			return m_id;
		}
		Type get_type() const noexcept {
			return m_type;
		}
		void execute(vulkan::CommandBufferHandle& cmd);
		void do_copies(vulkan::CommandBufferHandle& cmd);
		bool has_post_barriers() const noexcept {
			return !m_bufferBarriers.empty() || !m_imageBarriers.empty();
		}
		void apply_pre_barriers(vulkan::CommandBufferHandle& cmd);
		void apply_copy_barriers(vulkan::CommandBufferHandle& cmd);
		void apply_post_barriers(vulkan::CommandBufferHandle& cmd);
		vulkan::PipelineId add_pipeline(vulkan::GraphicsPipelineConfig config);
		void begin_rendering(vulkan::CommandBufferHandle& cmd);
		void end_rendering(vulkan::CommandBufferHandle& cmd);
		uint32_t get_write_bind(uint32_t idx);
		uint32_t get_read_bind(uint32_t idx);
		uint32_t get_write_bind(std::string_view v, Write::Type type = Write::Type::Graphics);
		uint32_t get_read_bind(std::string_view v, Read::Type type = Read::Type::ImageColor);
		void add_wait(VkSemaphore wait, VkPipelineStageFlags stage);
		void add_signal(uint32_t passId, VkPipelineStageFlags stage);
	private:
		bool is_read(RenderResourceId id) const;
		bool is_write(RenderResourceId id) const;
		bool is_compute_write(RenderResourceId id) const;
		bool is_attachment(RenderResourceId id) const;
		bool is_write(const RenderResource& resource) const;
		bool is_compute_write(const RenderResource& resource) const;
		bool is_attachment(const RenderResource& resource) const;


		Rendergraph& r_graph;
		std::string m_name;
		Type m_type;
		uint32_t m_id;
		std::vector<std::function<void(vulkan::CommandBufferHandle&, Renderpass&)>> m_renderFunctions;
		std::vector<bool> m_useRendering;
		bool m_rendersSwap = false;
		//Order Renderpass ressources as Reads first, then writes, i.e. [R] 1, [R] 5, [W] 2, [W] 3
		std::vector<Read> m_reads;
		std::vector<Write> m_writes;
		std::vector<RenderResourceId> m_attachments;
		RenderResourceId m_depth = InvalidResourceId;
		RenderResourceId m_stencil = InvalidResourceId;

		std::vector<Barrier> m_postBarriers;
		std::vector<Barrier> m_copyBarriers;
		std::vector<Barrier> m_preBarriers;
		std::vector<VkImageMemoryBarrier> m_imageBarriers;
		std::vector<VkBufferMemoryBarrier> m_bufferBarriers;

		struct ImageBarriers {
			std::vector<RenderResourceId> images;
			std::vector<VkImageMemoryBarrier2> barriers;
		} m_imageBarriers2;

		size_t m_imagePostBarrierIndex{ 0 };
		size_t m_imageCopyBarrierIndex{ 0 };
		size_t m_imagePreBarrierIndex{ 0 };

		struct BufferBarriers {
			std::vector<RenderResourceId> buffers;
			std::vector<VkBufferMemoryBarrier2> barriers;
		} m_bufferBarriers2;

		size_t m_bufferPostBarrierIndex{ 0 };
		size_t m_bufferCopyBarrierIndex{ 0 };
		size_t m_bufferPreBarrierIndex{ 0 };

		VkRenderingInfo m_renderInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		};
		std::array<VkRenderingAttachmentInfo, vulkan::MAX_ATTACHMENTS> m_colorAttachments;
		VkRenderingAttachmentInfo m_depthAttachment;
		VkRenderingAttachmentInfo m_stencilAttachment;
		vulkan::RenderingCreateInfo m_renderingCreateInfo;

		std::vector<VkSemaphore> m_waitSemaphores;
		std::vector<VkPipelineStageFlags> m_waitStages;
		std::vector<VkPipelineStageFlags> m_signalStages;
		std::vector<uint32_t> m_signalPassIds;

	};
	class Rendergraph {
		friend class Renderpass;
	private:
		enum class State : uint8_t{
			Setup,
			Build,
			Execute
		};
	public:
		Rendergraph(vulkan::LogicalDevice& device);
		Renderpass& add_pass(const std::string& name, Renderpass::Type type);
		Renderpass& get_pass(const std::string& name);
		void build();
		void execute();
		RenderResource& add_ressource(const std::string& name, Attachment attachment);
		RenderResource& get_resource(std::string_view v);
		bool resource_exists(std::string_view v);
		const RenderResource& get_resource(std::string_view v) const;
		void set_swapchain(const std::string& name);
		vulkan::LogicalDevice& get_device() const;
	private:
		RenderResource& get_resource(RenderResourceId id);
		const RenderResource& get_resource(RenderResourceId id) const;
		void swapchain_present_transition(RenderpassId src_const);
		void swapchain_write_transition(RenderpassId src_const);
		void swapchain_present_transition2(RenderpassId src_const);
		void swapchain_write_transition2(RenderpassId src_const);
		void set_up_transition(RenderpassId from, RenderpassId to, const RenderResource& resource);
		void set_up_first_transition(RenderpassId dst, const RenderResource& resource);
		void set_up_copy(RenderpassId dst, const RenderResource& resource);
		void set_up_RaW(RenderpassId write, RenderpassId read, const RenderResource& resource);
		void set_up_WaW(RenderpassId src_, RenderpassId dst_, const RenderResource& resource);
		void set_up_WaR(RenderpassId write, RenderpassId read, const RenderResource& resource);
		void set_up_barrier(const ImageBarrier& imageBarrier, const RenderResource& resource);

		State m_state = State::Setup;
		vulkan::LogicalDevice& r_device;
		std::vector<Renderpass*> m_submissionOrder;
		Utility::NonInvalidatingMap<std::string, Renderpass> m_renderpasses;
		Utility::NonInvalidatingMap<std::string, RenderResource> m_renderresources;
		RenderResourceId m_swapchainResource = InvalidResourceId;
		RenderpassId m_renderpassCount = 0;
		RenderResourceId m_resourceCount = 0;
	};
}

#endif !RDRENDERGRAPH_H