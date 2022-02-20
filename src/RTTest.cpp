#include <iostream>
#include <Util>
#include "Application.h"
using namespace nyan;
int main() {

	nyan::Application application("Demo");
	//try {

		auto& device = application.get_device();
		auto& window = application.get_window();
		vulkan::ShaderManager shaderManager(device);
		nyan::ImguiRenderer imgui(device, shaderManager);
		window.configure_imgui();
		nyan::Rendergraph rendergraph(device);
		nyan::TextureManager textureManager(device, false);
		nyan::MeshManager meshManager(device);
		application.add_renderer(&imgui);

		auto& pass = rendergraph.add_pass("test", nyan::Renderpass::Type::Graphics);
		nyan::ImageAttachment depth;
		depth.clearColor[0] = 1.0f;
		//depth.format = VK_FORMAT_D24_UNORM_S8_UINT;
		//depth.format = VK_FORMAT_D16_UNORM;
		depth.format = VK_FORMAT_D32_SFLOAT;
		pass.add_depth_output("depth", depth);
		nyan::ImageAttachment color;
		//color.format = VK_FORMAT_R8G8B8A8_UNORM;
		color.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		//color.format = VK_FORMAT_R8G8B8A8_SRGB;
		color.clearColor = Math::vec4(.4f, .3f, .7f, 0.f);

		nyan::ImageAttachment normal;
		normal.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		normal.clearColor = Math::vec4({ 0.f, 0.f, 1.f, 0.f});
		pass.add_output("color", color);
		auto& ScreenPass = rendergraph.add_pass("ScreenPass", nyan::Renderpass::Type::Graphics);
		ScreenPass.add_input("color");
		//ScreenPass.add_read_dependency("depth");
		ScreenPass.add_input("depth");
		ScreenPass.add_swapchain_output();
		rendergraph.build();

		auto& attachment = std::get<nyan::ImageAttachment>(rendergraph.get_resource("color").attachment);

		//auto* fullscreenProgram = shaderManager.request_program("fullscreen_vert", "fullscreen_frag");
		//auto* fullscreenProgram = shaderManager.request_program("fullscreen_vert", "deferredPBR_frag");
		auto* fullscreenProgram = shaderManager.request_program("fullscreen_vert", "fullscreen_frag");
		struct ShaderStuff {
			Math::vec4 lightDir = Math::vec3(-1, -2, 1);
			Math::vec4 lightColor = Math::vec3(0.9, 0.9, 0.9);
			Math::vec4 camPos { 0.0f, -4.0f, 0.0f };
		} shaderstuff;
		ScreenPass.add_renderfunction([&](vulkan::CommandBufferHandle& cmd) {
			cmd->bind_program(fullscreenProgram);
			cmd->disable_depth();
			cmd->set_cull_mode(VK_CULL_MODE_NONE);
			cmd->push_constants(&shaderstuff, 0, sizeof(ShaderStuff));
			cmd->bind_input_attachment(0, 0);
			cmd->draw(3, 1, 0, 0);
		});
		//pass2.add_post_barrier("depth");

		RendererCamera camera{};
		Transform* transform = nullptr;
		Math::vec3 rotation({ 0.0f, 0.0f, 0.0f });
		Math::vec3 position({ 0.0f, 0.0f, 0.0f });
		float fovX = 90.f;
		float fovY = 45.f;
		float distance = 1.0f;
		float aspect = application.get_height() / static_cast<float>(application.get_width());
		//camera.view = Math::mat44::look_at(Math::vec3({ 0.0f, -4.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
		//camera.proj = Math::mat44::perspectiveX(1.f, 10.f, fov, aspect);

		//device.create_stuff(tex->get_view()->get_image_view());
		bool is_fullscreen_window = false;
		bool should_fullscreen_window = false;
		int mipLevel = 0;
		auto* rtProgram = shaderManager.request_program("rt.comp");
		pass.add_renderfunction([&](vulkan::CommandBufferHandle& cmd) {
			cmd->begin_region("Renderer");
			cmd->bind_program(rtProgram);
			cmd->dispatch(1920, 1080, 1);
			cmd->end_region();
		});
		while (!window.should_close())
		{
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
				aspect = static_cast<float>(application.get_width())/ application.get_height();
				if (transform) {
					transform->transform = Math::mat44(Math::mat33::rotation_matrix(rotation));
					transform->transform(3, 3) = 1;
					transform->transform = Math::mat44::translation_matrix(position) * transform->transform;
				}
				camera.proj = Math::mat44::perspectiveY(0.01f, 10.f, fovY, aspect);
				//camera.proj = Math::mat44::perspectiveXY(0.01f, 100.f, fovX, fovY);
				camera.view = Math::mat44::look_at(shaderstuff.camPos * distance, Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
				application.next_frame();

				ImGui::Begin("Interaction");
				ImGui::ColorEdit4("Clearcolor", &attachment.clearColor[0]);
				
				ImGui::SliderFloat("distance", &distance, 0.0f, 10.f);
				//ImGui::SliderFloat("fovX", &fovX, 45.f, 110.0f);
				ImGui::SliderFloat("fovY", &fovY, 25.f, 70.0f);

				ImGui::ColorEdit3("Lightcolor", &shaderstuff.lightColor[0]);
				ImGui::Text("Camera Position");
				ImGui::DragFloat3("   ", &shaderstuff.camPos[0], 0.01f, -10.0f, 10.f, "%.5f", ImGuiSliderFlags_None);
				ImGui::Text("Light Direction");
				ImGui::DragFloat3("  ", &shaderstuff.lightDir[0], 0.01f, -1.f, 1.f, "%.5f", ImGuiSliderFlags_None);
				shaderstuff.lightDir.normalize();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Mip Level:");
				ImGui::SameLine();

				// Arrow buttons with Repeater
				float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
				ImGui::PushButtonRepeat(true);

				ImGui::SameLine(0.0f, spacing);

				ImGui::PopButtonRepeat();
				ImGui::SameLine();
				ImGui::Text("%d", mipLevel);

				ImGui::Checkbox("Fullscreen Windowed", &should_fullscreen_window);
				ImGui::End();
				ImGui::Begin("Model");
				ImGui::SameLine();
				ImGui::End();

				//auto rp = device.request_swapchain_render_pass(vulkan::SwapchainRenderpassType::Depth);
				//pass.set_render_pass(rp);
				rendergraph.execute();
				//imgui.end_frame();
				application.end_frame();
			}
		}
		device.wait_idle();
	//}
	//catch (const std::runtime_error& error) {
	//	std::cerr << error.what() << std::endl;
	//	return EXIT_FAILURE;
	//}

	return 0;
}