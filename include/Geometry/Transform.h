#pragma once
#include "LinAlg.h"

class Transform {
private:
	bla::vec3 m_position;
	bla::vec3 m_scale;
	bla::quat m_orientation;
};