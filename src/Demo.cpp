
#include <iostream>

#include "imgui.h"
#include "VkWrapper.h"
#include <glfwWrapper.h>
#include "ShaderManager.h"
#include "ImguiRenderer.h"
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

	int width = 1920, height = 1080;
	std::string applicationName{ "Demo" };
	std::string engineName{ "NYAN" };
	try {
		glfww::Library library;
		glfww::Window window(width, height, nullptr, nullptr, applicationName.c_str());
		
		//auto window = glfww::Window::create_full_screen(width, height);
		//auto window = glfww::Window::create_full_screen(width, height);
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		Vulkan::Instance instance(glfwExtensions, glfwExtensionCount, applicationName, engineName);
		instance.setup_win32_surface(window.get_win32_window(), GetModuleHandle(nullptr));

		Vulkan::LogicalDevice device = instance.setup_device();
		Vulkan::WindowSystemInterface wsi(device, instance);
		
		Vulkan::Imgui imgui(device);
		window.configure_imgui();
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		Vulkan::ImageInfo info = Vulkan::ImageInfo::immutable_2d_image(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, false);
		auto tex = device.create_image(info, reinterpret_cast<char*>(pixels));
		stbi_image_free(pixels);

		Vulkan::BufferInfo buffInfo;
		buffInfo.size = sizeof(Vulkan::vertices);
		buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		auto vbo = device.create_buffer(buffInfo, Vulkan::vertices.data());
		buffInfo.size = sizeof(Vulkan::indices);
		buffInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		auto ibo = device.create_buffer(buffInfo, Vulkan::indices.data());
		Vulkan::Ubo ubo_data[2]{ {
			Math::mat44::identity(),
			Math::mat44::identity(),
			Math::mat44::identity()
		} ,{
			Math::mat44::identity(),
			Math::mat44::identity(),
			Math::mat44::identity()
		} };
		float degrees = 0;
		ubo_data[0].model = Math::mat44(Math::mat33::rotation_matrix(0, 0, (degrees)));
		ubo_data[0].model(3, 3) = 1;
		ubo_data[0].view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }), Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
		ubo_data[0].proj = Math::mat44::perspective(0.01f, 10.f, 45.0f, static_cast<float>(1920.f / 1080.f));
		ubo_data[1].model = Math::mat44(Math::mat33::rotation_matrix(0, 0, (degrees)));
		ubo_data[1].model(3, 3) = 1;
		ubo_data[1].model = ubo_data[1].model * Math::mat44::translation_matrix(Math::vec3({ 0.0f, 0.0f, -0.5f }));
		ubo_data[1].view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }), Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
		ubo_data[1].proj = Math::mat44::perspective(0.01f, 10.f, 45.0f, static_cast<float>(1920.f / 1080.f));

		buffInfo.size = sizeof(Vulkan::Ubo) * 2;
		buffInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		auto ubo = device.create_buffer(buffInfo, ubo_data);
		Vulkan::ShaderManager shaderManager(device);
		auto program = shaderManager.request_program("basic_vert.spv", "basic_frag.spv");

		//device.create_stuff(tex->get_view()->get_image_view());
		uint64_t frame = 0;
		auto start = chrono::high_resolution_clock::now();
		auto total = start - start;

		while (!window.should_close())
		{
			std::chrono::duration<float, std::milli> delta = std::chrono::high_resolution_clock::now() - start;
			start = chrono::steady_clock::now();
			//window.swap_buffers();
			glfwPollEvents();
			window.imgui_update_mouse_keyboard();
			if (!window.is_iconified()) {
				ubo_data[0].model = Math::mat44(Math::mat33::rotation_matrix(0, 0, (degrees)));
				ubo_data[0].model(3, 3) = 1;
				ubo_data[1].model = Math::mat44(Math::mat33::rotation_matrix(0, 0, (degrees)));
				ubo_data[1].model(3, 3) = 1;
				ubo_data[1].model = ubo_data[1].model * Math::mat44::translation_matrix(Math::vec3({ 0.0f, 0.0f, -0.5f }));
				{
					auto mapped = ubo->map_data();
					std::memcpy(mapped.get(), ubo_data, sizeof(ubo_data));
				}
				wsi.begin_frame();
				imgui.next_frame();
				//device.update_uniform_buffer();
				ImGui::Begin("Interaction");
				ImGui::SliderFloat("rotation", &degrees, 0.0f, 360.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::End();
				auto rp = device.request_swapchain_render_pass();
				auto cmd = device.request_command_buffer(Vulkan::CommandBuffer::Type::Generic);
				cmd->begin_render_pass(rp);
				cmd->bind_program(program);
				cmd->bind_index_buffer(Vulkan::IndexState{ .buffer = ibo->get_handle(), .offset = 0,.indexType = VK_INDEX_TYPE_UINT16 });
				cmd->set_vertex_attribute(0, 0, Vulkan::get_format<Math::vec3>());
				cmd->set_vertex_attribute(1, 0, Vulkan::get_format<Math::vec3>());
				cmd->set_vertex_attribute(2, 0, Vulkan::get_format<Math::vec2>());
				cmd->bind_vertex_buffer(0, *vbo, 0, VK_VERTEX_INPUT_RATE_VERTEX);
				cmd->bind_uniform_buffer(0, 0, *ubo, 0, 256);
				cmd->bind_texture(0, 1, *tex->get_view(), Vulkan::DefaultSampler::TrilinearClamp);
				cmd->draw_indexed(static_cast<uint32_t>(Vulkan::indices.size()), 1, 0, 0, 0);
				cmd->bind_uniform_buffer(0, 0, *ubo, 256, 256);
				cmd->draw_indexed(static_cast<uint32_t>(Vulkan::indices.size()), 1, 0, 0, 0);
				imgui.end_frame(cmd);
				cmd->end_render_pass();
				device.submit(cmd, 0, nullptr);
				wsi.end_frame();
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