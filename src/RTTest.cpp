#include <iostream>
#include <Util>
#include "Application.h"
#include <span>

#include "Renderer/Mesh.h"
#include "Renderer/Material.h"
#include "Renderer/Light.h"

#include "Renderer/MeshRenderer.h"
#include "Renderer/DDGIRenderer.h"
#include "Renderer/CameraController.h"
#include "Renderer/DeferredLighting.h"
#include "Renderer/Profiler.hpp"
#include "GLTFReader/GLTFReader.hpp"

using namespace nyan;

int main() {
	auto name = "Demo";
	nyan::Application application(name);

	auto& device = application.get_device();
	auto& window = application.get_window();

	auto& input = application.get_input();

	auto directory = getenv("USERPROFILE") / std::filesystem::path{ "Assets" };

	nyan::RenderManager renderManager(device, true, directory);
	nyan::CameraController cameraController(renderManager, input);
	auto& registry = renderManager.get_registry();
	std::filesystem::path file;
	//file = "sponza-gltf-pbr/sponza.glb";
	//file = "Sponza/glTF/Sponza.gltf";
	//file = "TestScene.gltf";
	//file = "SanMiguel.gltf";
	//file = "SunTemple.glb";
	//file = "NewSponza_Main_glTF_002.gltf";
	//file = "glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF/NormalTangentMirrorTest.gltf";
	std::filesystem::path dir = "glTF-Sample-Models/2.0";
	//file = "SanMiguel.gltf";
	//dir = "SanMiguel2";
	//dir = "TestScene";
	//dir = "SanMiguel";
	//dir = "Main.1_Sponza";
	//file = dir / "OrientationTest/glTF/OrientationTest.gltf";
	//file = dir / "BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf";
	//file = "coord.glb";
	//file = "sphere.glb";
	//file = "cornellFixed.gltf";


	Transform cameraTransform;
	std::string scene = "Sponza1";
	float dirIntensity = 1.45f;
	float envIntensity = 1.0f;
	auto dirDir = Math::vec3{ 0.f, -1.f, 0.300f };
	auto envCol = Math::vec3{ 1.f, 1.f, 1.f };
	bool antiAliasing = false;
	float fovX = 68.f;
	if (scene == "Sponza" || scene == "Sponza1" || scene == "Sponza2") {
		file = "Sponza/glTF/Sponza.gltf";
		dir = "glTF-Sample-Models/2.0";
		file = dir / file;
		cameraTransform = Transform{
					.position{7.46, 5.07f, 0.92f},
					.scale{1.f},
					.orientation{-13.22f, -295.23f, 0.f},
		};

		if (scene == "Sponza1") {
			cameraTransform = Transform{
						.position{10.f, 5.f, 3.f},
						.scale{1.f},
						.orientation{0.f, -270.f, 0.f},
			};
			dirIntensity = 1000.f;
		}
		if (scene == "Sponza2") {
			cameraTransform = Transform{
						.position{9.8f, 1.9f, -3.2f},
						.scale{1.f},
						.orientation{0.f, -250.f, 0.f},
			};
			dirIntensity = 1000.f;
		}
	}
	else if (scene == "SanMiguel" || scene == "SanMiguel1" || scene == "SanMiguel2") {
		file = "SanMiguel.gltf";
		dir = "SanMiguel2";
		file = dir / file;
		cameraTransform = Transform{
				.position{6.527f, 9.183f, 4.929f},
				.scale{1.f},
				.orientation{-30.625f, -45.f, 0.f},
		};
		if (scene == "SanMiguel2") {
			cameraTransform = Transform{
					.position{21.68f, 1.1f, 6.8f},
					.scale{1.f},
					.orientation{-3.f, -320.f, 0.f},
			};
			dirIntensity = 1000.f;
			dirDir = Math::vec3{ -1.f, -1.f, 1.f };
		}
	}
	else if (scene == "Cornell") {
		file = "cornellFixed.gltf";
		cameraTransform = Transform{
				.position{0.f, 0.f, 11.f},
				.scale{1.f},
				.orientation{0.f, 0.f, 0.f},
		};
		dirIntensity = 0.f;
		envIntensity = 0.f;
		envCol = Math::vec3{ 0.f, 0.f, 0.f };
	}
	else if (scene == "Spheres") {
		file = "Spheres.gltf";
		cameraTransform = Transform{
				.position{0.f, 0.f, 1000.f},
				.scale{1.f},
				.orientation{0.f, 0.f, 0.f},
		};
		fovX = 179.75f;
		dirIntensity = 5.f;
		envIntensity = 1.0f;
		//dirDir = Math::vec3{ -1.f, -1.f, -1.f };
		dirDir = Math::vec3{ -1.f, 0.f, -1.f };
		envCol = Math::vec3{ 1.f, 1.f, 1.f };
		antiAliasing = true;
	}


	std::filesystem::path path = directory  / file;
	//path = directory / file;
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
	Transform sponzaTransform {
			.position{7.46, 5.07f, 0.92f},
			.scale{1.f},
			.orientation{-13.22f, -295.23f, 0.f},
	};
	Transform sanMiguelTransform{
			.position{6.527f, 9.183f, 4.929f},
			.scale{1.f},
			.orientation{-25.f, -45.f, 0.f},
	};
	registry.emplace<Transform>(camera, cameraTransform);
	registry.emplace<PerspectiveCamera>(camera,
		PerspectiveCamera{
			.nearPlane {.1f},
			.farPlane {10000.f},
			.fovX {fovX},
			.aspect {16.f / 9.f },
			.forward {0.f, 0.f ,1.f},
			.up {0.f, 1.f ,0.f},
			.right {1.f, 0.f ,0.f},
		});
	registry.emplace<Directionallight>(parent, Directionallight
		{
			.enabled {true},
			.shadows{ true },
			.color {1.f, 1.f, 1.f},
			//.intensity {light.intensity},
			.intensity {dirIntensity},
			.direction {dirDir},
		});

	registry.emplace<SkyLight>(parent, SkyLight
		{
			.color {envCol},
			//.intensity {0.1},
			.intensity {envIntensity},
		});

	renderManager.set_primary_camera(camera);

	auto& rendergraph{ renderManager.get_render_graph() };
	rendergraph.set_profiler(&renderManager.get_profiler());

	auto lighting = rendergraph.add_lighting("lighting");


	auto& ptPass = rendergraph.get_pass(rendergraph.add_pass("PathTrace-Pass", nyan::Renderpass::Type::Generic));
	auto& compositePass = rendergraph.get_pass(rendergraph.add_pass("Composite-Pass", nyan::Renderpass::Type::Generic));

	auto& imguiPass = rendergraph.get_pass(rendergraph.add_pass("Imgui-Pass", nyan::Renderpass::Type::Generic));

	nyan::RTMeshRenderer meshRenderer(device, registry, renderManager, ptPass, lighting);
	//nyan::DeferredLighting deferredLighting(device, registry, renderManager, deferredLightingPass);
	nyan::LightComposite lightComposite(device, registry, renderManager, compositePass, lighting);
	nyan::ImguiRenderer imgui(device, registry, renderManager, imguiPass, &window);
	rendergraph.build();

	application.each_update([&](std::chrono::nanoseconds dt)
		{
			cameraController.update(dt);
			if (cameraController.changed())
				meshRenderer.reset();
			renderManager.update(dt);
		});
	application.each_frame_begin([&]()
		{
			//ImGui first here since we might want to use ImGui in other begin_frames
			imgui.begin_frame();

			renderManager.get_profiler().begin_frame();
			ImGui::Begin("Render Config");
			if (ImGui::Button("Screenshot"))
				lightComposite.queue_screenshot();
			static constexpr std::array tonemappingOptions = { "None", "ACESFilm", "Reinhard" , "Uncharted" };
			static uint32_t current_tonemapping = 0;
			if (ImGui::BeginCombo("##Tonemapping", tonemappingOptions[current_tonemapping])) {
				for (uint32_t item = 0; item < tonemappingOptions.size(); item++) {
					bool selected = (current_tonemapping == item);
					if (ImGui::Selectable(tonemappingOptions[item], selected))
						current_tonemapping = item;
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Reset Path Tracer"))
				meshRenderer.reset();
			ImGui::Checkbox("Antialiasing", &antiAliasing);
			meshRenderer.set_antialiasing(antiAliasing);
			static int maxPathLength = 10;
			ImGui::DragInt("Max Path Length", &maxPathLength, 1.f, 1.f, 64.f);
			meshRenderer.set_max_path_length(maxPathLength);

			ImGui::End();
			lightComposite.set_tonemapping(static_cast<nyan::LightComposite::ToneMapping>(current_tonemapping));

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