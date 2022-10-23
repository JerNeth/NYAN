#include <iostream>
#include <Util>
#include "Application.h"
#include "tiny_gltf.h"

#include "Renderer/Mesh.h"
#include "Renderer/Material.h"
#include "Renderer/Light.h"

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

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	std::filesystem::path file;
	file = "sponza-gltf-pbr/sponza.glb";
	//file = "glTF-Sample-Models/2.0/NormalTangentMirrorTest/glTF/NormalTangentMirrorTest.gltf";
	std::filesystem::path dir = "glTF-Sample-Models/2.0";
	//file = dir / "OrientationTest/glTF/OrientationTest.gltf";
	//file = dir / "BoomBoxWithAxes/glTF/BoomBoxWithAxes.gltf";
	//file = "coord.glb";
	//file = "sphere.glb";
	//file = "cornellFixed.gltf";
	std::filesystem::path path = directory / file;
	bool ret = false;
	
	if(file.extension() == ".glb")
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, path.string());
	else if(file.extension() == ".gltf")
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, path.string());

	if (!warn.empty())
		Utility::log_warning(warn);

	if (!err.empty())
		Utility::log_warning(err);

	auto& textureManager = renderManager.get_texture_manager();

	if (ret) {
		for (const auto& sampler : model.samplers) {
			//Ignore for now
		}
		for (const auto& light : model.lights) {
			assert(false);
		}
		for (const auto& node : model.nodes) {
		}
		std::vector<std::string> textureMap;
		for (const auto& texture : model.textures) {
			if (texture.source != -1) {
				auto& image = model.images[texture.source];
				if (!image.uri.empty()) {
					textureManager.request_texture(file.parent_path() / image.uri);
					textureMap.push_back(image.uri);
				}
				else {
					textureMap.push_back(image.name);
					assert(false);
				}
			}

		}
		auto texResolve = [&](int idx) {return (idx != -1) ? textureMap[idx] : ""; };
		auto alphaModeResolve = [&](const std::string& mode) {
			if (mode == "OPAQUE") return AlphaMode::Opaque;
			if (mode == "MASK") return AlphaMode::AlphaTest;
			if (mode == "BLEND") return AlphaMode::AlphaBlend;
			assert(false);
		};
		std::vector<nyan::MaterialId> materialMap;
		for (const auto& material : model.materials) {
			nyan::PBRMaterialData materialData{
				.name{material.name},
				.albedoTex{texResolve(material.pbrMetallicRoughness.baseColorTexture.index)},
				.emissiveTex {texResolve(material.emissiveTexture.index)},
				.roughnessMetalnessTex {texResolve(material.pbrMetallicRoughness.metallicRoughnessTexture.index)},
				.normalTex {texResolve(material.normalTexture.index)},
				.albedoFactor{material.pbrMetallicRoughness.baseColorFactor},
				.emissiveFactor {material.emissiveFactor},
				.alphaMode {alphaModeResolve(material.alphaMode)},
				.alphaCutoff {static_cast<float>(material.alphaCutoff)},
				.doubleSided {material.doubleSided},
				.metallicFactor {static_cast<float>(material.pbrMetallicRoughness.metallicFactor)},
				.roughnessFactor {static_cast<float>(material.pbrMetallicRoughness.roughnessFactor)}
			};
			materialMap.push_back(renderManager.get_material_manager().add_material(materialData));
		}
		auto test = [](uint32_t flags, uint32_t flag) {return (flags & flag) == flag; };
		std::vector<std::vector<nyan::MeshID>> meshMap;
		for (const auto& mesh : model.meshes) {
			size_t idx{ 0 };
			meshMap.emplace_back();
			auto& map = meshMap.back();
			for (const auto& primitive : mesh.primitives) {
				assert(primitive.mode == TINYGLTF_MODE_TRIANGLES);
				nyan::Mesh nMesh{
					.type {nyan::Mesh::RenderType::Opaque},
					.name {mesh.name + std::to_string(idx++)},
				};
				if (primitive.material != -1)
					nMesh.materialBinding = materialMap[primitive.material];
				{
					auto accessorId = primitive.indices;
					auto& accessor = model.accessors[accessorId];
					auto& bufferView = model.bufferViews[accessor.bufferView];
					assert(bufferView.buffer != -1);
					auto& buffer = model.buffers[bufferView.buffer];
					assert(!accessor.sparse.isSparse);
					assert(bufferView.byteLength);
					std::byte* data = reinterpret_cast<std::byte*>(buffer.data.data() + bufferView.byteOffset);
					auto stride = bufferView.byteStride;
					auto numComponents = tinygltf::GetNumComponentsInType(accessor.type);
					auto componentByteSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
					if (!stride)
						stride = numComponents * componentByteSize;
					else
						assert(false);
					nMesh.indices.reserve(accessor.count);
					assert(numComponents == 1);
					size_t offset{ 0 };
					for (size_t i{ 0 }; i < accessor.count; ++i) {
						auto* ptr = data + offset;
						if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
							nMesh.indices.push_back(*reinterpret_cast<uint8_t*>(ptr));
						else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
							nMesh.indices.push_back(*reinterpret_cast<uint16_t*>(ptr));
						else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
							nMesh.indices.push_back(*reinterpret_cast<uint32_t*>(ptr));
						else
							assert(false);
						offset += stride;
						assert(offset <= bufferView.byteLength);
					}
				}

				for (const auto& [attribute, accessorId]: primitive.attributes) {

					auto& accessor = model.accessors[accessorId];
					auto& bufferView = model.bufferViews[accessor.bufferView];
					assert(bufferView.buffer != -1);
					auto& buffer = model.buffers[bufferView.buffer];
					assert(!accessor.sparse.isSparse);
					assert(bufferView.byteLength);
					std::byte* data = reinterpret_cast<std::byte*>(buffer.data.data() + bufferView.byteOffset);
					auto stride = bufferView.byteStride;
					auto numComponents = tinygltf::GetNumComponentsInType(accessor.type);
					auto componentByteSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
					if (!stride)
						stride = numComponents * componentByteSize;


					size_t offset{ 0 };
					if (attribute == "POSITION") {
						decltype(nMesh.positions)::value_type position;

						nMesh.positions.reserve(accessor.count);
						for (size_t i{ 0 }; i < accessor.count; ++i) {
							auto* ptr = data + offset;
							for (size_t j{ 0 }; j < numComponents; ++j) {
								if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
									auto* fPtr = reinterpret_cast<float*>(ptr) + j;
									position[j] = *fPtr;
								}
								else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
									auto* dPtr = reinterpret_cast<double*>(ptr) + j;
									position[j] = *dPtr;
								}
								else
									assert(false);
							}
							nMesh.positions.push_back(position);
							offset += stride;
							assert(offset <= bufferView.byteLength);
						}
					} else if (attribute == "NORMAL") {
						decltype(nMesh.normals)::value_type normal;

						nMesh.normals.reserve(accessor.count);
						for (size_t i{ 0 }; i < accessor.count; ++i) {
							auto* ptr = data + offset;
							for (size_t j{ 0 }; j < numComponents; ++j) {
								if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
									auto* fPtr = reinterpret_cast<float*>(ptr) + j;
									normal[j] = *fPtr;
								}
								else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
									auto* dPtr = reinterpret_cast<double*>(ptr) + j;
									normal[j] = *dPtr;
								}
								else
									assert(false);
							}
							nMesh.normals.push_back(normal);
							offset += stride;
							assert(offset <= bufferView.byteLength);
						}
					} else if (attribute == "TANGENT") {
						decltype(nMesh.tangents)::value_type tangent;

						nMesh.tangents.reserve(accessor.count);
						for (size_t i{ 0 }; i < accessor.count; ++i) {
							auto* ptr = data + offset;
							for (size_t j{ 0 }; j < numComponents; ++j) {
								if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
									auto* fPtr = reinterpret_cast<float*>(ptr) + j;
									tangent[j] = *fPtr;
								}
								else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
									auto* dPtr = reinterpret_cast<double*>(ptr) + j;
									tangent[j] = *dPtr;
								}
								else
									assert(false);
							}
							nMesh.tangents.push_back(tangent);
							offset += stride;
							assert(offset <= bufferView.byteLength);
						}
					} else if (attribute == "TEXCOORD_0") {
						decltype(nMesh.uvs)::value_type uv;

						nMesh.uvs.reserve(accessor.count);
						for (size_t i{ 0 }; i < accessor.count; ++i) {
							auto* ptr = data + offset;
							for (size_t j{ 0 }; j < numComponents; ++j) {
								if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
									auto* fPtr = reinterpret_cast<float*>(ptr) + j;
									uv[j] = *fPtr;
								}
								else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
									auto* dPtr = reinterpret_cast<double*>(ptr) + j;
									uv[j] = *dPtr;
								}
								else
									assert(false);
							}
							nMesh.uvs.push_back(uv);
							offset += stride;
							assert(offset <= bufferView.byteLength);
						}
					}
					else {
						assert(false && "unsupported attribute");
					}

				}
				auto &material = renderManager.get_material_manager().get_material(materialMap[primitive.material]);
				if (nMesh.tangents.empty()) {
					Utility::log_warning().format("{}: has no tangents, skipping mesh", nMesh.name);
					for (auto i = 0; i < nMesh.normals.size(); ++i)
						nMesh.tangents.push_back(decltype(nMesh.tangents)::value_type{-1, 0,0, 1});
					//continue;
				}
				if (nMesh.uvs.empty()) {
					Utility::log_warning().format("{}: has no uvs, skipping mesh", nMesh.name);
					//continue;
				}
				if (nMesh.normals.empty()) {
					Utility::log_warning().format("{}: has no normals, skipping mesh", nMesh.name);
					//continue;
				}
				if (nMesh.positions.empty()) {
					Utility::log_warning().format("{}: has no positions, skipping mesh", nMesh.name);
					//continue;
				}

				if (test(material.flags, nyan::shaders::MATERIAL_ALPHA_TEST_FLAG))
					nMesh.type = nyan::Mesh::RenderType::AlphaTest;
				else if (test(material.flags, nyan::shaders::MATERIAL_ALPHA_BLEND_FLAG))
					nMesh.type = nyan::Mesh::RenderType::AlphaBlend;

				map.emplace_back(renderManager.get_mesh_manager().add_mesh(nMesh));
			}
		}

		for (const auto& scene : model.scenes) {
			for (const auto nodeId : scene.nodes) {
				std::vector<int> nodes{ nodeId };
				std::vector<entt::entity> parents{};

				while (!nodes.empty()) {
					auto entity = registry.create();
					const auto* node = &model.nodes[nodes.back()];
					nodes.pop_back();
					if (!parents.empty()) {
						registry.emplace<Parent>(entity,
							Parent{
								.parent {parents.back()},
							});
						parents.pop_back();
					}
					for (auto child : node->children) {
						nodes.push_back(child);
						parents.push_back(entity);
					}
					if (!node->name.empty())
						registry.emplace<std::string>(entity, node->name);
					//node.
					Math::vec3 pos{};
					Math::vec3 scale{1.f};
					Math::vec3 orientation{};
					if (!node->translation.empty())
						pos = Math::vec3{ node->translation };
					if (!node->scale.empty())
						scale = Math::vec3{ node->scale };
					if (!node->rotation.empty())
						orientation = Math::vec3{ Math::quat(Math::vec4(node->rotation)).to_euler_angles() };
					if (!node->matrix.empty()) {
						Math::mat44 mat{ node->matrix };
						pos = Math::vec3{ mat.col(3) };
						Math::vec3 tmpX{ mat.col(0) };
						Math::vec3 tmpY{ mat.col(1) };
						Math::vec3 tmpZ{ mat.col(2) };
						scale = Math::vec3{ tmpX.L2_norm(), tmpY.L2_norm(), tmpZ.L2_norm() };
						tmpX *= 1.f / scale.x();
						tmpY *= 1.f / scale.y();
						tmpZ *= 1.f / scale.z();
						mat.set_col(tmpX, 0);
						mat.set_col(tmpY, 0);
						mat.set_col(tmpZ, 0);
						Math::quat a{ mat };
						orientation = Math::vec3{ a.to_euler_angles() };
					}

					registry.emplace<Transform>(entity,
						Transform{
							.position{pos},
							.scale{scale},
							.orientation{orientation},
						});
					if (node->mesh != -1) {

						for (auto meshId : meshMap[node->mesh]) {
							auto meshEntity = registry.create();
							const auto& mesh = renderManager.get_mesh_manager().get_shader_mesh(meshId);
							const auto& material = renderManager.get_material_manager().get_material(mesh.materialId);
							registry.emplace<MeshID>(meshEntity, meshId);
							registry.emplace<MaterialId>(meshEntity, mesh.materialId);
							auto instance = InstanceData{
									.transform{
										.transformMatrix = Math::Mat<float, 3, 4, false>::identity()
									}
							};
							if(!model.meshes[node->mesh].name.empty())
								registry.emplace<std::string>(meshEntity, model.meshes[node->mesh].name);

							instance.instance.instanceCustomIndex = meshId;
							registry.emplace<InstanceId>(meshEntity, renderManager.get_instance_manager().add_instance(instance));
							registry.emplace<Transform>(meshEntity,
								Transform{
									.position{},
									.scale{1.f},
									.orientation{},
								});

							registry.emplace<Parent>(meshEntity,
								Parent{
									.parent {entity},
								});
							if (test(material.flags, nyan::shaders::MATERIAL_ALPHA_TEST_FLAG))
								if (test(material.flags, nyan::shaders::MATERIAL_DOUBLE_SIDED_FLAG))
									registry.emplace<DeferredDoubleSidedAlphaTest>(meshEntity);
								else
									registry.emplace<DeferredAlphaTest>(meshEntity);
							else if (test(material.flags, nyan::shaders::MATERIAL_ALPHA_BLEND_FLAG))
								registry.emplace<ForwardTransparent>(meshEntity);
							else
								if (test(material.flags, nyan::shaders::MATERIAL_DOUBLE_SIDED_FLAG))
									registry.emplace<DeferredDoubleSided>(meshEntity);
								else
									registry.emplace<Deferred>(meshEntity);
						}
					}
				}

			}
		}
	}




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
			.visualization {false}
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
			.intensity {1.0},
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