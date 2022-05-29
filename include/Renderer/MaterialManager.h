#pragma once
#ifndef RDMATERIALMANAGER_H
#define RDMATERIALMANAGER_H
#include "LogicalDevice.h"
#include "DataManager.h"
#include "ShaderInterface.h"
#include "Material.h"
namespace nyan {
	class TextureManager;

	struct MaterialId {
		uint32_t id;
		MaterialId() : id(0) {}
		MaterialId(uint32_t id) : id(id){}
		operator uint32_t() const {
			return id;
		}
	};
	class MaterialManager : public DataManager<nyan::shaders::Material> {
	public:
		MaterialManager(vulkan::LogicalDevice& device, nyan::TextureManager& textureManager);
		void set_material(MaterialId idx, const MaterialData& data);
		MaterialId add_material(const MaterialData& data);
		MaterialId get_material(const std::string& name);
		nyan::shaders::Material& get_material(MaterialId idx);
		const nyan::shaders::Material& get_material(MaterialId idx) const;
	private:

		nyan::TextureManager& r_textureManager;
		std::unordered_map<std::string, MaterialId> m_materialIndex;
	};
}
#endif !RDMATERIALMANAGER_H