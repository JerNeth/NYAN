#include "Renderer/RenderManager.h"
#include "Renderer/Light.h"
#include "Utility/Exceptions.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/AccelerationStructure.h"
#include "VulkanWrapper/Shader.h"

nyan::RenderManager::RenderManager(vulkan::LogicalDevice& device, bool useRaytracing, const std::filesystem::path& directory) :
	r_device(device),
	m_rendergraph(r_device),
	m_shaderManager(r_device),
	m_textureManager(r_device, false, directory),
	m_materialManager(r_device, m_textureManager),
	m_meshManager(r_device, m_materialManager,
		r_device.get_physical_device().get_acceleration_structure_features().accelerationStructure? useRaytracing : false),
	m_instanceManager(r_device,
		r_device.get_physical_device().get_acceleration_structure_features().accelerationStructure ? useRaytracing : false),
	m_sceneManager(r_device),
	m_ddgiManager(r_device, m_rendergraph, m_registry),
	m_useRayTracing(r_device.get_physical_device().get_acceleration_structure_features().accelerationStructure &&
		r_device.get_physical_device().get_ray_tracing_pipeline_features().rayTracingPipeline),
	m_primaryCamera(entt::null)
{
}
nyan::Rendergraph& nyan::RenderManager::get_render_graph()
{
	return m_rendergraph;
}
const nyan::Rendergraph& nyan::RenderManager::get_render_graph() const
{
	return m_rendergraph;
}
vulkan::ShaderManager& nyan::RenderManager::get_shader_manager() 
{ 
	return m_shaderManager;
}
const vulkan::ShaderManager& nyan::RenderManager::get_shader_manager() const
{
	return m_shaderManager;
}
nyan::TextureManager& nyan::RenderManager::get_texture_manager()
{
	return m_textureManager;
}
const nyan::TextureManager& nyan::RenderManager::get_texture_manager() const
{
	return m_textureManager;
}
nyan::MaterialManager& nyan::RenderManager::get_material_manager()
{
	return m_materialManager;
}
const nyan::MaterialManager& nyan::RenderManager::get_material_manager() const
{
	return m_materialManager;
}
nyan::MeshManager& nyan::RenderManager::get_mesh_manager()
{
	return m_meshManager;
}
const nyan::MeshManager& nyan::RenderManager::get_mesh_manager() const
{
	return m_meshManager;
}
nyan::InstanceManager& nyan::RenderManager::get_instance_manager()
{
	return m_instanceManager;
}
const nyan::InstanceManager& nyan::RenderManager::get_instance_manager() const
{
	return m_instanceManager;
}
nyan::SceneManager& nyan::RenderManager::get_scene_manager()
{
	return m_sceneManager;
}
const nyan::SceneManager& nyan::RenderManager::get_scene_manager() const
{
	return m_sceneManager;
}

nyan::DDGIManager& nyan::RenderManager::get_ddgi_manager()
{
	return m_ddgiManager;
}

const nyan::DDGIManager& nyan::RenderManager::get_ddgi_manager() const
{
	return m_ddgiManager;
}

entt::registry& nyan::RenderManager::get_registry()
{
	return m_registry;
}

const entt::registry& nyan::RenderManager::get_registry() const
{
	return m_registry;
}

void nyan::RenderManager::add_materials(const std::vector<nyan::MaterialData>& materials)
{
	auto materialCopy = materials;
	for (auto& material : materialCopy) {
		try {
			if (!material.diffuseTex.empty())
				m_textureManager.request_texture(material.diffuseTex);
		}
		catch (Utility::FileNotFoundException e) {
			Utility::log_error().message(e.what());
			material.diffuseTex = "";
		}
		try {
			if (!material.normalTex.empty())
				m_textureManager.request_texture(material.normalTex);
		}
		catch (Utility::FileNotFoundException e) {
			Utility::log_error().message(e.what());
			material.normalTex = "";
		}
		m_materialManager.add_material(material);
	}
}

void nyan::RenderManager::set_primary_camera(entt::entity entity)
{
	assert(m_registry.all_of<PerspectiveCamera>(entity));
	m_primaryCamera = entity;
}

const entt::entity& nyan::RenderManager::get_primary_camera() const
{
	return m_primaryCamera;
}

