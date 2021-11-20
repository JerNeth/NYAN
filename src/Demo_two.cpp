
#include <iostream>

#include "Application.h"
#include "imgui.h"
#include "VkWrapper.h"
#include <glfwWrapper.h>
#include "VulkanRenderer"
#include <chrono>
#include "LinAlg.h"
#include <new>
#include <thread>
using namespace std;
using namespace Math;
using namespace vulkan;
using namespace nyan;
using namespace Utility;



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
		vulkan::ShaderManager shaderManager(device);
		nyan::ImguiRenderer imgui(device, shaderManager);
		window.configure_imgui();
		nyan::VulkanRenderer renderer(device, &shaderManager);
		nyan::Rendergraph rendergraph(device);
		nyan::TextureManager textureManager(device, false);
		nyan::MeshManager meshManager(device);
		application.add_renderer(&renderer);
		application.add_renderer(&imgui);
		bool wireframe = false;

		
		auto& pass = rendergraph.add_pass("test", nyan::Renderpass::Type::Graphics);
		nyan::ImageAttachment depth;
		depth.clearColor[0] = 1.0f;
		depth.format = VK_FORMAT_D16_UNORM;
		pass.add_depth_output("depth", depth);
		nyan::ImageAttachment color;
		color.format = VK_FORMAT_R8G8B8A8_UNORM;
		color.clearColor = Math::vec4({ .2f, .3f, .1f, 0.f });
		pass.add_output("color", color);
		auto& compute = rendergraph.add_pass("ScreenPass", nyan::Renderpass::Type::Compute);
		nyan::ImageAttachment swap;
		rendergraph.add_ressource("swap", swap);
		compute.add_read_dependency("color", true);
		compute.add_write_dependency("swap", true);
		rendergraph.set_swapchain("swap");
		rendergraph.build();

		auto& attachment = std::get<nyan::ImageAttachment>(rendergraph.get_resource("color").attachment);

		//auto* fullscreenProgram = shaderManager.request_program("fullscreen_vert", "fullscreen_frag");
		/*pass2.add_renderfunction([&](vulkan::CommandBufferHandle& cmd) {
			cmd->bind_program(fullscreenProgram);
			cmd->disable_depth();
			cmd->set_cull_mode(VK_CULL_MODE_NONE);
			cmd->bind_input_attachment(0, 0);
			cmd->draw(3, 1, 0, 0);
		});*/

		auto* computeProgram = shaderManager.request_program("test_comp");
		compute.add_renderfunction([&](vulkan::CommandBufferHandle& cmd) {
			auto& in = rendergraph.get_resource("color");
			auto& out = rendergraph.get_resource("swap");
			cmd->image_barrier(*in.handle->get_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0);
			cmd->image_barrier(*out.handle->get_image(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0);

			cmd->bind_program(computeProgram);
			cmd->bind_storage_image(0, 0, *in.handle);
			cmd->bind_storage_image(0, 1, *out.handle);
			cmd->dispatch(1920 / 8, 1080 / 8, 1);
			cmd->image_barrier(*in.handle->get_image(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0);
			cmd->image_barrier(*out.handle->get_image(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0);

		});
		
		Material testMaterial(0, "default_frag");
		StaticMesh* testMesh = meshManager.request_static_mesh("TestMesh");
		testMesh->set_material(&testMaterial);

		testMaterial.add_texture(textureManager.request_texture("grass"));

		RendererCamera camera{};
		Transform& transform = testMesh->get_transform();
		float x = 0;
		float y = 0;
		float z = 0;
		float fov = 90.f;
		float distance = 1.0f;
		float aspect = application.get_height() / static_cast<float>(application.get_width());
		transform.transform = Math::mat44(Math::mat33::rotation_matrix(x, y, z));
		//std::cout<< transform.transform.convert_to_string() << '\n';
		transform.transform(3, 3) = 1;
		camera.view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }), Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
		camera.proj = Math::mat44::perspectiveX(1.f, 10.f, fov, aspect);

		//device.create_stuff(tex->get_view()->get_image_view());
		uint64_t frame = 0;
		auto total = start - start;
		bool is_fullscreen_window = false;
		bool should_fullscreen_window = false;
		int mipLevel = 2;

		pass.add_renderfunction([&](vulkan::CommandBufferHandle& cmd) {
			if (wireframe)
				cmd->set_polygon_mode(VK_POLYGON_MODE_LINE);
			cmd->begin_region("Renderer");
			renderer.render(cmd);
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
				aspect = application.get_height() / static_cast<float>(application.get_width());
				transform.transform = Math::mat44(Math::mat33::rotation_matrix(x, y, z));
				transform.transform(3, 3) = 1;
				camera.proj = Math::mat44::perspectiveX(1.f, 10.f, fov, aspect);
				camera.view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }) * distance, Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
				renderer.update_camera(camera);
				application.next_frame();
				renderer.queue_mesh(testMesh);

				ImGui::Begin("Interaction");
				ImGui::ColorEdit4("Clearcolor", &attachment.clearColor[0]);
				ImGui::SliderFloat("x_rotation", &x, 0.0f, 360.0f);
				ImGui::SliderFloat("y_rotation", &y, 0.0f, 360.0f);
				ImGui::SliderFloat("z_rotation", &z, 0.0f, 360.0f);
				ImGui::SliderFloat("distance", &distance, 0.0f, 10.f);
				ImGui::SliderFloat("fov", &fov, 45.f, 110.0f);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Hold to repeat:");
				ImGui::SameLine();

				// Arrow buttons with Repeater
				float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
				ImGui::PushButtonRepeat(true);
				if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
					if (mipLevel) {
						mipLevel--; textureManager.change_mip("grass", mipLevel);
						
					}
				}
				ImGui::SameLine(0.0f, spacing);
				if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { 
					mipLevel++; 
					textureManager.change_mip("grass", mipLevel);
				}
				ImGui::PopButtonRepeat();
				ImGui::SameLine();
				ImGui::Text("%d", mipLevel);

				ImGui::Checkbox("Fullscreen Windowed", &should_fullscreen_window);
				ImGui::Checkbox("Wireframe", &wireframe);
				ImGui::End();

				//auto rp = device.request_swapchain_render_pass(vulkan::SwapchainRenderpassType::Depth);
				//pass.set_render_pass(rp);
				rendergraph.execute();
				//imgui.end_frame();
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