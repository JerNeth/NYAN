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
#include "VulkanWrapper/Shader.h"
using namespace nyan;

int main() {
	auto name = "Demo";
	nyan::Application application(name);
	//TODO Tonemap: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting

	auto& device = application.get_device();
	auto& window = application.get_window();

	auto& input = application.get_input();

	auto directory = getenv("USERPROFILE") / std::filesystem::path{ "Assets" };

	nyan::RenderManager renderManager(device, true, directory);
	nyan::CameraController cameraController(renderManager, input);
	auto& registry = renderManager.get_registry();
	Utility::FBXReader reader(directory);
	std::vector<nyan::Mesh> meshes;
	std::vector<nyan::MaterialData> materials;
	reader.parse_meshes("cube.fbx", meshes, materials);
	renderManager.add_materials(materials);
	//TODO do barrier issues, issue is first aquire of ddgi, but aquire is already implicitly done, also wrong initial format

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


	auto& rendergraph {renderManager.get_render_graph()};

	
	auto gbuffer = rendergraph.add_gbuffer("gbuffer");
	auto lighting = rendergraph.add_lighting("lighting");


	auto& ddgiPass = rendergraph.get_pass(rendergraph.add_pass("DDGI-Pass", nyan::Renderpass::Type::AsyncCompute));
	auto& deferredPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Pass", nyan::Renderpass::Type::Generic));
	
	auto& deferredRTPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Lighting-Pass", nyan::Renderpass::Type::Generic));

	auto& forwardPass = rendergraph.get_pass(rendergraph.add_pass("Forward-Pass", nyan::Renderpass::Type::Generic));

	auto& compositePass = rendergraph.get_pass(rendergraph.add_pass("Composite-Pass", nyan::Renderpass::Type::Generic));


	auto& imguiPass = rendergraph.get_pass(rendergraph.add_pass("Imgui-Pass", nyan::Renderpass::Type::Generic));

	nyan::DDGIRenderer ddgiRenderer(device, registry, renderManager, ddgiPass);
	nyan::MeshRenderer meshRenderer(device, registry, renderManager, deferredPass, gbuffer);
	//nyan::DeferredLighting deferredLighting(device, registry, renderManager, deferredLightingPass);
	nyan::DeferredRayShadowsLighting deferredLighting2(device, registry, renderManager, deferredRTPass, gbuffer, lighting);
	nyan::ForwardMeshRenderer forwardMeshRenderer(device, registry, renderManager, forwardPass, lighting, gbuffer.depth);
	nyan::LightComposite lightComposite(device, registry, renderManager, compositePass, lighting);
	nyan::ImguiRenderer imgui(device, registry, renderManager, imguiPass, &window);
	rendergraph.build();
	application.each_update([&](std::chrono::nanoseconds dt)
		{
			cameraController.update(dt);
			renderManager.update(dt);
			//imgui.update();
		});
	application.each_frame_begin([&]()
		{
			ddgiRenderer.begin_frame();
			rendergraph.begin_frame();

			imgui.next_frame();

			//Upload/sync point here, don't really want stuff after here
			renderManager.begin_frame();

		});
	application.each_frame_end([&]()
		{
			renderManager.end_frame();
			rendergraph.end_frame();
			//device.wait_idle(); //Brute forcing synchronization, currently 2 Frames in flight and I think the view matrix update gets mangled into the frame
			//					//Potential solution, use staging buffer for rendermanager updates and synchronize via semaphores and barriers
		});
	application.main_loop();


	return 0;
}