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
		Math::vec3 direction;
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
		bool enabled;
		bool shadows;
		Math::vec3 color;
		float intensity;
		Math::vec3 direction;
	};
	struct SkyLight {
		Math::vec3 color;
		float intensity;
	};
}

#endif //!RDMATERIAL_H