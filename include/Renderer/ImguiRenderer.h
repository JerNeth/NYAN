#ifndef RDIMGUI_H
#define RDIMGUI_H
#pragma once
#include "VkWrapper.h"
#include "ShaderManager.h"
#include "imgui.h"
#include "Renderer.h"
#include <chrono>
namespace nyan {
	class ImguiRenderer : public Renderer {
	public:
		ImguiRenderer(vulkan::LogicalDevice& device, vulkan::ShaderManager& shaderManager);
		~ImguiRenderer();
		void next_frame();
		void end_frame();
	private:
		void create_cmds(ImDrawData* draw_data, vulkan::CommandBufferHandle& cmd);
		void prep_buffer(ImDrawData* draw_data);
		void set_up_program(vulkan::ShaderManager& shaderManager);
		void set_up_font();
		vulkan::LogicalDevice& r_device;
		vulkan::Program* m_program;
		std::chrono::high_resolution_clock::time_point start;

		std::optional < vulkan::ImageHandle> m_font;
		std::optional < vulkan::BufferHandle> m_vbo;
		std::optional<vulkan::BufferHandle> m_ibo;
		float values[230] = {};
		int values_offset = 0;
		//BufferHandle m_ubo;
	};
}

#endif !RDIMGUI_H