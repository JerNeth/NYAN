#pragma once
#ifndef RDRENDERMANAGER_H
#define RDRENDERMANAGER_H

#include "VkWrapper.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "RenderGraph.h"
#include "MaterialManager.h"
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
		entt::registry& get_registry();
		const entt::registry& get_registry() const;
		void set_primary_camera(entt::entity entity);
		void update();
	private:
		vulkan::LogicalDevice& r_device;
		entt::registry m_registry;
		vulkan::ShaderManager m_shaderManager;
		//TODO possibly chunk textures and queue upload
		nyan::TextureManager m_textureManager;
		nyan::MaterialManager m_materialManager;
		nyan::MeshManager m_meshManager;
		nyan::InstanceManager m_instanceManager;
		nyan::SceneManager m_sceneManager;

		bool m_useRayTracing;
		entt::entity m_primaryCamera;
	};
}
#endif !RDRENDERMANAGER_H