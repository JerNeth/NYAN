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
	constexpr RenderResourceId InvalidResourceId = -1;
	struct RenderResource {
		enum class Type : uint8_t {
			Image,
			Buffer
		};
		RenderResource() = default;
		RenderResource(RenderResourceId id) : m_id(id){}
		Type m_type = Type::Image;
		RenderResourceId m_id = InvalidResourceId;
		std::set<uint32_t> m_readIn;
		std::set<uint32_t> m_writeToIn;
		Attachment attachment;
		vulkan::ImageView* handle = nullptr;
	};
	struct Barrier {
		VkPipelineStageFlags src = 0;
		VkPipelineStageFlags dst = 0;
		uint32_t bufferBarrierCount = 0;
		uint32_t bufferBarrierOffset = 0;
		uint32_t imageBarrierCount = 0;
		uint32_t imageBarrierOffset = 0;

	};
	class Rendergraph;
	using RenderpassId = uint32_t;
	class Renderpass {
		friend class Rendergraph;
	public:
		enum class Type :uint8_t{
			Graphics,
			Compute
		};
		Renderpass(Rendergraph& graph, Type type, uint32_t id, const std::string& name);
		Renderpass(const Renderpass&) = delete;
		Renderpass(Renderpass&&) = default;
		Renderpass& operator=(const Renderpass&) = delete;
		Renderpass& operator=(Renderpass&&) = default;
		void add_input(const std::string& name);
		void add_output(const std::string& name, ImageAttachment attachment);
		void add_depth_input(const std::string& name);
		void add_depth_output(const std::string& name, ImageAttachment attachment);
		void add_read_dependency(const std::string& name);
		void add_write_dependency(const std::string& name);
		void add_renderfunction(const std::function<void(vulkan::CommandBufferHandle&)>& functor) {
			m_renderFunction = functor;
		}
		uint32_t get_id() const noexcept {
			return m_id;
		}
		Type get_type() const noexcept {
			return m_type;
		}
		vulkan::RenderpassCreateInfo& get_info() const noexcept {
			assert(m_rpInfo);
			return *m_rpInfo;
		}
		void execute(vulkan::CommandBufferHandle& cmd) {
			m_renderFunction(cmd);
		}
		void set_render_pass(vulkan::RenderpassCreateInfo info) {
			m_rpInfo = std::make_unique<vulkan::RenderpassCreateInfo>(info);
		}
		bool has_post_barriers() const noexcept {
			return !m_postBufferBarriers.empty() || !m_postImageBarriers.empty();
		}
		void add_post_barrier(const std::string& name);
		void apply_post_barriers(vulkan::CommandBufferHandle& cmd);
	private:
		Rendergraph& r_graph;
		std::string m_name;
		Type m_type;
		uint32_t m_id;
		std::function<void(vulkan::CommandBufferHandle&)> m_renderFunction;
		//Order Renderpass as Reads first, then writes, i.e. [R] 1, [R] 5, [W] 2, [W] 3
		std::set<RenderResourceId> m_reads;
		std::set<RenderResourceId> m_writes;
		RenderResourceId m_depthStencilRead = InvalidResourceId;
		RenderResourceId m_depthStencilWrite = InvalidResourceId;
		std::unique_ptr<vulkan::RenderpassCreateInfo> m_rpInfo;
		std::vector<Barrier> m_postBarriers;
		std::vector<VkImageMemoryBarrier> m_postImageBarriers;
		std::vector<RenderResourceId> m_postResource;
		std::vector<VkBufferMemoryBarrier> m_postBufferBarriers;
	};
	class Rendergraph {
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
		void add_ressource(const std::string& name, Attachment attachment);
		RenderResource& get_resource(const std::string& name);
		void set_swapchain(const std::string& name);
	private:
		void set_up_RaW(RenderpassId write, RenderpassId read, const RenderResource& resource);
		void set_up_WaW(RenderpassId src_, RenderpassId dst_, const RenderResource& resource);

		State m_state = State::Setup;
		vulkan::LogicalDevice& r_device;
		Utility::NonInvalidatingMap<std::string, Renderpass> m_renderpasses;
		Utility::NonInvalidatingMap<std::string, RenderResource> m_renderresources;
		RenderResourceId m_swapchainResource = InvalidResourceId;
		RenderpassId m_renderpassCount = 0;
		RenderResourceId m_resourceCount = 0;
	};
}

#endif !RDRENDERGRAPH_H