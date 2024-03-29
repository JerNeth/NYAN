#include "Renderer/MeshManager.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/CommandBuffer.h"

using namespace vulkan;
using namespace nyan;

nyan::MeshManager::MeshManager(vulkan::LogicalDevice& device, nyan::MaterialManager& materialManager, bool buildAccelerationStructures) :
	DataManager(device),
	r_materialManager(materialManager),
	m_builder(std::make_unique<AccelerationStructureBuilder>(r_device)),
	m_buildAccs(buildAccelerationStructures)
{
}
nyan::MeshManager::~MeshManager()
{
}
nyan::MeshID nyan::MeshManager::add_mesh(const nyan::Mesh& data)
{
	std::vector<vulkan::InputData> inputData;
	std::vector<uint32_t> offsets;
	inputData.push_back(InputData{ .ptr = data.indices.data(), .size = data.indices.size() * sizeof(decltype(data.indices)::value_type) });
	inputData.push_back(InputData{ .ptr = data.positions.data(), .size = data.positions.size() * sizeof(decltype(data.positions)::value_type) });
	inputData.push_back(InputData{ .ptr = data.uvs0.data(), .size = data.uvs0.size() * sizeof(decltype(data.uvs0)::value_type) });
	inputData.push_back(InputData{ .ptr = data.normals.data(), .size = data.normals.size() * sizeof(decltype(data.normals)::value_type) });
	inputData.push_back(InputData{ .ptr = data.tangents.data(), .size = data.tangents.size() * sizeof(decltype(data.tangents)::value_type) });
	uint32_t offset = 0;
	for (auto& inputDate : inputData) {
		inputDate.stride = Utility::align_up(inputDate.size, 16ull);
		offsets.push_back(offset);
		offset += static_cast<uint32_t>(inputDate.stride);
	}
	vulkan::BufferInfo info{
		.size = offset,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | 
				VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.offset = 0,
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY

	};
	if (m_buildAccs) {
		info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 
			| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}
	VkIndexType indexType { VK_INDEX_TYPE_UINT32 };
	if constexpr (std::is_same_v<decltype(data.indices)::value_type, uint32_t>) {
		indexType = VK_INDEX_TYPE_UINT32;
	}
	if constexpr (std::is_same_v<decltype(data.indices)::value_type, uint16_t>) {
		indexType = VK_INDEX_TYPE_UINT16;
	}

	MeshManager::Mesh mesh{
		.buffer = r_device.create_buffer(info, inputData),
		.offset {0},
		.mesh {
			.indexCount {static_cast<uint32_t>(data.indices.size())},
			.firstIndex {0},
			.vertexOffset {0},
			.firstInstance {0},

			.indexBuffer {mesh.buffer->get_handle()},
			.indexOffset {offsets[0]},
			.indexType {indexType},
			.vertexBuffers {
				.positionBuffer {mesh.buffer->get_handle()},
				.texCoordBuffer {mesh.buffer->get_handle()},
				.normalBuffer {mesh.buffer->get_handle()},
				.tangentBuffer {mesh.buffer->get_handle()},
			},
			.vertexOffsets {
				.positionOffset {offsets[1]},
				.texCoordOffset {offsets[2]},
				.normalOffset {offsets[3]},
				.tangentOffset {offsets[4]},
			},
		}
	};
	auto addr = mesh.buffer->get_address();

	auto id = add(nyan::shaders::Mesh {
		.materialBinding { r_materialManager.get_binding() },
		.materialId {(data.materialBinding == nyan::shaders::INVALID_BINDING) ? r_materialManager.get_material(data.material) : data.materialBinding },
		.indicesAddress { addr + mesh.mesh.indexOffset },
		.positionsAddress { addr + mesh.mesh.vertexOffsets.positionOffset },
		.uvsAddress { addr + mesh.mesh.vertexOffsets.texCoordOffset },
		.normalsAddress { addr + mesh.mesh.vertexOffsets.normalOffset },
		.tangentsAddress { addr + mesh.mesh.vertexOffsets.tangentOffset },
		});

	if (m_buildAccs) {
		vulkan::AccelerationStructureBuilder::BLASInfo blasInfo{
			.vertexBuffer { mesh.mesh.vertexBuffers.positionBuffer },
			.vertexCount { static_cast<uint32_t>(data.positions.size()) },
			.vertexOffset { mesh.mesh.vertexOffsets.positionOffset },
			.vertexFormat { get_format<decltype(data.positions)::value_type>() },
			.vertexStride { format_bytesize<decltype(data.positions)::value_type>() },
			.indexBuffer { mesh.mesh.indexBuffer },
			.indexCount {mesh.mesh.indexCount},
			.indexOffset { mesh.mesh.indexOffset },
			.transformBuffer { VK_NULL_HANDLE },
			.transformOffset { 0 },
			.indexType { mesh.mesh.indexType },
			.geometryFlags {(data.type == nyan::Mesh::RenderType::Opaque) ? VK_GEOMETRY_OPAQUE_BIT_KHR : VkGeometryFlagsKHR{VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV }},
		};
		auto ret = m_builder->queue_item(blasInfo);
		if(ret)
			m_pendingAccBuildIndex.emplace_back( *ret, id);
	}
	m_staticTangentMeshes.emplace(id, mesh);
	m_meshIndex.emplace(data.name, id);
	return id;
}

