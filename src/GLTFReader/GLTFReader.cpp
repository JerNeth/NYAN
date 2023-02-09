#include "GLTFReader/GLTFReader.hpp"
#include "tiny_gltf.h"
#include "Renderer/MeshRenderer.h"
#include <vector>

nyan::GLTFReader::GLTFReader(nyan::RenderManager& renderManager) :
	r_renderManager(renderManager)
{

}

void nyan::GLTFReader::load_file(const std::filesystem::path& path)
{
	auto& textureManager = r_renderManager.get_texture_manager();
	auto& materialManager = r_renderManager.get_material_manager();
	auto& instanceManager = r_renderManager.get_instance_manager();
	auto& meshManager = r_renderManager.get_mesh_manager();
	auto& registry = r_renderManager.get_registry();
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string err;
	std::string warn; 
	bool ret = false;

	if (path.extension() == ".glb")
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, path.string());
	else if (path.extension() == ".gltf")
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, path.string());

	if (!warn.empty())
		Utility::log_warning(warn);

	if (!err.empty())
		Utility::log_warning(err);
	if (!ret)
		return;

	for (const auto& material : model.materials) {
		assert(material.pbrMetallicRoughness.baseColorTexture.texCoord == 0);
		assert(material.emissiveTexture.texCoord == 0);
		assert(material.pbrMetallicRoughness.metallicRoughnessTexture.texCoord == 0);
		assert(material.normalTexture.texCoord == 0);
		assert(!material.pbrMetallicRoughness.baseColorTexture.extras.Size());
		assert(material.pbrMetallicRoughness.baseColorTexture.extensions.empty());
		if (material.pbrMetallicRoughness.baseColorTexture.index != -1) {
			assert(!model.textures[material.pbrMetallicRoughness.baseColorTexture.index].extras.Size());
			assert(model.textures[material.pbrMetallicRoughness.baseColorTexture.index].extensions.empty());
		}
	}
	//for (const auto& sampler : model.samplers) {
	//	//Ignore for now
	//}
	//for (const auto& light : model.lights) {
	//	//assert(false);
	//}
	//for (const auto& node : model.nodes) {
	//	//
	//}
	std::vector<std::string> textureMap;
	for (const auto& texture : model.textures) {
		if (texture.source != -1) {
			auto& image = model.images[texture.source];
			if (!image.uri.empty()) {
				textureManager.request_texture(path.parent_path() / image.uri);
				textureMap.push_back(image.uri);
			}
			else {
				textureManager.request_texture(nyan::TextureManager::TextureInfo{ 
					.name {image.name}, 
					.width {static_cast<uint32_t>(image.width)},
					.height {static_cast<uint32_t>(image.height)},
					.components {static_cast<uint32_t>(image.component)},
					.bitsPerChannel {static_cast<uint32_t>(image.bits)},
					.sRGB{true},
					}, image.image);
				textureMap.push_back(image.name);
				assert(image.width > 0);
				assert(image.height > 0);
				assert(image.component > 0);
				assert(image.bits > 0);
			}
		}

	}
	auto texResolve = [&](int idx) {return (idx != -1) ? textureMap[idx] : ""; };
	auto alphaModeResolve = [&](const std::string& mode) {
		if (mode == "OPAQUE") return AlphaMode::Opaque;
		if (mode == "MASK") return AlphaMode::AlphaTest;
		if (mode == "BLEND") return AlphaMode::AlphaBlend;
		assert(false);
		return AlphaMode::Opaque;
	};
	std::vector<nyan::MaterialId> materialMap;
	for (size_t matIdx{ 0 }; matIdx < model.materials.size(); ++matIdx) {
		const auto& material = model.materials[matIdx];
		std::string materialName = material.name;
		if (materialName.empty())
			materialName = "Material_" + std::to_string(matIdx);
		nyan::PBRMaterialData materialData{
			.name{materialName},
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
		materialMap.push_back(materialManager.add_material(materialData));
	}
	auto test = [](uint32_t flags, uint32_t flag) {return (flags & flag) == flag; };
	std::vector<std::vector<nyan::MeshID>> meshMap;

	for (size_t meshIdx{ 0 }; meshIdx < model.meshes.size(); ++meshIdx) {
		const auto& mesh = model.meshes[meshIdx];
		size_t idx{ 0 };
		meshMap.emplace_back();
		auto& map = meshMap.back();
		std::string meshName = mesh.name;
		if (meshName.empty())
			meshName = "Mesh_" + std::to_string(meshIdx);
		for (const auto& primitive : mesh.primitives) {
			assert(primitive.mode == TINYGLTF_MODE_TRIANGLES);
			nyan::Mesh nMesh{
				.type {nyan::Mesh::RenderType::Opaque},
				.name {meshName + "_Primitive_" + std::to_string(idx++)},
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
				assert(accessor.type == TINYGLTF_TYPE_SCALAR);
				assert(bufferView.byteLength);
				std::byte* data = reinterpret_cast<std::byte*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
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
				assert(buffer.data.size() >= offset + bufferView.byteOffset + accessor.byteOffset);
			}

			for (const auto& [attribute, accessorId] : primitive.attributes) {

				auto& accessor = model.accessors[accessorId];
				auto& bufferView = model.bufferViews[accessor.bufferView];
				assert(bufferView.buffer != -1);
				auto& buffer = model.buffers[bufferView.buffer];
				assert(!accessor.sparse.isSparse);
				assert(bufferView.byteLength);
				std::byte* data = reinterpret_cast<std::byte*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
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
								position[j] = static_cast<decltype(nMesh.positions)::value_type::value_type>(*fPtr);
							}
							else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
								auto* dPtr = reinterpret_cast<double*>(ptr) + j;
								position[j] = static_cast<decltype(nMesh.positions)::value_type::value_type>(*dPtr);
							}
							else
								assert(false);
						}
						nMesh.positions.push_back(position);
						offset += stride;
						assert(offset <= bufferView.byteLength);
					}
				}
				else if (attribute == "NORMAL") {
					decltype(nMesh.normals)::value_type normal;

					nMesh.normals.reserve(accessor.count);
					for (size_t i{ 0 }; i < accessor.count; ++i) {
						auto* ptr = data + offset;
						for (size_t j{ 0 }; j < numComponents; ++j) {
							if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
								auto* fPtr = reinterpret_cast<float*>(ptr) + j;
								normal[j] = static_cast<decltype(nMesh.normals)::value_type::value_type>(*fPtr);
							}
							else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
								auto* dPtr = reinterpret_cast<double*>(ptr) + j;
								normal[j] = static_cast<decltype(nMesh.normals)::value_type::value_type>(*dPtr);
							}
							else
								assert(false);
						}
						nMesh.normals.push_back(normal);
						offset += stride;
						assert(offset <= bufferView.byteLength);
					}
				}
				else if (attribute == "TANGENT") {
					decltype(nMesh.tangents)::value_type tangent;

					nMesh.tangents.reserve(accessor.count);
					for (size_t i{ 0 }; i < accessor.count; ++i) {
						auto* ptr = data + offset;
						for (size_t j{ 0 }; j < numComponents; ++j) {
							if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
								auto* fPtr = reinterpret_cast<float*>(ptr) + j;
								tangent[j] = static_cast<decltype(nMesh.tangents)::value_type::value_type>(*fPtr);
							}
							else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
								auto* dPtr = reinterpret_cast<double*>(ptr) + j;
								tangent[j] = static_cast<decltype(nMesh.tangents)::value_type::value_type>(*dPtr);
							}
							else
								assert(false);
						}
						nMesh.tangents.push_back(tangent);
						offset += stride;
						assert(offset <= bufferView.byteLength);
					}
				}
				else if (attribute == "TEXCOORD_0") {
					decltype(nMesh.uvs0)::value_type uv;

					nMesh.uvs0.reserve(accessor.count);
					for (size_t i{ 0 }; i < accessor.count; ++i) {
						auto* ptr = data + offset;
						for (size_t j{ 0 }; j < numComponents; ++j) {
							if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
								auto* fPtr = reinterpret_cast<float*>(ptr) + j;
								uv[j] = static_cast<decltype(nMesh.uvs0)::value_type::value_type>(*fPtr);
							}
							else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
								auto* dPtr = reinterpret_cast<double*>(ptr) + j;
								uv[j] = static_cast<decltype(nMesh.uvs0)::value_type::value_type>(*dPtr);
							}
							else
								assert(false);
						}
						nMesh.uvs0.push_back(uv);
						offset += stride;
						assert(offset <= bufferView.byteLength);
					}
				}
				else if (attribute == "TEXCOORD_1") {
					decltype(nMesh.uvs1)::value_type uv;
					nMesh.uvs1.reserve(accessor.count);
					for (size_t i{ 0 }; i < accessor.count; ++i) {
						auto* ptr = data + offset;
						for (size_t j{ 0 }; j < numComponents; ++j) {
							if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
								auto* fPtr = reinterpret_cast<float*>(ptr) + j;
								uv[j] = static_cast<decltype(nMesh.uvs1)::value_type::value_type>(*fPtr);
							}
							else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
								auto* dPtr = reinterpret_cast<double*>(ptr) + j;
								uv[j] = static_cast<decltype(nMesh.uvs1)::value_type::value_type>(*dPtr);
							}
							else
								assert(false);
						}
						nMesh.uvs1.push_back(uv);
						offset += stride;
						assert(offset <= bufferView.byteLength);
					}
					Utility::log_warning().format("Mesh: {}, TEXCOORD_1 unsupported attribute", nMesh.name);
				}
				else if (attribute == "TEXCOORD_2") {
					decltype(nMesh.uvs2)::value_type uv;
					nMesh.uvs2.reserve(accessor.count);
					for (size_t i{ 0 }; i < accessor.count; ++i) {
						auto* ptr = data + offset;
						for (size_t j{ 0 }; j < numComponents; ++j) {
							if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
								auto* fPtr = reinterpret_cast<float*>(ptr) + j;
								uv[j] = static_cast<decltype(nMesh.uvs2)::value_type::value_type>(*fPtr);
							}
							else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
								auto* dPtr = reinterpret_cast<double*>(ptr) + j;
								uv[j] = static_cast<decltype(nMesh.uvs2)::value_type::value_type>(*dPtr);
							}
							else
								assert(false);
						}
						nMesh.uvs2.push_back(uv);
						offset += stride;
						assert(offset <= bufferView.byteLength);
					}
					Utility::log_warning().format("Mesh: {}, TEXCOORD_2 unsupported attribute", nMesh.name);
				}
				else if(attribute == "COLOR_0") {
					Utility::log_warning().format("Mesh: {}, COLOR_0 unsupported attribute",nMesh.name);
				}
				else {
					assert(false && "unsupported attribute");
				}
				assert(buffer.data.size() >= offset + bufferView.byteOffset + accessor.byteOffset);


			}
			auto& material = materialManager.get_material(materialMap[primitive.material]);
			if (nMesh.tangents.empty()) {
				Utility::log_warning().format("{}: has no tangents, skipping mesh", nMesh.name);
				for (auto i = 0; i < nMesh.normals.size(); ++i)
					nMesh.tangents.push_back(decltype(nMesh.tangents)::value_type{ -1, 0,0, 1 });
				//continue;
			}
			if (nMesh.uvs0.empty()) {
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
			assert(nMesh.tangents.size() == nMesh.uvs0.size());
			assert(nMesh.normals.size() == nMesh.uvs0.size());
			assert(nMesh.positions.size() == nMesh.uvs0.size());

			if (test(material.flags, nyan::shaders::MATERIAL_ALPHA_TEST_FLAG))
				nMesh.type = nyan::Mesh::RenderType::AlphaTest;
			else if (test(material.flags, nyan::shaders::MATERIAL_ALPHA_BLEND_FLAG))
				nMesh.type = nyan::Mesh::RenderType::AlphaBlend;

			map.emplace_back(meshManager.add_mesh(nMesh));
		}
	}
	for (const auto& scene : model.scenes) {
		auto root = registry.create();
		if (!scene.name.empty())
			registry.emplace<std::string>(root, scene.name);
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
				else {
					registry.emplace<Parent>(entity,
						Parent{
							.parent {root},
						});
				}
				for (auto child : node->children) {
					nodes.push_back(child);
					parents.push_back(entity);
				}
				if (!node->name.empty())
					registry.emplace<std::string>(entity, node->name);
				//node.
				Math::vec3 pos{};
				Math::vec3 scale{ 1.f };
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
						const auto& mesh = meshManager.get_shader_mesh(meshId);
						const auto& material = materialManager.get_material(mesh.materialId);
						registry.emplace<MeshID>(meshEntity, meshId);
						registry.emplace<MaterialId>(meshEntity, mesh.materialId);
						auto instance = InstanceData{
								.transform{
									.transformMatrix = Math::Mat<float, 3, 4, false>::identity()
								}
						};
						if (!model.meshes[node->mesh].name.empty())
							registry.emplace<std::string>(meshEntity, model.meshes[node->mesh].name);
						else
							registry.emplace<std::string>(meshEntity, "Mesh_" + std::to_string(node->mesh));

						instance.instance.instanceCustomIndex = meshId;
						registry.emplace<InstanceId>(meshEntity, instanceManager.add_instance(instance));
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
