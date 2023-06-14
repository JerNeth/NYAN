#include <iostream>
#include <Util>
#include "Application.h"
#include <span>

#include "Renderer/Mesh.h"
#include "Renderer/Material.h"
#include "Renderer/Light.h"

#include "Renderer/MeshRenderer.h"
#include "Renderer/CameraController.h"
#include "Renderer/DeferredLighting.h"
#include "Renderer/Profiler.hpp"
#include "GLTFReader/GLTFReader.hpp"

using namespace nyan;

enum class ExitCode : int {
	Success = EXIT_SUCCESS,
	Failure = EXIT_FAILURE
};

int main(const int argc, char const* const* const argv)
{
	[[nodiscard]] ExitCode better_main(std::span<const std::string_view>);
	std::vector<std::string_view>
		args(argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)));

	return static_cast<int>(better_main(args));
}

[[nodiscard]] ExitCode better_main([[maybe_unused]] std::span<const std::string_view> args) {
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
	std::filesystem::path dir;
	Transform cameraTransform;

	//Params
	std::string scene = "Sponza";
	std::optional<std::filesystem::path> assetPath{ std::nullopt };
	bool quitAfterRecording = false;


	float dirIntensity = 1.45f;
	float envIntensity = 1.0f;
	auto dirDir = Math::vec3{ 0.f, -1.f, 0.300f };
	auto envCol = Math::vec3{ 1.f, 1.f, 1.f };
	if (scene == "Sponza") {
		file = "Sponza/glTF/Sponza.gltf";
		dir = "glTF-Sample-Models/2.0";
		file = dir / file;
		cameraTransform = Transform{
					.position{7.46, 5.07f, 0.92f},
					.scale{1.f},
					.orientation{-13.22f, -295.23f, 0.f},
		};
	}

	file = "OrientationTest/glTF/OrientationTest.gltf";
	dir = "glTF-Sample-Models/2.0";
	file = dir / file;

	std::filesystem::path path = directory / file;
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

	registry.emplace<Transform>(camera, cameraTransform);
	registry.emplace<PerspectiveCamera>(camera,
		PerspectiveCamera{
			.nearPlane {.1f},
			.farPlane {10000.f},
			.fovX {68.f},
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


	auto gbuffer = rendergraph.add_gbuffer("gbuffer");
	auto lighting = rendergraph.add_lighting("lighting");


	auto& deferredPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Pass", nyan::Renderpass::Type::Generic));

	auto& deferredRTPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Lighting-Pass", nyan::Renderpass::Type::Generic));

	auto& forwardPass = rendergraph.get_pass(rendergraph.add_pass("Forward-Pass", nyan::Renderpass::Type::Generic));

	auto& compositePass = rendergraph.get_pass(rendergraph.add_pass("Composite-Pass", nyan::Renderpass::Type::Generic));

	auto& imguiPass = rendergraph.get_pass(rendergraph.add_pass("Imgui-Pass", nyan::Renderpass::Type::Generic));

	nyan::MeshRenderer meshRenderer(device, registry, renderManager, deferredPass, gbuffer);
	nyan::DeferredRayShadowsLighting deferredLighting2(device, registry, renderManager, deferredRTPass, gbuffer, lighting);
	//nyan::DeferredLighting deferredLighting(device, registry, renderManager, deferredRTPass, gbuffer, lighting);
	nyan::ForwardMeshRenderer forwardMeshRenderer(device, registry, renderManager, forwardPass, lighting, gbuffer.depth);
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

	return ExitCode::Success;
}