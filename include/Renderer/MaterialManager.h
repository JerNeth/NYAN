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
	struct MaterialBinding {
		union {
			struct {
				uint32_t binding;
				uint32_t id;
			};
			uint64_t data;
		};
		template<std::size_t Index>
		auto get() const
		{
			static_assert(Index < 2,
				"Index out of bounds for Custom::MaterialBinding");
			if constexpr (Index == 0) return id;
			if constexpr (Index == 1) return binding;
		}
	};
	class MaterialManager : public DataManager<InternalMaterialData, MaterialBinding, 1024> {
	public:
		MaterialManager(vulkan::LogicalDevice& device, nyan::TextureManager& textureManager);
		void set_material(const nyan::MaterialBinding& idx, const MaterialData& data);
		MaterialBinding add_material(const MaterialData& data);
		MaterialBinding get_material(const std::string& name);
	private:

		nyan::TextureManager& r_textureManager;
		std::unordered_map<std::string, MaterialBinding> m_materialIndex;
	};
}
#endif !RDMATERIALMANAGER_H