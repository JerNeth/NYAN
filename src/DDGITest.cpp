#include <iostream>
#include <fbxsdk.h>
#include <Util>
#include "Application.h"
#include "Input.h"
#include "FBXReader/FBXReader.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/DDGIRenderer.h"
#include "Renderer/CameraController.h"
#include "Renderer/DeferredLighting.h"
using namespace nyan;

int main() {
	auto name = "Demo";
	nyan::Application application(name);
	//TODO Tonemap: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting

	auto& device = application.get_device();
	auto& window = application.get_window();

	auto& input = application.get_input();

	nyan::RenderManager renderManager(device, true);
	nyan::CameraController cameraController(renderManager, input);
	auto& registry = renderManager.get_registry();
	Utility::FBXReader reader;
	std::vector<nyan::Mesh> meshes;
	std::vector<nyan::MaterialData> materials;
	reader.parse_meshes("ShaderBall.fbx", meshes, materials);
	renderManager.add_materials(materials);


	auto parent = registry.create();
	registry.emplace<Transform>(parent,
		Transform{
			.position{},
			.scale{},
			.orientation{0, 180, 0},
		});
	auto camera = registry.create();
	registry.emplace<Transform>(camera,
		Transform{
			.position{600.f, 350.f,960.f},
			.scale{},
			.orientation{14.f, -145.f, 0.f}, //Cathedral
		});
	//Transform{
	//.position{108.f, 216.f,320.f},
	//.scale{},
	//.orientation{31.8f, -157.f, 0.f},
	//});
	//Transform{
	//.position{600.f, 660.f, -1400.f},
	//.scale{},
	//.orientation{18.f, -27.f, 0.f},
	//}
	registry.emplace<PerspectiveCamera>(camera,
		PerspectiveCamera{
			.nearPlane {.1f},
			.farPlane {10000.f},
			.fovX {90.f},
			.aspect {16.f / 9.f },
			.forward {0.f, 0.f ,1.f},
			.up {0.f, 1.f ,0.f},
			.right {1.f, 0.f ,0.f},
		});
	renderManager.set_primary_camera(camera);
	for (const auto& a : meshes) {

		auto meshId = renderManager.get_mesh_manager().add_mesh(a);
		auto accHandle = renderManager.get_mesh_manager().get_acceleration_structure(meshId);
		auto entity = registry.create();
		registry.emplace<MeshID>(entity, meshId);
		registry.emplace<MaterialId>(entity, renderManager.get_material_manager().get_material(a.material));
		auto instance = accHandle ?
			InstanceData{
				(*accHandle)->create_instance()
		} :
			InstanceData{
				.transform{
					.transformMatrix = Math::Mat<float, 3, 4, false>::identity()
				}
		};
		instance.instance.instanceCustomIndex = meshId;
		registry.emplace<InstanceId>(entity, renderManager.get_instance_manager().add_instance(instance));
		registry.emplace<Transform>(entity,
			Transform{
				.position{},
				.scale{},
				.orientation{},
			});
		registry.emplace<Parent>(entity,
			Parent{
				.parent {parent},
			});
		registry.emplace<std::string>(entity, a.name);
		registry.emplace<DeferredAlphaTest>(entity);

	}



	nyan::Rendergraph rendergraph{ device };
	auto g_Depth = rendergraph.add_ressource("g_Depth", nyan::ImageAttachment
		{
			.format{VK_FORMAT_D32_SFLOAT_S8_UINT},
			.clearColor{0.f, static_cast<float>(static_cast<uint8_t>(0)), 0.f, 0.f},
		});
	auto g_Albedo = rendergraph.add_ressource("g_Albedo", nyan::ImageAttachment
		{
			//.format{VK_FORMAT_R16G16B16A16_SFLOAT},
			.format{VK_FORMAT_R8G8B8A8_SRGB},
			//.format{VK_FORMAT_B10G11R11_UFLOAT_PACK32},
			.clearColor{0.0f, 0.0f, 0.0f, 1.f},
		});
	auto g_Normal = rendergraph.add_ressource("g_Normal", nyan::ImageAttachment
		{
			//.format{VK_FORMAT_R8G8B8A8_UNORM},
			.format{VK_FORMAT_R8G8B8A8_UNORM},
			.clearColor{0.f, 0.f, 1.f, 1.f},
		});
	auto g_PBR = rendergraph.add_ressource("g_PBR", nyan::ImageAttachment
		{
			.format{VK_FORMAT_R8G8B8A8_UNORM},
			.clearColor{0.f, 0.f, 0.f, 1.f},
		});
	auto SpecularLighting = rendergraph.add_ressource("SpecularLighting", nyan::ImageAttachment
		{
			.format{VK_FORMAT_R16G16B16A16_SFLOAT},
			//.clearColor{0.0f, 0.0f, 0.0f, 1.f},
			.clearColor{0.4f, 0.6f, 0.8f, 1.f},
		});
	auto DiffuseLighting = rendergraph.add_ressource("DiffuseLighting", nyan::ImageAttachment
		{
			.format{VK_FORMAT_B10G11R11_UFLOAT_PACK32},
			//.clearColor{0.4f, 0.6f, 0.8f, 1.f},
			.clearColor{0.0f, 0.0f, 0.0f, 1.f},
		});


	auto& deferredPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Pass", nyan::Renderpass::Type::Generic));
	deferredPass.add_depth_stencil_attachment(g_Depth, true);
	deferredPass.add_attachment(g_Albedo, true);
	deferredPass.add_attachment(g_Normal, true);
	deferredPass.add_attachment(g_PBR, true);
	//deferredPass.add_swapchain_attachment();
	//deferredPass.copy("g_Depth", "g_Depth2");
	//deferredPass.copy("g_Normal", "g_Normal2");

	//auto& deferredLightingPass = rendergraph.add_pass("Deferred-Lighting-Pass", nyan::Renderpass::Type::Generic);
	//deferredLightingPass.add_read("g_Albedo");
	//deferredLightingPass.add_read("g_Normal");
	//deferredLightingPass.add_read("g_PBR");
	//deferredLightingPass.add_read("g_Depth", nyan::Renderpass::Read::Type::ImageDepth);
	//deferredLightingPass.add_read("g_Depth", nyan::Renderpass::Read::Type::ImageStencil);
	//deferredLightingPass.add_attachment("SpecularLighting", nyan::ImageAttachment
	//	{
	//		.format{VK_FORMAT_R16G16B16A16_SFLOAT},
	//		//.clearColor{0.0f, 0.0f, 0.0f, 1.f},
	//		.clearColor{0.4f, 0.6f, 0.8f, 1.f},
	//	});
	//deferredLightingPass.add_attachment("DiffuseLighting", nyan::ImageAttachment
	//	{
	//		.format{VK_FORMAT_B10G11R11_UFLOAT_PACK32},
	//		//.clearColor{0.4f, 0.6f, 0.8f, 1.f},
	//		.clearColor{0.0f, 0.0f, 0.0f, 1.f},
	//	});
	//auto& ddgiPass = rendergraph.add_pass("DDGI-Pass", nyan::Renderpass::Type::Generic);

	
	auto& deferredRTPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Lighting-Pass", nyan::Renderpass::Type::Generic));
	deferredRTPass.add_read(g_Albedo);
	deferredRTPass.add_read(g_Normal);
	deferredRTPass.add_read(g_PBR);
	deferredRTPass.add_read(g_Depth, nyan::Renderpass::Read::Type::ImageDepth);
	deferredRTPass.add_read(g_Depth, nyan::Renderpass::Read::Type::ImageStencil);
	deferredRTPass.add_write(SpecularLighting, nyan::Renderpass::Write::Type::Compute);
	deferredRTPass.add_write(DiffuseLighting, nyan::Renderpass::Write::Type::Compute);


	auto& forwardPass = rendergraph.get_pass(rendergraph.add_pass("Forward-Pass", nyan::Renderpass::Type::Generic));
	forwardPass.add_depth_attachment(g_Depth);
	forwardPass.add_attachment(SpecularLighting);
	forwardPass.add_attachment(DiffuseLighting);

	auto& compositePass = rendergraph.get_pass(rendergraph.add_pass("Composite-Pass", nyan::Renderpass::Type::Generic));
	compositePass.add_read(SpecularLighting);
	compositePass.add_read(DiffuseLighting);
	compositePass.add_swapchain_attachment(Math::vec4{}, true);


	auto& imguiPass = rendergraph.get_pass(rendergraph.add_pass("Imgui-Pass", nyan::Renderpass::Type::Generic));
	imguiPass.add_swapchain_attachment();


	nyan::MeshRenderer meshRenderer(device, registry, renderManager, deferredPass);
	//nyan::DeferredLighting deferredLighting(device, registry, renderManager, deferredLightingPass);
	//nyan::DDGIRenderer ddgiRenderer(device, registry, renderManager, ddgiPass);
	nyan::DeferredRayShadowsLighting deferredLighting2(device, registry, renderManager, deferredRTPass, g_Albedo, g_Normal, g_PBR, g_Depth, g_Depth, DiffuseLighting, SpecularLighting);
	nyan::ForwardMeshRenderer forwardMeshRenderer(device, registry, renderManager, forwardPass);
	nyan::LightComposite lightComposite(device, registry, renderManager, compositePass, DiffuseLighting, SpecularLighting);
	nyan::ImguiRenderer imgui(device, registry, renderManager, imguiPass, &window);
	rendergraph.build();
	application.each_frame_begin([&]()
		{
			renderManager.update();
			imgui.next_frame();
		});
	application.each_update([&](std::chrono::nanoseconds dt)
		{
			cameraController.update(dt);
			//imgui.update();
		});
	application.each_frame_end([&rendergraph, &device]()
		{
			//Could split an update out
			rendergraph.execute();
			//device.wait_idle(); //Brute forcing synchronization, currently 2 Frames in flight and I think the view matrix update gets mangled into the frame
			//					//Potential solution, use staging buffer for rendermanager updates and synchronize via semaphores and barriers
		});
	application.main_loop();


	return 0;
}