#pragma once
#ifndef RDMATERIAL_H
#define RDMATERIAL_H
#include "LinAlg.h"
#include <string>
namespace nyan {
	struct MaterialId {
		uint32_t id;
		MaterialId() : id(0) {}
		MaterialId(uint32_t id) : id(id) {}
		operator uint32_t() const {
			return id;
		}
	};
	using MaterialID = uint32_t;
	struct MaterialData {
		std::string name;
		std::string diffuseTex;
		std::string normalTex;
		Math::vec3 ambientColor;
		Math::vec3 diffuseColor;
		float ambientFactor;
		float diffuseFactor;
		float shininessFactor;
		float transparendyFactor;
	};
	enum class AlphaMode : uint32_t {
		Opaque,
		AlphaTest,
		AlphaBlend
	};
	struct PBRMaterialData {
		std::string name;
		std::string albedoTex;
		std::string emissiveTex;
		std::string roughnessMetalnessTex;
		std::string normalTex;
		Math::vec4 albedoFactor{ 1.f, 1.f, 1.f, 1.f };
		Math::vec3 emissiveFactor{ 0.f, 0.f, 0.f };
		AlphaMode alphaMode{ AlphaMode::Opaque };
		float alphaCutoff{ 0.5f };
		bool doubleSided{ false };
		float metallicFactor{ 1.f };
		float roughnessFactor{ 1.f };
	};
}

#endif !RDMATERIAL_H