nyan::MeshID nyan::MeshManager::get_mesh(const std::string& name)
{
	assert(m_meshIndex.find(name) != m_meshIndex.end());
	return m_meshIndex.find(name)->second;
}
void nyan::MeshManager::build()
{
	if (m_buildAccs) {
		auto handles = m_builder->build_pending();
		for (auto [handleId, meshId] : m_pendingAccBuildIndex) {
			assert(handleId < handles.size());
			auto it = m_staticTangentMeshes.find(meshId);
			assert(it != m_staticTangentMeshes.end());
			it->second.accStructure = handles[handleId];
		}
		m_pendingAccBuildIndex.clear();
	}
}
const nyan::shaders::Mesh& nyan::MeshManager::get_shader_mesh(MeshID idx) const
{
	return get(idx);
}
const StaticTangentVulkanMesh& nyan::MeshManager::get_static_tangent_mesh(MeshID idx) const
{
	assert(m_staticTangentMeshes.find(idx) != m_staticTangentMeshes.end());
	return m_staticTangentMeshes.find(idx)->second.mesh;
}

const StaticTangentVulkanMesh& nyan::MeshManager::get_static_tangent_mesh(const std::string& name) const
{
	assert(m_meshIndex.find(name) != m_meshIndex.end());
	assert(m_staticTangentMeshes.find(m_meshIndex.find(name)->second) != m_staticTangentMeshes.end());
	return m_staticTangentMeshes.find(m_meshIndex.find(name)->second)->second.mesh;
}

std::optional<vulkan::AccelerationStructureHandle> nyan::MeshManager::get_acceleration_structure(MeshID idx) 
{
	assert(m_staticTangentMeshes.find(idx) != m_staticTangentMeshes.end());
	return m_staticTangentMeshes.find(idx)->second.accStructure;
}

std::optional<vulkan::AccelerationStructureHandle> nyan::MeshManager::get_acceleration_structure(const std::string& name)
{
	assert(m_meshIndex.find(name) != m_meshIndex.end());
	assert(m_staticTangentMeshes.find(m_meshIndex.find(name)->second) != m_staticTangentMeshes.end());
	return m_staticTangentMeshes.find(m_meshIndex.find(name)->second)->second.accStructure;
}

nyan::InstanceManager::InstanceManager(vulkan::LogicalDevice& device, bool buildAccelerationStructures) :
	DataManager(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 
		| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
		| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR),
	m_builder(buildAccelerationStructures?std::make_unique<AccelerationStructureBuilder>(r_device): nullptr),
	m_buildAccs(buildAccelerationStructures),
	m_tlasBind(r_device.get_bindless_set().reserve_acceleration_structure())
{
}

void nyan::InstanceManager::set_transform(InstanceId id, const Math::Mat<float, 3, 4, false>& transformMatrix)
{
	auto& instance = get(id);

	instance.transform.transformMatrix = transformMatrix;

}
void nyan::InstanceManager::set_acceleration_structure(InstanceId id, uint64_t accelerationStructureReference)
{
	auto& instance = get(id);
	instance.instance.accelerationStructureReference = accelerationStructureReference;
}
void nyan::InstanceManager::set_flags(InstanceId id, VkGeometryInstanceFlagsKHR flags)
{
	auto& instance = get(id);
	instance.instance.flags = flags;
}
void nyan::InstanceManager::set_instance_shader_binding_table_record_offset(InstanceId id, uint32_t instanceShaderBindingTableRecordOffset)
{
	auto& instance = get(id);
	instance.instance.instanceShaderBindingTableRecordOffset = instanceShaderBindingTableRecordOffset;
}
void nyan::InstanceManager::set_mask(InstanceId id, uint32_t mask)
{
	auto& instance = get(id);
	instance.instance.mask = mask;
}
void nyan::InstanceManager::set_instance_custom_index(InstanceId id, uint32_t instanceCustomIndex)
{
	auto& instance = get(id);
	instance.instance.instanceCustomIndex = instanceCustomIndex;
}
void nyan::InstanceManager::set_instance(InstanceId id, const InstanceData& instance)
{
	set(id, instance);
}
const InstanceData& nyan::InstanceManager::get_instance(InstanceId id) const
{
	return get(id);
}
InstanceId nyan::InstanceManager::add_instance(const InstanceData& instanceData)
{
	return add(instanceData);
}



