#pragma once
#include "LinAlg.h"

namespace nyan {
	struct Transform {
		Math::vec3 position;
		Math::vec3 scale;
		Math::vec3 orientation;
	};
}