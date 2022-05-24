#pragma once
#ifndef RDMATERIAL_H
#define RDMATERIAL_H
#include "LinAlg.h"
#include <string>
namespace nyan {
	using MaterialID = uint32_t;
	struct MaterialData {
		std::string name;
		std::string diffuseTex;
		std::string normalTex;
		Math::vec3 ambientColor;
		Math::vec3 diffuseColor;
		float ambientFactor;
		float diffuseFactor;
		float shininessFacor;
	};
}

#endif !RDMATERIAL_H