void nyan::RenderManager::update([[maybe_unused]]std::chrono::nanoseconds dt)
{
	{
		auto view = m_registry.view<const MeshID, const InstanceId>();
		for (const auto& [entity, meshId, instanceId] : view.each()) {
			//auto transformMatrix = Math::Mat<float, 4, 4, false>::affine_transformation_matrix(transform.orientation, transform.position);
			auto transformMatrix = Math::Mat<float, 4, 4, false>::identity();
			for (auto parent = entity; parent != entt::null; parent = m_registry.all_of<Parent>(parent) ? m_registry.get<Parent>(parent).parent : entt::null) {
				if (m_registry.all_of<Transform>(parent)) {
					const auto& parentTransform = m_registry.get<Transform>(parent);
					transformMatrix = Math::Mat<float, 4, 4, false>::affine_transformation_matrix(parentTransform.orientation, parentTransform.position) * transformMatrix;
				}
			}

			m_instanceManager.set_transform(instanceId, Math::Mat<float, 3, 4, false>(transformMatrix));

			auto accHandle = m_meshManager.get_acceleration_structure(meshId);
			if (accHandle) {
				auto instance = (*accHandle)->create_instance();
				m_instanceManager.set_acceleration_structure(instanceId, instance.accelerationStructureReference);
				m_instanceManager.set_flags(instanceId, instance.flags); 
				m_instanceManager.set_mask(instanceId, instance.mask);
				m_instanceManager.set_instance_shader_binding_table_record_offset(instanceId, instance.instanceShaderBindingTableRecordOffset);
				m_instanceManager.set_instance_custom_index(instanceId, meshId);
			}
		}
	}

	{
		auto transformMatrix = Math::Mat<float, 4, 4, false>::identity();
		for (auto parent = m_primaryCamera; parent != entt::null; parent = m_registry.all_of<Parent>(parent) ? m_registry.get<Parent>(parent).parent : entt::null) {
			if (m_registry.all_of<Transform>(parent)) {
				const auto& parentTransform = m_registry.get<Transform>(parent);
				transformMatrix = Math::Mat<float, 4, 4, false>::affine_transformation_matrix(parentTransform.orientation, parentTransform.position) * transformMatrix;
			}
		}

		PerspectiveCamera perspective{};

		if (m_primaryCamera != entt::null && m_registry.all_of<PerspectiveCamera>(m_primaryCamera)) {
			perspective = m_registry.get<PerspectiveCamera>(m_primaryCamera);
		}
		bool useInverseDepth = true;
		if (useInverseDepth) {
			m_sceneManager.set_proj_matrix(
				Math::Mat<float, 4, 4, true>::perspectiveInverseDepthFovXLH(perspective.nearPlane, perspective.fovX, perspective.aspect));
		}
		else {
			m_sceneManager.set_proj_matrix(
				Math::Mat<float, 4, 4, true>::perspectiveFovXLH(perspective.nearPlane, perspective.farPlane, perspective.fovX, perspective.aspect));
		}
		auto cameraPos = static_cast<Math::vec3>(transformMatrix.col(3));
		auto cameraDir = static_cast<Math::vec3>(transformMatrix * static_cast<Math::vec4>(perspective.forward)).normalize();
		auto cameraUp = static_cast<Math::vec3>(transformMatrix * static_cast<Math::vec4>(perspective.up)).normalize();
		auto cameraRight = static_cast<Math::vec3>(transformMatrix * static_cast<Math::vec4>(perspective.right)).normalize();
		m_sceneManager.set_view_matrix(Math::Mat<float, 4, 4, true>::first_person(cameraPos, cameraDir, cameraUp, cameraRight));
		m_sceneManager.set_view_pos(cameraPos);
		m_sceneManager.set_camera_up(cameraUp);
	}

	{
		{
			nyan::shaders::DirectionalLight light;
			light.enabled = false;
			light.color = Math::vec3{ 1, 1, 1 };
			light.intensity = 1;
			light.dir = Math::vec3{ -0.577, -0.577, -0.577 };
			m_sceneManager.set_dirlight(light);
		}
		{
			auto view = m_registry.view<const Directionallight>();
			for (const auto& [entity, dirLight] : view.each()) {
				nyan::shaders::DirectionalLight light;
				light.enabled = dirLight.enabled;
				light.color = dirLight.color;
				light.intensity = dirLight.intensity;
				light.dir = dirLight.direction;
				m_sceneManager.set_dirlight(light);
			}
		}
		{
			auto view = m_registry.view<Pointlight>();
			uint32_t lightCount{ 0 };
			for (const auto& it : view.each()) {
				const auto& entity = it._Myfirst._Val;
				auto& pointLight = it._Get_rest()._Myfirst._Val;
				auto transformMatrix = Math::Mat<float, 4, 4, false>::identity();
				for (auto parent = entity; parent != entt::null; parent = m_registry.all_of<Parent>(parent) ? m_registry.get<Parent>(parent).parent : entt::null) {
					if (m_registry.all_of<Transform>(parent)) {
						const auto& parentTransform = m_registry.get<Transform>(parent);
						transformMatrix = Math::Mat<float, 4, 4, false>::affine_transformation_matrix(parentTransform.orientation, parentTransform.position) * transformMatrix;
					}
				}
				auto pos = transformMatrix *Math::vec4(0, 0, 0, 1);
				nyan::shaders::PointLight light;
				light.pos = pos.xyz();
				light.color = pointLight.color;
				light.intensity = pointLight.intensity;
				light.attenuationDistance = pointLight.attenuation;
				m_sceneManager.set_point_light(lightCount++, light);
			}

			for (; lightCount < nyan::shaders::maxNumPointLights; ++lightCount) {
				m_sceneManager.set_point_light(lightCount, nyan::shaders::PointLight{
						.pos {0.f, 0.f, 0.f},
						.intensity {0.f},
						.color {0.f, 0.f, 0.f},
						.attenuationDistance {0.f},
					});
			}
		}
	}
	//Order doesn't matter
	m_ddgiManager.update();
}

void nyan::RenderManager::begin_frame()
{
	//Skeletal animations have to be before the mesh manager build
	//Has to be before instance manager build and instance manager update
	m_ddgiManager.begin_frame();

	m_meshManager.build();

	bool needsSemaphore = false;
	auto transferCmdHandle = r_device.request_command_buffer(vulkan::CommandBufferType::Transfer);
	vulkan::CommandBuffer& transferCmd = transferCmdHandle;

	needsSemaphore |= m_ddgiManager.upload(transferCmd);
	needsSemaphore |= m_meshManager.upload(transferCmd);
	needsSemaphore |= m_materialManager.upload(transferCmd);
	needsSemaphore |= m_sceneManager.upload(transferCmd);
	needsSemaphore |= m_instanceManager.upload(transferCmd);

	VkSemaphore semaphore{ VK_NULL_HANDLE };
	r_device.submit(transferCmdHandle, needsSemaphore, &semaphore);
	if (needsSemaphore) {
		r_device.add_wait_semaphore(vulkan::CommandBufferType::Compute, semaphore, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
	}

	m_instanceManager.build();
}

void nyan::RenderManager::end_frame()
{
	m_ddgiManager.end_frame();
}
