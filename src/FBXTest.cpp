#include <iostream>
#include <fbxsdk.h>
#include <Util>
#include "Application.h"
#include "Utility/FBXReader.h"
using namespace nyan;
int main() {
	Utility::FBXReader fbxReader;
	char meshName[255] = "sphere.fbx";
	//auto meshes = fbxReader.parse_meshes(meshName);
	//auto& mesh = meshes.back();
	auto texName = "tex";
	auto texNameNormal = "NormalTangent";
	//auto texNameNormal = "Normal";
	//auto texNameNormal = "FGD";

	nyan::Application application("Demo");
	//try {

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
		//depth.format = VK_FORMAT_D24_UNORM_S8_UINT;
		//depth.format = VK_FORMAT_D16_UNORM;
		depth.format = VK_FORMAT_D32_SFLOAT;
		pass.add_depth_output("depth", depth);
		nyan::ImageAttachment color;
		//color.format = VK_FORMAT_R8G8B8A8_UNORM;
		//color.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		color.format = VK_FORMAT_R8G8B8A8_SRGB;
		color.clearColor = Math::vec4(.1f, .3f, .7f, 0.f);

		Math::mat22(1, 2, 3, 4);
		nyan::ImageAttachment normal;
		normal.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		normal.clearColor = Math::vec4({ 0.f, 0.f, 1.f, 0.f});
		pass.add_output("color", color);
		pass.add_output("normal", normal);
		auto& pass2 = rendergraph.add_pass("ScreenPass", nyan::Renderpass::Type::Graphics);
		pass2.add_input("color");
		//pass2.add_read_dependency("depth");
		pass2.add_input("depth");
		pass2.add_input("normal");
		pass2.add_swapchain_output();
		rendergraph.build();

		auto& attachment = std::get<nyan::ImageAttachment>(rendergraph.get_resource("color").attachment);

		//auto* fullscreenProgram = shaderManager.request_program("fullscreen_vert", "fullscreen_frag");
		auto* fullscreenProgram = shaderManager.request_program("fullscreen_vert", "deferred_frag");
		pass2.add_renderfunction([&](vulkan::CommandBufferHandle& cmd) {
			cmd->bind_program(fullscreenProgram);
			cmd->disable_depth();
			cmd->set_cull_mode(VK_CULL_MODE_NONE);
			cmd->bind_input_attachment(0, 0);
			//cmd->bind_input_attachment(0, 1);
			//cmd->bind_input_attachment(0, 2);
			//auto& depth= rendergraph.get_resource("depth");
			//cmd->bind_texture(0, 1, *depth.handle, vulkan::DefaultSampler::LinearClamp);
			cmd->draw(3, 1, 0, 0);
		});
		//pass2.add_post_barrier("depth");

		Material testMaterial(0, "default");
		//StaticMesh* testMesh = meshManager.request_static_mesh("TestMesh");
		StaticMesh* viewPortMesh = nullptr; 

		testMaterial.add_texture(textureManager.request_texture(texName));
		testMaterial.add_texture(textureManager.request_texture(texNameNormal));

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
		bool culling = true;
		int mipLevel = 0;
		pass.add_renderfunction([&](vulkan::CommandBufferHandle& cmd) {
			if (wireframe)
				cmd->set_polygon_mode(VK_POLYGON_MODE_LINE);
			cmd->begin_region("Renderer");
			//cmd->set_cull_mode(VK_CULL_MODE_BACK_BIT);
			if (culling) {
				cmd->set_cull_mode(VK_CULL_MODE_BACK_BIT);
			}
			else {
				cmd->set_cull_mode(VK_CULL_MODE_NONE);
			}
			cmd->set_depth_write(VK_TRUE);
			cmd->set_depth_test(VK_TRUE);
			renderer.render(cmd);
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
				camera.view = Math::mat44::look_at(Math::vec3({ 0.0f, -4.0f, 0.0f }) * distance, Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
				renderer.update_camera(camera);
				application.next_frame();

				ImGui::Begin("Interaction");
				ImGui::ColorEdit4("Clearcolor", &attachment.clearColor[0]);
				
				ImGui::SliderFloat("distance", &distance, 0.0f, 10.f);
				//ImGui::SliderFloat("fovX", &fovX, 45.f, 110.0f);
				ImGui::SliderFloat("fovY", &fovY, 25.f, 70.0f);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Mip Level:");
				ImGui::SameLine();

				// Arrow buttons with Repeater
				float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
				ImGui::PushButtonRepeat(true);
				if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
					if (mipLevel) {
						mipLevel--; textureManager.change_mip(texName, mipLevel);

					}
				}
				ImGui::SameLine(0.0f, spacing);
				if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
					mipLevel++;
					textureManager.change_mip(texName, mipLevel);
				}
				ImGui::PopButtonRepeat();
				ImGui::SameLine();
				ImGui::Text("%d", mipLevel);

				ImGui::Checkbox("Fullscreen Windowed", &should_fullscreen_window);
				ImGui::Checkbox("Backface Culling", &culling);
				ImGui::Checkbox("Wireframe", &wireframe);
				ImGui::End();
				ImGui::Begin("Model");
				if (ImGui::Button("Load Model")) {
					//auto meshes = fbxReader.parse_meshes(meshName, false);
					//if (!meshes.empty()) {
					//	auto& mesh = meshes.back();
					//	if(mesh.get_static_vertices().size() < UINT16_MAX)
					//		viewPortMesh = meshManager.request_mesh<StaticMesh>(mesh.name, mesh.get_static_vertices(), mesh.get_indices16());
					//	else
					//		viewPortMesh = meshManager.request_mesh<StaticMesh>(mesh.name, mesh.get_static_vertices(), mesh.get_indices32());
					//	viewPortMesh->set_material(&testMaterial);
					//	transform = &viewPortMesh->get_transform();
					//}
					auto meshes = fbxReader.parse_meshes(meshName, true);
					if (!meshes.empty()) {
						auto& mesh = meshes.back();
						if (mesh.get_static_tangent_vertices().size() < UINT16_MAX)
							viewPortMesh = meshManager.request_mesh<StaticMesh>(mesh.name, mesh.get_static_tangent_vertices(), mesh.get_indices16());
						else
							viewPortMesh = meshManager.request_mesh<StaticMesh>(mesh.name, mesh.get_static_tangent_vertices(), mesh.get_indices32());
						viewPortMesh->set_material(&testMaterial);
						transform = &viewPortMesh->get_transform();
					}
				}
				ImGui::SameLine();
				ImGui::InputText("", meshName, 255);
				if (viewPortMesh) {
					if (ImGui::TreeNode("Transform")) {
						ImGui::Text("Orientation");
						ImGui::DragFloat3("", &rotation[0], 1.0f, -360.0f, 720.f, "%.2f", ImGuiSliderFlags_None);
						ImGui::SameLine();
						if (ImGui::Button("Reset")) {
							rotation[0] = 0.f;
							rotation[1] = 0.f;
							rotation[2] = 0.f;
						}
						if (rotation[0] > 360.0f)
							rotation[0] -= 360.0f;
						if (rotation[1] > 360.0f)
							rotation[1] -= 360.0f;
						if (rotation[2] > 360.0f)
							rotation[2] -= 360.0f;
						if (rotation[0] < 0.f)
							rotation[0] += 360.0f;
						if (rotation[1] < 0.f)
							rotation[1] += 360.0f;
						if (rotation[2] < 0.f)
							rotation[2] += 360.0f;

						ImGui::Text("Position");
						ImGui::DragFloat3(" ", &position[0], 0.01f, -10.0f, 10.f, "%.5f", ImGuiSliderFlags_None); 
						ImGui::SameLine();
						if (ImGui::Button("Reset ")) {
							position[0] = 0.f;
							position[1] = 0.f;
							position[2] = 0.f;
						}
						ImGui::TreePop();
					}
				}
				ImGui::End();

				renderer.queue_mesh(viewPortMesh);

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