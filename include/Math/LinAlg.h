#ifndef LINALG_H
#define LINALG_H
#pragma once
#include "Util.h"
#include "Matrix.h"
#include "Vector.h"
#include "Quaternion.h"
#include "Constants.h"

#include <type_traits>
#include <cstdint>
#include <array>
#include <compare>
#undef min
#undef max

namespace Math
{
	

	//C++20 alternative
	//Should work but somehow doesn't
	//template<typename T>
	//concept Is_Scalar = requires std::is_arithmetic<T>::value;
	//replaces
	//typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type 




	typedef Vec<float, 2> vec2;
	typedef Vec<float, 3> vec3;
	typedef Vec<float, 4> vec4;
	typedef Vec<double, 2> dvec2;
	typedef Vec<double, 3> dvec3;
	typedef Vec<double, 4> dvec4;
	typedef Vec<int32_t, 2> ivec2;
	typedef Vec<int32_t, 3> ivec3;
	typedef Vec<int32_t, 4> ivec4;
	typedef Vec<int64_t, 2> lvec2;
	typedef Vec<int64_t, 3> lvec3;
	typedef Vec<int64_t, 4> lvec4;
	typedef Vec<int16_t, 2> svec2;
	typedef Vec<int16_t, 3> svec3;
	typedef Vec<int16_t, 4> svec4;
	typedef Vec<int8_t, 3> bvec2;
	typedef Vec<int8_t, 3> bvec3;
	typedef Vec<int8_t, 4> bvec4;

	// Unsigned integer vectors do not really make that much sense right now
	typedef Vec<uint32_t, 2> uvec2;
	typedef Vec<uint32_t, 3> uvec3;
	typedef Vec<uint32_t, 4> uvec4;
	typedef Vec<uint64_t, 2> ulvec2;
	typedef Vec<uint64_t, 3> ulvec3;
	typedef Vec<uint64_t, 4> ulvec4;
	typedef Vec<uint16_t, 2> usvec2;
	typedef Vec<uint16_t, 3> usvec3;
	typedef Vec<uint16_t, 4> usvec4;
	typedef Vec<uint8_t, 2> ubvec2;
	typedef Vec<uint8_t, 3> ubvec3;
	typedef Vec<uint8_t, 4> ubvec4;
	


	
	typedef Mat<float, 2, 2> mat22;
	typedef Mat<float, 3, 3> mat33;
	typedef Mat<float, 4, 4> mat44;
	typedef Mat<double, 2, 2> dmat22;
	typedef Mat<double, 3, 3> dmat33;
	typedef Mat<double, 4, 4> dmat44;
	typedef Mat<int32_t, 2, 2> imat22;
	typedef Mat<int32_t, 3, 3> imat33;
	typedef Mat<int32_t, 4, 4> imat44;
	typedef Mat<int64_t, 2, 2> lmat22;
	typedef Mat<int64_t, 3, 3> lmat33;
	typedef Mat<int64_t, 4, 4> lmat44;
	typedef Mat<int8_t, 2, 2> bmat22;
	typedef Mat<int8_t, 3, 3> bmat33;
	typedef Mat<int8_t, 4, 4> bmat44;
	/* Unsigned integer matrices do not really make that much sense right now
	typedef Mat<uint32_t, 2, 2> mat22_ui;
	typedef Mat<uint32_t, 3, 3> mat33_ui;
	typedef Mat<uint32_t, 4, 4> mat44_ui;
	typedef Mat<uint64_t, 2, 2> mat22_ul;
	typedef Mat<uint64_t, 3, 3> mat33_ul;
	typedef Mat<uint64_t, 4, 4> mat44_ul;
	typedef Mat<uint8_t, 2, 2> mat22_ub;
	typedef Mat<uint8_t, 3, 3> mat33_ub;
	typedef Mat<uint8_t, 4, 4> mat44_ub;
	*/

	typedef Quaternion<float>  quat;
	typedef Quaternion<double> quatd;
	
}
#endif // !LINALG_H