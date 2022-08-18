#pragma once
#ifndef RDRENDERMANAGER_H
#define RDRENDERMANAGER_H

#include "VkWrapper.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "RenderGraph.h"
#include "MaterialManager.h"
#include "DDGIManager.h"
#include "TextureManager.h"
#include "Camera.h"
#include "entt/entt.hpp"

namespace nyan {

	struct Parent {
		entt::entity parent{ entt::null };
	};
	class RenderManager {
	public:
		RenderManager(vulkan::LogicalDevice& device, bool useRaytracing = false);
		vulkan::ShaderManager& get_shader_manager();
		const vulkan::ShaderManager& get_shader_manager() const ;
		nyan::TextureManager& get_texture_manager();
		const nyan::TextureManager& get_texture_manager() const ;
		nyan::MaterialManager& get_material_manager();
		const nyan::MaterialManager& get_material_manager() const ;
		nyan::MeshManager& get_mesh_manager();
		const nyan::MeshManager& get_mesh_manager() const ;
		nyan::InstanceManager& get_instance_manager();
		const nyan::InstanceManager& get_instance_manager() const;
		nyan::SceneManager& get_scene_manager();
		const nyan::SceneManager& get_scene_manager() const;
		nyan::DDGIManager& get_ddgi_manager();
		const nyan::DDGIManager& get_ddgi_manager() const;
		entt::registry& get_registry();
		const entt::registry& get_registry() const;
		void add_materials(const std::vector<nyan::MaterialData>& materials);
		void set_primary_camera(entt::entity entity);
		const entt::entity& get_primary_camera() const;
		void update();
	private:
		vulkan::LogicalDevice& r_device;
		entt::registry m_registry;
		vulkan::ShaderManager m_shaderManager;
		nyan::TextureManager m_textureManager;
		nyan::MaterialManager m_materialManager;
		nyan::MeshManager m_meshManager;
		nyan::InstanceManager m_instanceManager;
		nyan::SceneManager m_sceneManager;
		nyan::DDGIManager m_ddgiManager;

		bool m_useRayTracing;
		entt::entity m_primaryCamera;
	};
}
#endif !RDRENDERMANAGER_H