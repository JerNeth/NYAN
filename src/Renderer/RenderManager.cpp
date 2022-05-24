#include "Renderer/RenderManager.h"

nyan::RenderManager::RenderManager(vulkan::LogicalDevice& device, bool useRaytracing) :
	r_device(device),
	m_shaderManager(r_device),
	m_textureManager(r_device),
	m_materialManager(r_device, m_textureManager),
	m_meshManager(r_device, m_materialManager, useRaytracing),
	m_instanceManager(r_device, useRaytracing),
	m_sceneManager(r_device),
	m_useRayTracing(useRaytracing),
	m_primaryCamera(entt::null)
{
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

entt::registry& nyan::RenderManager::get_registry()
{
	return m_registry;
}

const entt::registry& nyan::RenderManager::get_registry() const
{
	return m_registry;
}

void nyan::RenderManager::set_primary_camera(entt::entity entity)
{
	assert(m_registry.all_of<PerspectiveCamera>(entity));
	m_primaryCamera = entity;
}

void nyan::RenderManager::update()
{
	//Has to be before instance manager build and instance manager update
	m_meshManager.build();
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
			m_instanceManager.set_instance_shader_binding_table_record_offset(instanceId, instance.instanceShaderBindingTableRecordOffset);
			m_instanceManager.set_mask(instanceId, instance.mask);
			m_instanceManager.set_instance_custom_index(instanceId, meshId);
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
		
		PerspectiveCamera perspective {};
		
		if (m_primaryCamera != entt::null && m_registry.all_of<PerspectiveCamera>(m_primaryCamera)) {
			perspective = m_registry.get<PerspectiveCamera>(m_primaryCamera);
		}
		m_sceneManager.set_proj_matrix(
			Math::Mat<float, 4, 4, true>::perspectiveY(perspective.nearPlane, perspective.farPlane, perspective.fovX, perspective.aspect));
		Math::vec3 cameraPos = transformMatrix.col(3);
		Math::vec3 cameraDir = transformMatrix * perspective.forward;
		m_sceneManager.set_view_matrix(Math::Mat<float, 4, 4, true>::first_person(cameraPos, cameraDir, perspective.up));

	}
	m_meshManager.upload();
	m_materialManager.upload();
	m_sceneManager.upload();
	m_instanceManager.upload();
	m_instanceManager.build();
}
