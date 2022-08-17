#pragma once
#ifndef RDRENDERGRAPH_H
#define RDRENDERGRAPH_H
#include "VkWrapper.h"
#include <variant>
#include <set>
#include "entt/core/hashed_string.hpp"
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
		enum class ImageUse : uint32_t {
			Sample,
			ImageLoad,
			Attachment,
			ImageStore,
			BlitSource,
			BlitTarget,
			CopySource,
			CopyTarget,
			Clear,
			Size
		};
		RenderResource() = default;
		RenderResource(RenderResourceId id) : m_id(id){}
		Type m_type = Type::Image;
		RenderResourceId m_id = InvalidResourceId;
		entt::hashed_string name;
		std::vector<Utility::bitset<static_cast<size_t>(ImageUse::Size), ImageUse>> m_uses;
		//Utility::bitset<static_cast<size_t>(ImageUse::Size), ImageUse> totalUses;
		Attachment attachment;
		vulkan::Image* handle = nullptr;
	};
		
	class Rendergraph;
	using RenderpassId = uint32_t;
	constexpr RenderpassId invalidRenderpassId = UINT32_MAX;

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
		struct Copy {
			RenderResourceId src;
			RenderResourceId dst;
		};
	private:
		friend class Rendergraph;
	public:
		enum class Type :uint8_t{
			Generic,
			AsyncCompute,
			Transfer
		};
		Renderpass(Rendergraph& graph, Type type, uint32_t id, const entt::hashed_string& name);
		Renderpass(const Renderpass&) = delete;
		Renderpass(Renderpass&&) = default;
		Renderpass& operator=(const Renderpass&) = delete;
		Renderpass& operator=(Renderpass&&) = default;

		void add_read(const entt::hashed_string& name, Renderpass::Read::Type readType = Read::Type::ImageColor);
		void add_attachment(const entt::hashed_string& name, ImageAttachment attachment, bool clear = false);
		void add_attachment(const entt::hashed_string& name, bool clear = false);
		void add_swapchain_attachment(Math::vec4 clearColor = Math::vec4{}, bool clear = false);
		void add_depth_attachment(const entt::hashed_string& name, ImageAttachment attachment, bool clear = false);
		void add_depth_attachment(const entt::hashed_string& name, bool clear = false);
		void add_depth_stencil_attachment(const entt::hashed_string& name, ImageAttachment attachment, bool clear = false);
		void add_depth_stencil_attachment(const entt::hashed_string& name, bool clear = false);
		void add_stencil_attachment(const entt::hashed_string& name, ImageAttachment attachment, bool clear = false);
		void add_stencil_attachment(const entt::hashed_string& name, bool clear = false);
		void add_write(const entt::hashed_string& name, ImageAttachment attachment, Renderpass::Write::Type writeType = Write::Type::Graphics, bool clear = false);
		void add_swapchain_write(Math::vec4 clearColor = Math::vec4{}, Renderpass::Write::Type writeType = Write::Type::Graphics, bool clear = false);
			//void add_read_dependency(const std::string& name, bool storageImage = false);
		void add_renderfunction(const std::function<void(vulkan::CommandBuffer&, Renderpass&) > & functor, bool renderpass) {
			m_renderFunctions.push_back(functor);
			m_useRendering.push_back(renderpass);
		}
		void copy(const entt::hashed_string& source, const entt::hashed_string& target);
		uint32_t get_id() const noexcept {
			return m_id;
		}
		Type get_type() const noexcept {
			return m_type;
		}
		void update();
		void execute(vulkan::CommandBuffer& cmd);
		void do_copies(vulkan::CommandBuffer& cmd);
		void apply_pre_barriers(vulkan::CommandBuffer& cmd);
		void apply_copy_barriers(vulkan::CommandBuffer& cmd);
		void apply_post_barriers(vulkan::CommandBuffer& cmd);
		void add_pre_barrier(const VkImageMemoryBarrier2& barrier, RenderResourceId image);
		void add_copy_barrier(const VkImageMemoryBarrier2& barrier, RenderResourceId image);
		void add_post_barrier(const VkImageMemoryBarrier2& barrier, RenderResourceId image);
		void add_pre_barrier(const VkMemoryBarrier2& barrier);
		void add_copy_barrier(const VkMemoryBarrier2& barrier);
		void add_post_barrier(const VkMemoryBarrier2& barrier);


		void add_pipeline(vulkan::GraphicsPipelineConfig config, vulkan::PipelineId* id);
		void begin_rendering(vulkan::CommandBuffer& cmd);
		void end_rendering(vulkan::CommandBuffer& cmd);
		uint32_t get_write_bind(uint32_t idx);
		uint32_t get_read_bind(uint32_t idx);
		uint32_t get_write_bind(const entt::hashed_string& name, Write::Type type = Write::Type::Graphics);
		uint32_t get_read_bind(const entt::hashed_string& name, Read::Type type = Read::Type::ImageColor);
		void add_wait(VkSemaphore wait, VkPipelineStageFlags2 stage);
		void add_signal(uint32_t passId, VkPipelineStageFlags2 stage);
		void build();
	private:
		void build_rendering_info();
		void build_pipelines();
		void update_binds();
		void update_image_barriers();
		void update_rendering_info();
		void update_views();
		bool is_read(RenderResourceId id) const;
		bool is_write(RenderResourceId id) const;
		bool is_compute_write(RenderResourceId id) const;
		bool is_attachment(RenderResourceId id) const;
		bool is_write(const RenderResource& resource) const;
		bool is_compute_write(const RenderResource& resource) const;
		bool is_attachment(const RenderResource& resource) const;


		Rendergraph& r_graph;
		entt::hashed_string m_name;
		Type m_type;
		uint32_t m_id;
		std::vector<std::function<void(vulkan::CommandBuffer&, Renderpass&)>> m_renderFunctions;
		std::vector<bool> m_useRendering;
		bool m_rendersSwap = false;
		//Order Renderpass ressources as Reads first, then writes, i.e. [R] 1, [R] 5, [W] 2, [W] 3
		std::vector<Copy> m_copies;
		std::vector<Read> m_reads;
		std::vector<Write> m_writes;
		std::vector<RenderResourceId> m_attachments;
		RenderResourceId m_depth = InvalidResourceId;
		RenderResourceId m_stencil = InvalidResourceId;
		
		struct PipelineBuild 
		{
			vulkan::GraphicsPipelineConfig config;
			vulkan::PipelineId* id;
		};

		std::vector< PipelineBuild> m_queuedPipelineBuilds;


		struct GlobalBarriers {
			std::vector<VkMemoryBarrier2> barriers;
		} m_globalBarriers2;

		size_t m_globalPostBarrierIndex{ 0 };
		size_t m_globalCopyBarrierIndex{ 0 };
		size_t m_globalPreBarrierIndex{ 0 };

		struct BufferBarriers {
			std::vector<RenderResourceId> buffers;
			std::vector<VkBufferMemoryBarrier2> barriers;
		} m_bufferBarriers2;

		size_t m_bufferPostBarrierIndex{ 0 };
		size_t m_bufferCopyBarrierIndex{ 0 };
		size_t m_bufferPreBarrierIndex{ 0 };

		struct ImageBarriers {
			std::vector<RenderResourceId> images;
			std::vector<VkImageMemoryBarrier2> barriers;
		} m_imageBarriers2;

		size_t m_imagePostBarrierIndex{ 0 };
		size_t m_imageCopyBarrierIndex{ 0 };
		size_t m_imagePreBarrierIndex{ 0 };


		VkRenderingInfo m_renderInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		};
		std::array<VkRenderingAttachmentInfo, vulkan::MAX_ATTACHMENTS> m_colorAttachments;
		VkRenderingAttachmentInfo m_depthAttachment;
		VkRenderingAttachmentInfo m_stencilAttachment;
		vulkan::RenderingCreateInfo m_renderingCreateInfo;

		struct Signal {
			uint32_t passId;
			VkPipelineStageFlags2 stage;
		};
		std::vector<VkSemaphoreSubmitInfo> m_waitInfos;
		std::vector<Signal> m_signals;

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
		Renderpass& add_pass(const entt::hashed_string& name, Renderpass::Type type);
		Renderpass& get_pass(const entt::hashed_string& name);
		void build();
		void execute();
		RenderResource& add_ressource(const entt::hashed_string& name, Attachment attachment);
		RenderResource& get_resource(const entt::hashed_string& name);
		bool resource_exists(const entt::hashed_string& name);
		const RenderResource& get_resource(const entt::hashed_string& name) const;
		void set_swapchain(const entt::hashed_string& name);
		vulkan::LogicalDevice& get_device() const;
	private:
		RenderResource& get_resource(RenderResourceId id);
		const RenderResource& get_resource(RenderResourceId id) const;
		void swapchain_present_transition(RenderpassId src_const);
		void set_up_transition(RenderpassId from, RenderpassId to, const RenderResource& resource);
		void set_up_first_transition(RenderpassId dst, const RenderResource& resource);
		void set_up_copy(RenderpassId dst, const RenderResource& resource);
		void update_render_resource(RenderResource& resource);
		void update_render_resource_image(RenderResource& resource);

		State m_state = State::Setup;
		vulkan::LogicalDevice& r_device;
		std::vector<Renderpass*> m_submissionOrder;
		Utility::NonInvalidatingMap<entt::hashed_string::hash_type, Renderpass> m_renderpasses;
		Utility::NonInvalidatingMap<entt::hashed_string::hash_type, RenderResource> m_renderresources;
		RenderResourceId m_swapchainResource = InvalidResourceId;
		RenderpassId m_renderpassCount = 0;
		RenderResourceId m_resourceCount = 0;
		RenderpassId m_lastCompute = invalidRenderpassId;
		RenderpassId m_lastGeneric = invalidRenderpassId;

		//RenderpassId m_lastPass { invalidRenderpassId };
	};
}

#endif !RDRENDERGRAPH_H