void nyan::InstanceManager::build()
{
	if (m_buildAccs) {
		assert(m_builder);
		m_tlas = m_builder->build_tlas(static_cast<uint32_t>(m_slot->data.size()), m_slot->deviceBuffer->get_address());
		r_device.get_bindless_set().set_acceleration_structure(m_tlasBind, *(*m_tlas));
	}
}

std::optional<vulkan::AccelerationStructureHandle> nyan::InstanceManager::get_tlas()
{
	return m_tlas;
}
std::optional<uint32_t> nyan::InstanceManager::get_tlas_bind()
{
	return m_tlasBind;
}
//StaticMesh* nyan::MeshManager::request_static_mesh(const std::string& name)
//{
//	return nullptr;
//	//if (auto res = m_staticMeshes.find(name); res != m_staticMeshes.end())
//	//	return &res->second;
//
//	//StaticMesh mesh;
//	//vulkan::BufferInfo buffInfo;
//	//buffInfo.size = sizeof(nyan::cubeVertices) + sizeof(nyan::cubeIndices);
//	//buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//	//buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
//	//std::vector<vulkan::InputData> data;
//	//data.push_back({ const_cast<void*>(reinterpret_cast<const void*>(nyan::cubeVertices.data())) ,sizeof(nyan::cubeVertices) });
//	//data.push_back({ const_cast<void*>(reinterpret_cast<const void*>(nyan::cubeIndices.data())) ,  sizeof(nyan::cubeIndices) });
//	//auto vbo = r_device.create_buffer(buffInfo, data);
//
//	//m_usedBuffers.push_back(vbo);
//
//	//mesh.set_indices(vbo, sizeof(nyan::cubeVertices), static_cast<uint32_t>(nyan::cubeIndices.size()));
//	//mesh.set_vertices(vbo, 0, static_cast<uint32_t>(nyan::cubeVertices.size()));
//	//auto res = m_staticMeshes.emplace(name, mesh);
//
//	//return &res.first->second;
//}
//
//StaticMesh* nyan::MeshManager::request_static_mesh(const std::string& name, const std::vector<StaticMesh::Vertex>& vertices, const std::vector<uint16_t>& indices)
//{
//	if (auto res = m_staticMeshes.find(name); res != m_staticMeshes.end())
//		return &res->second;
//
//	StaticMesh mesh;
//	vulkan::BufferInfo buffInfo;
//	buffInfo.size = vertices.size()* sizeof(StaticMesh::Vertex) + indices.size()*sizeof(uint16_t);
//	buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//	buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
//	std::byte* tmp = (std::byte*)malloc(buffInfo.size);
//	std::memcpy(tmp, vertices.data(), vertices.size() * sizeof(StaticMesh::Vertex));
//	std::memcpy(tmp + vertices.size() * sizeof(StaticMesh::Vertex), indices.data(), indices.size() * sizeof(uint16_t));
//	auto vbo = r_device.create_buffer(buffInfo, tmp);
//
//	m_usedBuffers.push_back(vbo);
//
//	mesh.set_indices(vbo, vertices.size() * sizeof(StaticMesh::Vertex), indices.size());
//	mesh.set_vertices(vbo, 0, vertices.size());
//	auto res = m_staticMeshes.emplace(name, mesh);
//
//	return &res.first->second;
//}
//StaticMesh* nyan::MeshManager::request_static_mesh(const std::string& name, const std::vector<StaticMesh::Vertex>& vertices, const std::vector<uint32_t>& indices)
//{
//	if (auto res = m_staticMeshes.find(name); res != m_staticMeshes.end())
//		return &res->second;
//
//	StaticMesh mesh;
//	vulkan::BufferInfo buffInfo;
//	buffInfo.size = vertices.size() * sizeof(StaticMesh::Vertex) + indices.size() * sizeof(uint32_t);
//	buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//	buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
//	std::byte* tmp = (std::byte*)malloc(buffInfo.size);
//	std::memcpy(tmp, vertices.data(), vertices.size() * sizeof(StaticMesh::Vertex));
//	std::memcpy(tmp + vertices.size() * sizeof(StaticMesh::Vertex), indices.data(), indices.size() * sizeof(uint32_t));
//	auto vbo = r_device.create_buffer(buffInfo, tmp);
//
//	m_usedBuffers.push_back(vbo);
//
//	mesh.set_indices(vbo, vertices.size() * sizeof(StaticMesh::Vertex), indices.size());
//	mesh.set_vertices(vbo, 0, vertices.size());
//	auto res = m_staticMeshes.emplace(name, mesh);
//
//	return &res.first->second;
//}

