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
	typedef Vec<double, 2> vec2_d;
	typedef Vec<double, 3> vec3_d;
	typedef Vec<double, 4> vec4_d;
	typedef Vec<int32_t, 2> vec2_i;
	typedef Vec<int32_t, 3> vec3_i;
	typedef Vec<int32_t, 4> vec4_i;
	typedef Vec<int64_t, 2> vec2_l;
	typedef Vec<int64_t, 3> vec3_l;
	typedef Vec<int64_t, 4> vec4_l;
	typedef Vec<int8_t, 3> vec2_b;
	typedef Vec<int8_t, 3> vec3_b;
	typedef Vec<int8_t, 4> vec4_b;

	/* Unsigned integer vectors do not really make that much sense right now
	typedef Vec<uint32_t, 2> vec2_ui;
	typedef Vec<uint32_t, 3> vec3_ui;
	typedef Vec<uint32_t, 4> vec4_ui;
	typedef Vec<uint64_t, 2> vec2_ul;
	typedef Vec<uint64_t, 3> vec3_ul;
	typedef Vec<uint64_t, 4> vec4_ul;
	typedef Vec<uint8_t, 2> vec2_ub;
	typedef Vec<uint8_t, 3> vec3_ub;
	typedef Vec<uint8_t, 4> vec4_ub;
	*/


	
	typedef Mat<float, 2, 2> mat22;
	typedef Mat<float, 3, 3> mat33;
	typedef Mat<float, 4, 4> mat44;
	typedef Mat<double, 2, 2> mat22_d;
	typedef Mat<double, 3, 3> mat33_d;
	typedef Mat<double, 4, 4> mat44_d;
	typedef Mat<int32_t, 2, 2> mat22_i;
	typedef Mat<int32_t, 3, 3> mat33_i;
	typedef Mat<int32_t, 4, 4> mat44_i;
	typedef Mat<int64_t, 2, 2> mat22_l;
	typedef Mat<int64_t, 3, 3> mat33_l;
	typedef Mat<int64_t, 4, 4> mat44_l;
	typedef Mat<int8_t, 2, 2> mat22_b;
	typedef Mat<int8_t, 3, 3> mat33_b;
	typedef Mat<int8_t, 4, 4> mat44_b;
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