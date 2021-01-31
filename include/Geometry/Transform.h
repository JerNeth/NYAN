#pragma once
#include "LinAlg.h"

class Transform {
private:
	Math::vec3 m_position;
	Math::vec3 m_scale;
	Math::quat m_orientation;
};