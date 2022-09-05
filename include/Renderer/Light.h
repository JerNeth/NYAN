#pragma once
#ifndef RDLIGHT_H
#define RDLIGHT_H
#include "LinAlg.h"
#include <string>
namespace nyan {
	struct LightParameters {
		enum class Type {
			Directional,
			Point,
			Spot
		} type;
		std::string name;
		Math::vec3 color;
		float intensity;
		Math::vec3 translate;
		Math::vec3 rotate;
		Math::vec3 scale;
	};
	struct Spotlight {
		bool shadows;
		Math::vec3 color;
		float intensity;
		Math::vec3 direction;
		float cone;
		float attenuation;
	};
	struct Pointlight {
		bool shadows;
		Math::vec3 color;
		float intensity;
		float attenuation;
	};
	struct Directionallight {
		bool shadows;
		Math::vec3 color;
		float intensity;
		Math::vec3 direction;
	};
}

#endif !RDMATERIAL_H