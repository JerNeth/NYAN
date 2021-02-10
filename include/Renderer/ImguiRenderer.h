#ifndef RDIMGUI_H
#define RDIMGUI_H
#pragma once
#include "VkWrapper.h"
#include "imgui.h"
#include <chrono>
namespace Vulkan {
	class Imgui {
	public:
		Imgui(LogicalDevice& device);
		~Imgui();
		void next_frame();
		void end_frame(CommandBufferHandle& cmd);
	private:
		void create_cmds(ImDrawData* draw_data, CommandBufferHandle& cmd);
		void prep_buffer(ImDrawData* draw_data);
		void set_up_program();
		void set_up_font();
		LogicalDevice& r_device;
		Program* m_program;
		std::chrono::high_resolution_clock::time_point start;

		ImageHandle m_font;
		BufferHandle m_vbo;
		BufferHandle m_ibo;
		float values[230] = {};
		int values_offset = 0;
		//BufferHandle m_ubo;
	};
}

#endif !RDIMGUI_H