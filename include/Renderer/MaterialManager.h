#pragma once
#ifndef RDMATERIALMANAGER_H
#define RDMATERIALMANAGER_H
#include "LogicalDevice.h"
#include "DataManager.h"
#include "Material.h"
namespace nyan {
	class TextureManager;

	struct InternalMaterialData {
		uint32_t diffuseTexId;
		uint32_t normalTexId;
		uint32_t pad;
		float shininessFactor;
		Math::vec3 ambientColor;
		float ambientFactor;
		Math::vec3 diffuseColor;
		float diffuseFactor;
	};
	struct MaterialId {
		uint32_t id;
		MaterialId() {}
		MaterialId(uint32_t id) : id(id){}
		operator uint32_t() const {
			return id;
		}
	};
	class MaterialManager : public DataManager<InternalMaterialData> {
	public:
		MaterialManager(vulkan::LogicalDevice& device, nyan::TextureManager& textureManager);
		void set_material(MaterialId idx, const MaterialData& data);
		MaterialId add_material(const MaterialData& data);
		MaterialId get_material(const std::string& name);
	private:

		nyan::TextureManager& r_textureManager;
		std::unordered_map<std::string, MaterialId> m_materialIndex;
	};
}
#endif !RDMATERIALMANAGER_H