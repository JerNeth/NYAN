#pragma once
#ifndef RDIMGUI_H
#define RDIMGUI_H
#include "VkWrapper.h"
#include "ShaderManager.h"
#include "imgui.h"
#include "Renderer.h"
#include "RenderGraph.h"
#include "glfwWrapper.h"
#include <chrono>
#include "MeshManager.h"
#include "entt/fwd.hpp"
#include "RenderManager.h"
#include "imgui_entt_entity_editor.hpp"

namespace nyan {
	class ImguiRenderer{
	public:
		ImguiRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& shaderManager, nyan::Renderpass& pass, glfww::Window* window);
		~ImguiRenderer();
		void update(std::chrono::nanoseconds dt);
		void next_frame();
		void end_frame();
	private:
		void create_cmds(ImDrawData* draw_data, vulkan::CommandBuffer& cmd);
		void prep_buffer(ImDrawData* draw_data);
		void set_up_pipeline(vulkan::ShaderManager& shaderManager, nyan::Renderpass& pass);
		void set_up_font();
		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		MM::EntityEditor<entt::entity> m_editor;
		entt::entity m_entity;
		vulkan::PipelineId m_pipeline;
		glfww::Window* ptr_window;
		std::chrono::high_resolution_clock::time_point start;

		std::optional < vulkan::ImageHandle> m_font;
		uint32_t m_fontBind;
		std::optional < vulkan::BufferHandle> m_dataBuffer;
		std::array<VkDeviceSize, 4> m_bufferOffsets;
		float values[230] = {};
		int values_offset = 0;
		//BufferHandle m_ubo;
	};
}

#endif !RDIMGUI_H