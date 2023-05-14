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

enum class ExitCode : int {
	Success = EXIT_SUCCESS,
	Failure = EXIT_FAILURE
};

int main(const int argc, char const * const * const argv)
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
	nyan::DDGIManager::DDGIVolumeParameters ddgiParams;

	//Params
	std::string scene = "Sponza";
	//std::string scene = "Cornell";
	bool ddgiRestir = false;
	bool ddgiRestirRecurse = false;
	bool restirValidation = false;
	bool spatialReuse = false;
	bool useMoments = false;
	bool dynamicRayAllocation = false;
	//uint32_t recordingLength = 0;
	uint32_t recordingLength = 0;
	uint32_t recordingOffset = 0;
	uint32_t usedEstimator = 0;
	uint32_t numSamples = 256;
	std::filesystem::path recordingDir = scene + "Recording2";
	std::optional<std::filesystem::path> assetPath{std::nullopt};
	bool quitAfterRecording = false;

	
	for (auto it = args.begin(); it != args.end(); ++it) {
		bool last = (it + 1) == args.end();
		if (*it == "-q" || *it == "-Q" || *it == "-Quit") {
			quitAfterRecording = true;
		}
		if (*it == "-r" || *it == "-R" || *it == "-Restir") {
			ddgiRestir = true;
		}
		if (*it == "-rr" || *it == "-RR" || *it == "-RestirRecurse") {
			ddgiRestirRecurse = true;
		}
		if (*it == "-m" || *it == "-M" || *it == "-Moments") {
			useMoments = true;
		}
		if (*it == "-a" || *it == "-A" || *it == "-Allocation") {
			dynamicRayAllocation = true;
		}
		if (*it == "-v" || *it == "-V" || *it == "-Validation") {
			restirValidation = true;
		}
		if (*it == "-re" || *it == "-Re" || *it == "-Reuse") {
			spatialReuse = true;
		}
		if (last)
			continue;
		auto next = (it + 1);
		if (*it == "-s" || *it == "-S" || *it == "-Scene") {
			scene = *next;
			it++;
		}
		else if (*it == "-d" || *it == "-D" || *it == "-Dir") {
			recordingDir = *next;
			it++;
		}
		else if (*it == "-asset") {
			assetPath = *next;
			it++;
		}
		else if (*it == "-l" || *it == "-L" || *it == "-Length") {
			uint32_t uint;
			auto uintResult = std::from_chars(next->data(), next->data() + next->size(), uint);
			if (uintResult.ec != std::errc::invalid_argument) {
				recordingLength = uint;
			}
			it++;
		}
		else if (*it == "-e" || *it == "-E" || *it == "-Estimator") {
			uint32_t uint;
			auto uintResult = std::from_chars(next->data(), next->data() + next->size(), uint);
			if (uintResult.ec != std::errc::invalid_argument) {
				usedEstimator = uint;
			}
			it++;
		}
		else if (*it == "-n" || *it == "-N" || *it == "-NumSamples") {
			uint32_t uint;
			auto uintResult = std::from_chars(next->data(), next->data() + next->size(), uint);
			if (uintResult.ec != std::errc::invalid_argument) {
				numSamples = uint;
			}
			it++;
		}
		else if (*it == "-o" || *it == "-O" || *it == "-Offset") {
			uint32_t uint;
			auto uintResult = std::from_chars(next->data(), next->data() + next->size(), uint);
			if (uintResult.ec != std::errc::invalid_argument) {
				recordingOffset = uint;
			}
			it++;
		}
	}


	if(recordingLength)
		std::filesystem::create_directory(recordingDir);

	float dirIntensity = 1.45f;
	float envIntensity = 1.0f;
	auto dirDir = Math::vec3{ 0.f, -1.f, 0.300f };
	auto envCol = Math::vec3{ 1.f, 1.f, 1.f };
	if (scene == "Sponza" || scene == "Sponza1" || scene == "Sponza2") {
		file = "Sponza/glTF/Sponza.gltf";
		dir = "glTF-Sample-Models/2.0";
		file = dir / file;
		cameraTransform = Transform {
					.position{7.46, 5.07f, 0.92f},
					.scale{1.f},
					.orientation{-13.22f, -295.23f, 0.f},
		};

		ddgiParams = nyan::DDGIManager::DDGIVolumeParameters{
				.spacing {1.02f, 0.5f, 0.45f},
				//.origin {-0.4f - 12.f * 1.02f, 5.4f - 0.5f * 11.f, -0.25f - 0.45f * 12.f},
				.origin {-0.4f - 12.f * 1.02f, 0.15f, -5.590f},
				.probeCount {24, 22, 24},
				.raysPerProbe {numSamples},
				.depthBias {0.1},
				.depthViewBias {0.3},
				.hysteresis {0.97},
				.irradianceThreshold {0.2},
				.lightToDarkThreshold {0.08f},
				//.lightToDarkThreshold {2.0},
				.visualizerRadius {0.1},
				.minFrontFaceDistance {0.1},
				.visualization {false},
				.relocationEnabled {true},
		}; //Sponza
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
		ddgiParams = nyan::DDGIManager::DDGIVolumeParameters{
				.spacing {1.5f, 1.0f, 1.0f},
				.origin {-22.64f, 0.1f, -11.65f},
				.probeCount {48, 13, 26},
				.raysPerProbe {numSamples},
				.depthBias {0.1},
				.depthViewBias {0.3},
				.hysteresis {0.97},
				.irradianceThreshold {0.2},
				.lightToDarkThreshold {0.08f},
				//.lightToDarkThreshold {2.0},
				.visualizerRadius {0.1},
				.minFrontFaceDistance {0.1},
				.visualization {false},
				.relocationEnabled {true},
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
		ddgiParams = nyan::DDGIManager::DDGIVolumeParameters{
				.spacing {1.8f, 1.8f, 1.8f},
				.origin {-4.65f, -4.9f, -4.65f},
				.probeCount {6, 6, 7},
				.raysPerProbe {numSamples},
				.depthBias {0.1},
				.depthViewBias {0.3},
				.hysteresis {0.97},
				.irradianceThreshold {0.2},
				.lightToDarkThreshold {0.08f},
				//.lightToDarkThreshold {2.0},
				.visualizerRadius {0.1},
				.minFrontFaceDistance {0.1},
				.visualization {false},
				.relocationEnabled {true},
		};
		envCol = Math::vec3{ 0.f, 0.f, 0.f };
		dirIntensity = 0.f;
		envIntensity = 0.f;
	}
	if (assetPath)
		file = *assetPath;

	ddgiParams.useMoments = useMoments;
	ddgiParams.usedEstimator = usedEstimator;
	if(!usedEstimator)
		ddgiParams.dynamicRayAllocation = dynamicRayAllocation;

	ddgiParams.fixedRayCount = 8;
	ddgiParams.raysPerProbe = numSamples;

	//file = "sponza.glb";
	//file = "Sponza/glTF/Sponza.gltf";
	//file = "TestScene.gltf";
	//file = "SanMiguel.gltf";
	//file = "SunTemple.glb";
	//file = "SunTemple.gltf";
	//file = "occluderscene.gltf";
	//file = "NewSponza_Main_glTF_002.gltf";
	//file = "glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF/NormalTangentMirrorTest.gltf";
	//dir = "glTF-Sample-Models/2.0";
	//dir = "TestScene";
	//dir = "SanMiguel";
	//dir = "SanMiguel2";
	//dir = "SunTemple";
	//dir = "bedroom";
	//dir = "sponza-gltf-pbr";
	//dir = "Main.1_Sponza";
	//file = dir / "OrientationTest/glTF/OrientationTest.gltf";
	//file = dir / "BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf";
	//file = "coord.glb";
	//file = "sphere.glb";
	//file = "cornellFixed.gltf";
	std::filesystem::path path = directory / file;
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

	//registry.emplace<nyan::DDGIManager::DDGIVolumeParameters>(parent, nyan::DDGIManager::DDGIVolumeParameters{
	//		.visualization {false}
	//	});
	registry.emplace<nyan::DDGIManager::DDGIVolumeParameters>(parent, ddgiParams); 
	registry.emplace<nyan::DDGIReSTIRManager::DDGIReSTIRVolumeParameters>(parent, nyan::DDGIReSTIRManager::DDGIReSTIRVolumeParameters{
			.spacing {ddgiParams.spacing},
			.origin {ddgiParams.origin},
			.probeCount {ddgiParams.probeCount},
			.samplesPerProbe {numSamples},
			.validationEnabled {restirValidation},
			.recurse {ddgiRestirRecurse},
			.spatialReuse {spatialReuse},
			.spatialReuseValidation {spatialReuse},
			.maxPathLength{ddgiRestirRecurse? 1u: 10u}
		}); //Sponza

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


	auto& ddgiPass = rendergraph.get_pass(rendergraph.add_pass("DDGI-Pass", nyan::Renderpass::Type::Generic));
	auto& deferredPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Pass", nyan::Renderpass::Type::Generic));

	auto& deferredRTPass = rendergraph.get_pass(rendergraph.add_pass("Deferred-Lighting-Pass", nyan::Renderpass::Type::Generic));

	auto& forwardPass = rendergraph.get_pass(rendergraph.add_pass("Forward-Pass", nyan::Renderpass::Type::Generic));

	auto& compositePass = rendergraph.get_pass(rendergraph.add_pass("Composite-Pass", nyan::Renderpass::Type::Generic));

	auto& imguiPass = rendergraph.get_pass(rendergraph.add_pass("Imgui-Pass", nyan::Renderpass::Type::Generic));

	nyan::DDGIRenderer ddgiRenderer(device, registry, renderManager, ddgiPass);
	nyan::DDGIReSTIRRenderer ddgiReSTIRRenderer(device, registry, renderManager, ddgiPass);
	nyan::MeshRenderer meshRenderer(device, registry, renderManager, deferredPass, gbuffer);
	nyan::DeferredRayShadowsLighting deferredLighting2(device, registry, renderManager, deferredRTPass, gbuffer, lighting);
	nyan::ForwardMeshRenderer forwardMeshRenderer(device, registry, renderManager, forwardPass, lighting, gbuffer.depth);
	nyan::DDGIVisualizer ddgiVisualizer(device, registry, renderManager, forwardPass, lighting, gbuffer.depth);
	//nyan::DeferredLighting deferredLighting(device, registry, renderManager, deferredLightingPass);
	nyan::LightComposite lightComposite(device, registry, renderManager, compositePass, lighting);
	nyan::ImguiRenderer imgui(device, registry, renderManager, imguiPass, &window);
	rendergraph.build();
	if(recordingLength)
		lightComposite.queue_recording(recordingDir, recordingLength, recordingOffset);
	if (quitAfterRecording)
		application.quit_after(recordingLength + recordingOffset + 1);
	//uint64_t frameCount = 0;
	application.each_update([&](std::chrono::nanoseconds dt)
		{
			//if (frameCount == 99) {
			//	ddgiRenderer.clear();
			//}
			//if (frameCount == 100) {
			//	registry.get<SkyLight>(parent).intensity = envIntensity;
			//	registry.get<Directionallight>(parent).intensity = dirIntensity;

			//}
			//frameCount++;
			
			cameraController.update(dt);
			renderManager.update(dt);
		});
	application.each_frame_begin([&]()
		{
			//ImGui first here since we might want to use ImGui in other begin_frames
			imgui.begin_frame();

			renderManager.get_profiler().begin_frame();
			ImGui::Begin("Render Config");
			ImGui::Checkbox("Use DDGIReSTIR", &ddgiRestir);
			if (ImGui::Button("Screenshot DDGI ReSTIR Buffer"))
				ddgiReSTIRRenderer.dump_to_disk();
			if (ImGui::Button("Screenshot DDGI Buffer"))
				ddgiRenderer.screen_shot();
			if (ImGui::Button("Screenshot"))
				lightComposite.queue_screenshot();
			if (ImGui::Button("Clear DDGI ReSTIR Buffers"))
				ddgiReSTIRRenderer.clear_buffers();
			if (ImGui::Button("Clear DDGI Buffers"))
				ddgiRenderer.clear();
			static constexpr std::array tonemappingOptions = {"None", "ACESFilm", "Reinhard" , "Uncharted" };
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
			ddgiRenderer.set_enable(!ddgiRestir);
			ddgiReSTIRRenderer.set_enable(ddgiRestir);
			deferredLighting2.set_use_ddgi_restir(ddgiRestir);

			ddgiRenderer.begin_frame();
			ddgiReSTIRRenderer.begin_frame();
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