nyan::SceneManager::SceneManager(vulkan::LogicalDevice& device) :
	DataManager(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 1)
{
	add({});
	auto& scene = get(0);
	for (auto& light : scene.pointLights) {
		light = nyan::shaders::PointLight{
			.pos {0.f, 0.f, 0.f},
			.intensity {0.f},
			.color {0.f, 0.f, 0.f},
			.attenuationDistance {0.f},
		};
	}
}

uint32_t nyan::SceneManager::add_point_light(const nyan::shaders::PointLight& light)
{
	auto& scene = get(0);
	if (scene.numPointLights < nyan::shaders::maxNumPointLights) {
		auto id = scene.numPointLights++;
		scene.pointLights[id] = light;
		return id;
	}
	assert(false);
	return invalidShaderId;
}

void nyan::SceneManager::set_point_light(uint32_t id, const nyan::shaders::PointLight& light)
{
	{
		const auto& scene = get(0);
		//assert(id < scene.numPointLights);
		//if (id >= scene.numPointLights)
		//	return;
		if (scene.pointLights[id].attenuationDistance == light.attenuationDistance &&
			scene.pointLights[id].color == light.color &&
			scene.pointLights[id].intensity == light.intensity &&
			scene.pointLights[id].pos == light.pos)
			return;
	}
	auto& scene = get(0);
	if (id >= scene.numPointLights)
		scene.numPointLights = id + 1;
	scene.pointLights[id] = light;
}

void nyan::SceneManager::set_dirlight(const nyan::shaders::DirectionalLight& light)
{
	{
		const auto& scene = get(0);
		if (scene.dirLight.enabled == light.enabled &&
			scene.dirLight.color == light.color &&
			scene.dirLight.intensity == light.intensity &&
			scene.dirLight.dir == light.dir)
			return;
	}
	get(0).dirLight = light;
}

void nyan::SceneManager::set_sky_light(const nyan::shaders::SkyLight& light)
{
	{
		const auto& scene = get(0);
		if (scene.skyLight.color == light.color &&
			scene.skyLight.intensity == light.intensity)
			return;
	}
	get(0).skyLight = light;
}

void nyan::SceneManager::set_view_pos(const Math::vec3& pos)
{
	get(0).viewerPosX = pos[0];
	get(0).viewerPosY = pos[1];
	get(0).viewerPosZ = pos[2];
}

Math::vec3 nyan::SceneManager::get_view_pos() const
{
	return Math::vec3{ get(0).viewerPosX , get(0).viewerPosY ,get(0).viewerPosZ };
}

void nyan::SceneManager::set_camera_up(const Math::vec3& up)
{
	get(0).cameraUpX = up[0];
	get(0).cameraUpY = up[1];
	get(0).cameraUpZ = up[2];
}

void nyan::SceneManager::set_view_matrix(const Math::Mat<float, 4, 4, true>& view)
{
	auto& sceneData = get(0);
	sceneData.view = view;
	sceneData.viewProj = sceneData.proj * sceneData.view;
	sceneData.view.inverse(sceneData.invView);
	sceneData.invViewProj = sceneData.invView * sceneData.invProj;
}

void nyan::SceneManager::set_view_matrix(const Math::Mat<float, 4, 4, true>& view, const Math::Mat<float, 4, 4, true>& viewInverse)
{
	auto& sceneData = get(0);
	sceneData.view = view;
	sceneData.viewProj = sceneData.proj * sceneData.view;
	sceneData.invView = viewInverse;
	sceneData.invViewProj = sceneData.invView * sceneData.invProj;
}

void nyan::SceneManager::set_proj_matrix(const Math::Mat<float, 4, 4, true>& proj)
{
	auto& sceneData = get(0);
	sceneData.proj = proj;
	sceneData.viewProj = sceneData.proj * sceneData.view;
	sceneData.proj.inverse(sceneData.invProj);
	sceneData.invViewProj = sceneData.invView * sceneData.invProj;
}

void nyan::SceneManager::set_proj_matrix(const Math::Mat<float, 4, 4, true>& proj, const Math::Mat<float, 4, 4, true>& projInverse)
{
	auto& sceneData = get(0);
	sceneData.proj = proj;
	sceneData.viewProj = sceneData.proj * sceneData.view;
	sceneData.invProj = projInverse;
	sceneData.invViewProj = sceneData.invView * sceneData.invProj;
}

void nyan::SceneManager::update()
{
}