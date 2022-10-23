#include <iostream>
#include <fbxsdk.h>
#include <Util>
#include "Application.h"
#include "Input.h"
#include "Renderer/ShaderInterface.h"
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
	std::vector<nyan::LightParameters> lights;
	//reader.parse_meshes("test.fbx", meshes, materials, lights);
	reader.parse_meshes("SunTemple.fbx", meshes, materials, lights);
	//reader.parse_meshes("san_miguel.fbx", meshes, materials, lights);
	//reader.parse_meshes("cube.fbx", meshes, materials, lights);
	renderManager.add_materials(materials);
	//TODO do barrier issues on async Compute, issue is first queue aquire of ddgi, with no release and initial queue aquire is already implicitly done, also wrong initial format

	auto parent = registry.create();
;
	registry.emplace<Transform>(parent,
		Transform{
			.position{0.f, 0.f, 0.f},
			.scale{1.f},
			.orientation{0, 0, 0},
		});
	auto camera = registry.create();
	//registry.emplace<Transform>(camera,
	//	Transform{
	//		.position{600.f, 350.f,-960.f},
	//		.scale{1.f},
	//		.orientation{14.f, -145.f, 0.f}, //Cathedral
	//	});
	registry.emplace<Transform>(camera,
		Transform{
			.position{700.f, 1160.f,-1570.f},
			.scale{1.f},
			.orientation{16.4f, -20.f, 0.f}, //Cathedral
		});
	//registry.emplace<Transform>(camera,
	//	Transform{
	//		.position{0.f, 10.f,  5.f},
	//		.scale{1.f},
	//		.orientation{14.f, -145.f, 0.f}, //Cathedral
	//	});

	registry.emplace<nyan::DDGIManager::DDGIVolumeParameters>(parent, nyan::DDGIManager::DDGIVolumeParameters{
			.spacing {250.f, 250.f, 250.f},
			.origin {-2500.f, -200.f, -2400.f },
			.probeCount {20, 12, 40},
			.raysPerProbe {192},
			.depthBias {10},
			.visualizerRadius {10},
			.minFrontFaceDistance {100},
			
		});
	//registry.emplace<nyan::DDGIManager::DDGIVolumeParameters>(parent, nyan::DDGIManager::DDGIVolumeParameters{
	//		.spacing {57.5f, 25.f, 57.5f},
	//		.origin {-500, 0.f, -500.f },
	//		.probeCount {18, 4, 18},
	//	});
	//registry.emplace<Transform>(camera,
	//	Transform{
	//		.position{50, 10,20},
	//		.scale{1.f},
	//		.orientation{14.f, -145.f, 0.f}, //Cathedral
	//	});
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
	registry.emplace< CameraMovement>(camera,
		CameraMovement{
			.speed {500}
		});

	renderManager.set_primary_camera(camera);
	for (auto& a : meshes) {
		bool opaque = !(!a.name.empty() && a.name.find(".DoubleSided") != std::string::npos);
		a.type = opaque? nyan::Mesh::RenderType::Opaque : nyan::Mesh::RenderType::AlphaTest;
		auto meshId = renderManager.get_mesh_manager().add_mesh(a);
		auto accHandle = renderManager.get_mesh_manager().get_acceleration_structure(meshId);
		auto entity = registry.create();
		registry.emplace<MeshID>(entity, meshId);
		registry.emplace<MaterialId>(entity, renderManager.get_material_manager().get_material(a.material));
		auto instance = accHandle ?
			InstanceData{
				(*accHandle)->create_instance(static_cast<uint32_t>(a.type))
		} :
			InstanceData{
				.transform{
					.transformMatrix = Math::Mat<float, 3, 4, false>::identity()
				}
		};
		instance.instance.instanceCustomIndex = meshId;
		//instance.instance.instanceShaderBindingTableRecordOffset = static_cast<uint32_t>(a.type); //Check renderManager if changed here
		registry.emplace<InstanceId>(entity, renderManager.get_instance_manager().add_instance(instance));
		registry.emplace<Transform>(entity,
			Transform{
				.position{a.translate},
				.scale{1.f},
				.orientation{a.rotate},
			});
		registry.emplace<Parent>(entity,
			Parent{
				.parent {parent},
			});
		registry.emplace<std::string>(entity, a.name);
		if(a.type == nyan::Mesh::RenderType::Opaque)
			registry.emplace<Deferred>(entity);
		else if (a.type == nyan::Mesh::RenderType::AlphaTest)
			registry.emplace<DeferredDoubleSidedAlphaTest>(entity);
		else if (a.type == nyan::Mesh::RenderType::AlphaBlend)
			registry.emplace<ForwardTransparent>(entity);

	}

	for (const auto& light : lights) {
		auto entity = registry.create();
		registry.emplace<Transform>(entity,
			Transform{
				.position{light.translate},
				.scale{1.f},
				.orientation{light.rotate},
			});
		registry.emplace<Parent>(entity,
			Parent{
				.parent {parent},
			});
		registry.emplace<std::string>(entity, light.name);
		if (light.type == LightParameters::Type::Directional)
			registry.emplace<Directionallight>(entity, Directionallight
				{
					.enabled {true},
					.shadows{ true },
					.color {light.color},
					//.intensity {light.intensity},
					.intensity {1},
					.direction {light.direction},
				});
		if (light.type == LightParameters::Type::Point)
			registry.emplace<Pointlight>(entity, Pointlight
				{
					.shadows{ true },
					.color {light.color},
					.intensity {light.intensity},
					.attenuation {500}
				});
		if (light.type == LightParameters::Type::Spot)
			registry.emplace<Spotlight>(entity, Spotlight
				{
					.shadows{ true },
					.color {light.color},
					.intensity {light.intensity},
					.direction {Math::vec3 {0, 0, 1}},
					.cone {45},
					.attenuation {500},
				});
	}


	auto& rendergraph {renderManager.get_render_graph()};

	

	auto gbuffer = rendergraph.add_gbuffer("gbuffer");
	auto lighting = rendergraph.add_lighting("lighting");


	auto& ddgiPass = rendergraph.get_pass(rendergraph.add_pass("DDGI-Pass", nyan::Renderpass::Type::Generic));
	auto& deferredPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Pass", nyan::Renderpass::Type::Generic));

	auto& deferredRTPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Lighting-Pass", nyan::Renderpass::Type::Generic));

	auto& forwardPass = rendergraph.get_pass(rendergraph.add_pass("Forward-Pass", nyan::Renderpass::Type::Generic));

	auto& compositePass = rendergraph.get_pass(rendergraph.add_pass("Composite-Pass", nyan::Renderpass::Type::Generic));

	auto& imguiPass = rendergraph.get_pass(rendergraph.add_pass("Imgui-Pass", nyan::Renderpass::Type::Generic));

	nyan::DDGIRenderer ddgiRenderer(device, registry, renderManager, ddgiPass);
	nyan::MeshRenderer meshRenderer(device, registry, renderManager, deferredPass, gbuffer);
	nyan::DeferredRayShadowsLighting deferredLighting2(device, registry, renderManager, deferredRTPass, gbuffer, lighting);
	nyan::ForwardMeshRenderer forwardMeshRenderer(device, registry, renderManager, forwardPass, lighting, gbuffer.depth);
	nyan::DDGIVisualizer ddgiVisualizer(device, registry, renderManager, forwardPass, lighting, gbuffer.depth);
	//nyan::DeferredLighting deferredLighting(device, registry, renderManager, deferredLightingPass);
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
			//ImGui first here since we might want to use ImGui in other begin_frames
			imgui.begin_frame();

			ddgiRenderer.begin_frame();
			rendergraph.begin_frame();

			//ImGui::Begin("Input");
			//ImGui::Text("Look Right %f", input.get_axis(Input::Axis::LookRight));
			//ImGui::Text("Look Up %f", input.get_axis(Input::Axis::LookUp));
			//ImGui::Text("Move Right %f", input.get_axis(Input::Axis::MoveRight));
			//ImGui::Text("Move Forward %f", input.get_axis(Input::Axis::MoveForward));
			//ImGui::End();

			//Upload/sync point here, don't really want stuff after here
			renderManager.begin_frame();

		});
	application.each_frame_end([&]()
		{
			renderManager.end_frame();
			rendergraph.end_frame();
		});
	application.main_loop();


	return 0;
}