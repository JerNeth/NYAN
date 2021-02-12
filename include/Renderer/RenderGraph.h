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
		Size size = Size::Swapchain;
		float width = 1.0f;
		float height = 1.0f;
		VkFormat format = VK_FORMAT_UNDEFINED;
	};
	struct BufferAttachment {
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
	};
	using Attachment = std::variant<ImageAttachment, BufferAttachment>;
	using RenderRessourceId = uint32_t;
	struct RenderRessource {
		RenderRessourceId m_id;
		std::set<uint32_t> m_readIn;
		std::set<uint32_t> m_writeToIn;
		Attachment attachment;
	};
	class Rendergraph;
	class Renderpass {
	public:
		enum class Type :uint8_t{
			Graphics,
			Compute
		};
		Renderpass(Rendergraph& graph, Type type, uint32_t id);
		Renderpass(const Renderpass&) = delete;
		Renderpass(Renderpass&&) = default;
		Renderpass& operator=(const Renderpass&) = delete;
		Renderpass& operator=(Renderpass&&) = default;
		void add_input(const std::string& name);
		void add_output(const std::string& name, ImageAttachment attachment);
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
	private:
		Rendergraph& r_graph;
		Type m_type;
		uint32_t m_id;
		std::function<void(vulkan::CommandBufferHandle&)> m_renderFunction;
		std::set<RenderRessourceId> m_reads;
		std::set<RenderRessourceId> m_writes;
		std::unique_ptr<vulkan::RenderpassCreateInfo> m_rpInfo;
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
		RenderRessource& get_ressource(const std::string& name);
		void set_swapchain(const std::string& name);
	private:
		vulkan::LogicalDevice& r_device;
		Utility::NonInvalidatingMap<std::string, Renderpass> m_renderpasses;
		Utility::NonInvalidatingMap<std::string, RenderRessource> m_renderressources;
		std::string m_swapchainRessource;
		uint32_t m_renderpassCount = 0;
	};
}

#endif !RDRENDERGRAPH_H