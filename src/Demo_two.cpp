
#include <iostream>

#include "Application.h"
#include "imgui.h"
#include "VkWrapper.h"
#include <glfwWrapper.h>
#include "ShaderManager.h"
#include "ImguiRenderer.h"
#include "Rendergraph.h"
#include <chrono>
#include <stb_image.h>
#include "LinAlg.h"
#include "Transform.h"
#include <new>
#include <thread>
using namespace std;
using namespace Math;



int main()
{

	/*
	unsigned int n = std::thread::hardware_concurrency();
	std::cout << n << " concurrent threads are supported.\n";
	std::cout << std::hardware_constructive_interference_size << " hardware_constructive_interference_size.\n";
	std::cout << std::hardware_destructive_interference_size << " hardware_destructive_interference_size .\n";
	*/
	nyan::Application application("Demo");
	try {

		auto start = chrono::high_resolution_clock::now();
		auto& device = application.get_device();
		auto& window = application.get_window();
		nyan::ImguiRenderer imgui(device);

		nyan::Rendergraph rendergraph(device);
		auto& pass = rendergraph.add_pass("test", nyan::Renderpass::Type::Graphics);
		rendergraph.build();

		
		window.configure_imgui();
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		vulkan::ImageInfo info = vulkan::ImageInfo::immutable_2d_image(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, false);
		auto tex = device.create_image(info, reinterpret_cast<char*>(pixels));
		stbi_image_free(pixels);

		vulkan::BufferInfo buffInfo;
		buffInfo.size = sizeof(vulkan::vertices);
		buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		auto vbo = device.create_buffer(buffInfo, vulkan::vertices.data());
		buffInfo.size = sizeof(vulkan::indices);
		buffInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		auto ibo = device.create_buffer(buffInfo, vulkan::indices.data());
		vulkan::Ubo ubo_data[2]{ {
			Math::mat44::identity(),
			Math::mat44::identity(),
			Math::mat44::identity()
		} ,{
			Math::mat44::identity(),
			Math::mat44::identity(),
			Math::mat44::identity()
		} };
		float x = 0;
		float y = 0;
		float z = 0;
		float fov = 90.f;
		float distance = 1.0f;
		float aspect = static_cast<float>(1080.f / 1920.f);
		ubo_data[0].model = Math::mat44(Math::mat33::rotation_matrix(x, y, z));
		ubo_data[0].model(3, 3) = 1;
		ubo_data[0].view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }), Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
		ubo_data[0].proj = Math::mat44::perspective(0.01f, 10.f, fov, aspect);
		ubo_data[1].model = Math::mat44(Math::mat33::rotation_matrix(x, y, z));
		ubo_data[1].model(3, 3) = 1;
		ubo_data[1].model = ubo_data[1].model * Math::mat44::translation_matrix(Math::vec3({ 0.0f, 0.0f, -0.5f }));
		ubo_data[1].view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }), Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
		ubo_data[1].proj = Math::mat44::perspective(0.01f, 10.f, fov, aspect);

		buffInfo.size = sizeof(vulkan::Ubo) * 2;
		buffInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		auto ubo = device.create_buffer(buffInfo, ubo_data);
		vulkan::ShaderManager shaderManager(device);
		auto program = shaderManager.request_program("basic_vert.spv", "basic_frag.spv");

		//device.create_stuff(tex->get_view()->get_image_view());
		uint64_t frame = 0;
		auto total = start - start;
		bool is_fullscreen_window = false;
		bool should_fullscreen_window = false;
		auto mapped = ubo->map_data();
		pass.add_renderfunction([&](vulkan::CommandBufferHandle& cmd) {
			cmd->begin_region("Test");
			cmd->bind_program(program);
			cmd->bind_index_buffer(vulkan::IndexState{ .buffer = ibo->get_handle(), .offset = 0,.indexType = VK_INDEX_TYPE_UINT16 });
			cmd->set_vertex_attribute(0, 0, vulkan::get_format<Math::vec3>());
			cmd->set_vertex_attribute(1, 0, vulkan::get_format<Math::vec3>());
			cmd->set_vertex_attribute(2, 0, vulkan::get_format<Math::vec2>());
			cmd->bind_vertex_buffer(0, *vbo, 0, VK_VERTEX_INPUT_RATE_VERTEX);
			cmd->bind_uniform_buffer(0, 0, *ubo, 0, 256);
			cmd->bind_texture(0, 1, *tex->get_view(), vulkan::DefaultSampler::TrilinearClamp);
			cmd->draw_indexed(static_cast<uint32_t>(vulkan::indices.size()), 1, 0, 0, 0);
			//cmd->bind_uniform_buffer(0, 0, *ubo, 256, 256);
			//cmd->draw_indexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
			cmd->end_region();
		});

		std::chrono::duration<float, std::milli> delta = std::chrono::high_resolution_clock::now() - start;
		start = chrono::steady_clock::now();
		std::cout << "Demo setup took: " << delta.count() << "ms\n";
		while (!window.should_close())
		{
			delta = std::chrono::high_resolution_clock::now() - start;
			start = chrono::steady_clock::now();
			//window.swap_buffers();
			glfwPollEvents();
			window.imgui_update_mouse_keyboard();
			if (should_fullscreen_window != is_fullscreen_window) {
				is_fullscreen_window = should_fullscreen_window;
				if (is_fullscreen_window)
					window.change_mode(glfww::WindowMode::FullscreenWindowed);
				else
					window.change_mode(glfww::WindowMode::Windowed);
			}
			if (!window.is_iconified()) {
				ubo_data[0].model = Math::mat44(Math::mat33::rotation_matrix(x, y, z));
				ubo_data[0].model(3, 3) = 1;
				ubo_data[1].model = Math::mat44(Math::mat33::rotation_matrix(x, y, z));
				ubo_data[1].model(3, 3) = 1;
				ubo_data[1].model = ubo_data[1].model * Math::mat44::eye(0.2f) * Math::mat44::translation_matrix(Math::vec3({ 0.0f, 0.0f, -0.5f }));
				ubo_data[0].proj = Math::mat44::perspective(0.01f, 100.f, fov, aspect);
				ubo_data[0].view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }) * distance, Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
				ubo_data[1].view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }) * distance, Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
				ubo_data[1].proj = Math::mat44::perspective(0.01f, 100.f, fov, aspect);
				{
					std::memcpy(mapped, ubo_data, sizeof(ubo_data));
				}
				application.next_frame();
				imgui.next_frame();
				//device.update_uniform_buffer();
				ImGui::Begin("Interaction");
				ImGui::SliderFloat("x_rotation", &x, 0.0f, 360.0f);
				ImGui::SliderFloat("y_rotation", &y, 0.0f, 360.0f);
				ImGui::SliderFloat("z_rotation", &z, 0.0f, 360.0f);
				ImGui::SliderFloat("distance", &distance, 0.0f, 10.f);
				ImGui::SliderFloat("fov", &fov, 45.f, 110.0f);
				ImGui::Checkbox("Fullscreen Windowed", &should_fullscreen_window);
				ImGui::End();
				auto rp = device.request_swapchain_render_pass(vulkan::SwapchainRenderpassType::Depth);
				pass.set_render_pass(rp);
				//auto cmd = device.request_command_buffer(vulkan::CommandBuffer::Type::Generic);
				//cmd->begin_render_pass(rp);
				
				//cmd->begin_region("Test");
				//cmd->bind_program(program);
				//cmd->bind_index_buffer(vulkan::IndexState{ .buffer = ibo->get_handle(), .offset = 0,.indexType = VK_INDEX_TYPE_UINT16 });
				//cmd->set_vertex_attribute(0, 0, vulkan::get_format<Math::vec3>());
				//cmd->set_vertex_attribute(1, 0, vulkan::get_format<Math::vec3>());
				//cmd->set_vertex_attribute(2, 0, vulkan::get_format<Math::vec2>());
				//cmd->bind_vertex_buffer(0, *vbo, 0, VK_VERTEX_INPUT_RATE_VERTEX);
				//cmd->bind_uniform_buffer(0, 0, *ubo, 0, 256);
				//cmd->bind_texture(0, 1, *tex->get_view(), vulkan::DefaultSampler::TrilinearClamp);
				//cmd->draw_indexed(static_cast<uint32_t>(vulkan::indices.size()), 1, 0, 0, 0);
				////cmd->bind_uniform_buffer(0, 0, *ubo, 256, 256);
				////cmd->draw_indexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				//cmd->end_region();
				//cmd->end_render_pass();
				//device.submit(cmd, 0, nullptr);
				rendergraph.execute();
				imgui.end_frame();
				application.end_frame();
				frame++;
				total += chrono::high_resolution_clock::now() - start;
			}
		}
		auto end = chrono::steady_clock::now();
		std::cout << "Average: " << chrono::duration_cast<chrono::microseconds>(total).count() / frame << "microseconds\n";
		device.wait_idle();
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}