#pragma once
#include "LinAlg.h"

namespace nyan {
	struct Transform {
		Math::vec3 position {0.f};
		Math::vec3 scale {1.f};
		Math::vec3 orientation{ 0.f };
		//Math::quat orientation {};
	};
}