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
	auto name = "SwapchainTest";
	nyan::Application application(name);

	auto& device = application.get_device();
	auto& window = application.get_window();

	auto& input = application.get_input();

	auto directory = getenv("USERPROFILE") / std::filesystem::path{ "Assets" };

	nyan::RenderManager renderManager(device, true, directory);
	nyan::CameraController cameraController(renderManager, input);
	auto& registry = renderManager.get_registry();

	Transform cameraTransform;


	float dirIntensity = 1.45f;
	float envIntensity = 1.0f;
	auto dirDir = Math::vec3{ 0.f, -1.f, 0.300f };
	auto envCol = Math::vec3{ 1.f, 1.f, 1.f };



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

	auto& imguiPass = rendergraph.get_pass(rendergraph.add_pass("Imgui-Pass", nyan::Renderpass::Type::Generic));

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