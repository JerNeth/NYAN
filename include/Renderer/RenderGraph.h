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
	//using RenderResourceId = uint32_t;




	struct RenderResource {
		struct Id
		{
			using Type = uint32_t;
			Id() : id(std::numeric_limits<uint32_t>::max()) {}
			constexpr explicit Id(Type id) :id(id) {}
			constexpr operator Type() const noexcept { return id; }
			Type id;
		};
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
		RenderResource(Id id) : m_id(id){}
		Type m_type = Type::Image;
		Id m_id {};
		std::string name;
		std::vector<Utility::bitset<static_cast<size_t>(ImageUse::Size), ImageUse>> m_uses;
		//Utility::bitset<static_cast<size_t>(ImageUse::Size), ImageUse> totalUses;
		Attachment attachment;
		vulkan::Image* handle = nullptr;
	};
	static constexpr RenderResource::Id InvalidResourceId{ std::numeric_limits<uint32_t>::max() };
		


	class Renderpass {
	private:
		friend class Rendergraph;
	public:
		struct Id
		{
			using Type = uint32_t;
			Id() : id(std::numeric_limits<uint32_t>::max()) {}
			constexpr explicit Id(Type id) :id(id) {}
			constexpr operator Type() const noexcept { return id; }
			Type id;
		};
		struct Read {
			enum class Type {
				ImageColor,
				ImageDepth,
				ImageStencil,
			};
			RenderResource::Id id;
			Type type;
			VkImageView view;
			uint32_t binding{ std::numeric_limits<uint32_t>::max() };
		};
		struct Write {
			enum class Type {
				Graphics,
				Compute
			};
			RenderResource::Id id;
			Type type;
			VkImageView view;
			uint32_t binding{ std::numeric_limits<uint32_t>::max() };
		};
		struct Copy {
			RenderResource::Id src;
			RenderResource::Id dst;
		};
	public:
		enum class Type : uint8_t {
			Generic,
			AsyncCompute,
			Transfer
		};
		Renderpass(Rendergraph& graph, Renderpass::Type type, Renderpass::Id id, const std::string& name);
		Renderpass(const Renderpass&) = delete;
		Renderpass(Renderpass&&) = default;
		Renderpass& operator=(const Renderpass&) = delete;
		Renderpass& operator=(Renderpass&&) = default;

		void add_read(RenderResource::Id id, Renderpass::Read::Type readType = Read::Type::ImageColor);
		void add_attachment(RenderResource::Id id, bool clear = false);
		void add_swapchain_attachment(Math::vec4 clearColor = Math::vec4{}, bool clear = false);
		void add_depth_attachment(RenderResource::Id id, bool clear = false);
		void add_depth_stencil_attachment(RenderResource::Id id, bool clear = false);
		void add_stencil_attachment(RenderResource::Id id, bool clear = false);
		void add_write(RenderResource::Id id, Renderpass::Write::Type writeType = Write::Type::Graphics, bool clear = false);
		void add_swapchain_write(Math::vec4 clearColor = Math::vec4{}, Renderpass::Write::Type writeType = Write::Type::Graphics, bool clear = false);
			//void add_read_dependency(const std::string& name, bool storageImage = false);
		void add_renderfunction(const std::function<void(vulkan::CommandBuffer&, Renderpass&) > & functor, bool renderpass) {
			m_renderFunctions.push_back(functor);
			m_useRendering.push_back(renderpass);
		}
		void copy(RenderResource::Id source, RenderResource::Id target);
		Renderpass::Id get_id() const noexcept {
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
		void add_pre_barrier(const VkImageMemoryBarrier2& barrier, RenderResource::Id image);
		void add_copy_barrier(const VkImageMemoryBarrier2& barrier, RenderResource::Id image);
		void add_post_barrier(const VkImageMemoryBarrier2& barrier, RenderResource::Id image);
		void add_pre_barrier(const VkMemoryBarrier2& barrier);
		void add_copy_barrier(const VkMemoryBarrier2& barrier);
		void add_post_barrier(const VkMemoryBarrier2& barrier);


		void add_pipeline(vulkan::GraphicsPipelineConfig config, vulkan::PipelineId* id);
		void begin_rendering(vulkan::CommandBuffer& cmd);
		void end_rendering(vulkan::CommandBuffer& cmd);
		uint32_t get_write_bind(uint32_t idx);
		uint32_t get_read_bind(uint32_t idx);
		uint32_t get_write_bind(RenderResource::Id id, Write::Type type = Write::Type::Graphics);
		uint32_t get_read_bind(RenderResource::Id id, Read::Type type = Read::Type::ImageColor);
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
		bool is_read(RenderResource::Id id) const;
		bool is_write(RenderResource::Id id) const;
		bool is_compute_write(RenderResource::Id id) const;
		bool is_attachment(RenderResource::Id id) const;
		bool is_write(const RenderResource& resource) const;
		bool is_compute_write(const RenderResource& resource) const;
		bool is_attachment(const RenderResource& resource) const;


		Rendergraph& r_graph;
		entt::hashed_string m_name;
		Type m_type;
		Renderpass::Id m_id;
		std::vector<std::function<void(vulkan::CommandBuffer&, Renderpass&)>> m_renderFunctions;
		std::vector<bool> m_useRendering;
		bool m_rendersSwap = false;
		//Order Renderpass ressources as Reads first, then writes, i.e. [R] 1, [R] 5, [W] 2, [W] 3
		std::vector<Copy> m_copies;
		std::vector<Read> m_reads;
		std::vector<Write> m_writes;
		std::vector<RenderResource::Id> m_attachments;
		RenderResource::Id m_depth = InvalidResourceId;
		RenderResource::Id m_stencil = InvalidResourceId;
		
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
			std::vector<RenderResource::Id> buffers;
			std::vector<VkBufferMemoryBarrier2> barriers;
		} m_bufferBarriers2;

		size_t m_bufferPostBarrierIndex{ 0 };
		size_t m_bufferCopyBarrierIndex{ 0 };
		size_t m_bufferPreBarrierIndex{ 0 };

		struct ImageBarriers {
			std::vector<RenderResource::Id> images;
			std::vector<VkImageMemoryBarrier2> barriers;
		} m_imageBarriers2;

		size_t m_imagePostBarrierIndex{ 0 };
		size_t m_imageCopyBarrierIndex{ 0 };
		size_t m_imagePreBarrierIndex{ 0 };


		VkRenderingInfo m_renderInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		};
		std::array<VkRenderingAttachmentInfo, vulkan::MAX_ATTACHMENTS> m_colorAttachments{};
		VkRenderingAttachmentInfo m_depthAttachment{};
		VkRenderingAttachmentInfo m_stencilAttachment{};
		vulkan::RenderingCreateInfo m_renderingCreateInfo{};

		struct Signal {
			uint32_t passId;
			VkPipelineStageFlags2 stage;
		};
		std::vector<VkSemaphoreSubmitInfo> m_waitInfos{};
		std::vector<Signal> m_signals{};

	};
	static constexpr Renderpass::Id InvalidRenderpassId{ std::numeric_limits<uint32_t>::max() };

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
		Renderpass::Id add_pass(const std::string& name, Renderpass::Type type);
		Renderpass& get_pass(const entt::hashed_string& name);
		void build();
		void execute();
		//RenderResource& add_ressource(const entt::hashed_string& name, Attachment attachment);
		//RenderResource& get_resource(const entt::hashed_string& name);
		//bool resource_exists(const entt::hashed_string& name);
		RenderResource::Id add_ressource(const std::string& name, Attachment attachment);
		RenderResource::Id add_ressource(Attachment attachment);
		const RenderResource& get_resource(RenderResource::Id id) const;
		RenderResource& get_resource(RenderResource::Id id);
		bool resource_exists(RenderResource::Id id);
		vulkan::LogicalDevice& get_device() const;
	private:
		void swapchain_present_transition(Renderpass::Id src_const);
		void set_up_transition(Renderpass::Id from, Renderpass::Id to, const RenderResource& resource);
		void set_up_first_transition(Renderpass::Id dst, const RenderResource& resource);
		void set_up_copy(Renderpass::Id dst, const RenderResource& resource);
		void update_render_resource(RenderResource& resource);
		void update_render_resource_image(RenderResource& resource);

		State m_state = State::Setup;
		vulkan::LogicalDevice& r_device;
		std::vector<Renderpass*> m_submissionOrder;
		//Utility::NonInvalidatingMap<entt::hashed_string::hash_type, Renderpass> m_renderpasses;
		//Utility::NonInvalidatingMap<entt::hashed_string::hash_type, RenderResource> m_renderresources;
		Utility::NonInvalidatingMap<Renderpass::Id, Renderpass> m_renderpasses;
		Utility::NonInvalidatingMap<RenderResource::Id, RenderResource> m_renderresources;
		RenderResource::Id m_swapchainResource{};
		Renderpass::Id::Type m_renderpassCount{ 0 };
		RenderResource::Id::Type m_resourceCount{ 0 };
		Renderpass::Id m_lastCompute {};
		Renderpass::Id m_lastGeneric {};

		//RenderpassId m_lastPass { invalidRenderpassId };
	};
}

#endif !RDRENDERGRAPH_H