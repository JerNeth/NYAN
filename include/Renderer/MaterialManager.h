#pragma once
#ifndef RDMATERIALMANAGER_H
#define RDMATERIALMANAGER_H
#include "LogicalDevice.h"
#include "DataManager.h"
#include "ShaderInterface.h"
#include "Material.h"
namespace nyan {
	class TextureManager;

	//enum class F0E : size_t {
	//	Titanium,
	//	Chromium,
	//	Iron,
	//	Nickel,
	//	Platinum,
	//	Copper,
	//	Palladium,
	//	Zinc,
	//	Gold,
	//	Aluminium,
	//	Silver,
	//	Water,
	//	Plastic,
	//	Glass,
	//};
	//std::array F0{Math::vec3(0.542f, 0.497f, 0.449f),
	//			Math::vec3(0.549f, 0.556f, 0.554f),
	//			Math::vec3(0.562f, 0.565f, 0.578f),
	//			Math::vec3(0.660f, 0.609f, 0.526f),
	//			Math::vec3(0.673f, 0.637f, 0.585f),
	//			Math::vec3(0.955f, 0.638f, 0.538f),
	//			Math::vec3(0.733f, 0.697f, 0.652f),
	//			Math::vec3(0.664f, 0.824f, 0.850f),
	//			Math::vec3(1.022f, 0.782f, 0.344f),
	//			Math::vec3(0.913f, 0.922f, 0.924f),
	//			Math::vec3(0.972f, 0.960f, 0.915f),
	//			Math::vec3(0.02, 0.020f, 0.020f),
	//			Math::vec3(0.045f, 0.045f, 0.045f),
	//			Math::vec3(0.040f, 0.040f, 0.040f), };
	class MaterialManager : public DataManager<nyan::shaders::Material> {
	public:
		MaterialManager(vulkan::LogicalDevice& device, nyan::TextureManager& textureManager);
		void set_material(MaterialId idx, const MaterialData& data);
		MaterialId add_material(const MaterialData& data);
		MaterialId add_material(const PBRMaterialData& data);
		MaterialId get_material(const std::string& name);
		nyan::shaders::Material& get_material(MaterialId idx);
		const nyan::shaders::Material& get_material(MaterialId idx) const;
	private:

		nyan::TextureManager& r_textureManager;
		std::unordered_map<std::string, MaterialId> m_materialIndex;
	};
}
#endif //!RDMATERIALMANAGER_H