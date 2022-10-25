#include <iostream>
#include <Util>
#include "Application.h"

#include "Renderer/Mesh.h"
#include "Renderer/Material.h"
#include "Renderer/Light.h"

#include "Renderer/MeshRenderer.h"
#include "Renderer/DDGIRenderer.h"
#include "Renderer/CameraController.h"
#include "Renderer/DeferredLighting.h"
#include "GLTFReader/GLTFReader.hpp"

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

	std::filesystem::path file;
	file = "sponza-gltf-pbr/sponza.glb";
	file = "Sponza/glTF/Sponza.gltf";
	//file = "NewSponza_Main_glTF_002.gltf";
	//file = "glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF/NormalTangentMirrorTest.gltf";
	std::filesystem::path dir = "glTF-Sample-Models/2.0";
	//dir = "Main.1_Sponza";
	//file = dir / "OrientationTest/glTF/OrientationTest.gltf";
	//file = dir / "BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf";
	//file = "coord.glb";
	//file = "sphere.glb";
	//file = "cornellFixed.gltf";
	std::filesystem::path path = directory / dir / file;
	//std::filesystem::path path = directory /file;
	nyan::GLTFReader reader{ renderManager };
	reader.load_file(path);




	auto parent = registry.create();
	
	registry.emplace<Transform>(parent,
		Transform{
			.position{0.f, 0.f, 0.f},
			.scale{1.f},
			.orientation{0, 0, 0},
		});
	auto camera = registry.create();
	registry.emplace<Transform>(camera,
		Transform{
			.position{0., 10.f, 0.f},
			.scale{1.f},
			.orientation{16.4f, -20.f, 0.f},
		});

	//registry.emplace<nyan::DDGIManager::DDGIVolumeParameters>(parent, nyan::DDGIManager::DDGIVolumeParameters{
	//		.visualization {false}
	//	});
	registry.emplace<nyan::DDGIManager::DDGIVolumeParameters>(parent, nyan::DDGIManager::DDGIVolumeParameters{
			.spacing {1.02f, 0.5f, 0.45f},
			.origin {-0.4f - 11.f * 1.02f, 5.4f - 0.5f * 11.f, -0.25f - 0.45f * 11.f},
			.probeCount {22, 22, 22},
			.raysPerProbe {256},
			.depthBias {0.1},
			.depthViewBias {0.3},
			.hysteresis {0.97},
			.irradianceThreshold {0.2},
			.lightToDarkThreshold {2.0},
			.visualizerRadius {0.1},
			.minFrontFaceDistance {0.3},
			.visualization {false},
			.relocationEnabled {true},
		}); //Sponza

	registry.emplace<Directionallight>(parent, Directionallight
		{
			.enabled {true},
			.shadows{ true },
			.color {1.f, 1.f, 1.f},
			//.intensity {light.intensity},
			.intensity {1.45},
			.direction {0.f, -1.f, -0.300f},
		});

	registry.emplace<SkyLight>(parent, SkyLight
		{
			.color {1.f, 1.f, 1.f},
			.intensity {0.1},
			//.intensity {5.1},
		});

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

	auto& rendergraph{ renderManager.get_render_graph() };


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


}