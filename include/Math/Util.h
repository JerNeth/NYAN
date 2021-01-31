#ifndef LINALGUTIL_H
#define LINALGUTIL_H
#pragma once
#include <type_traits>

namespace Math {
	template<typename Scalar,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
		inline constexpr const Scalar& min(const Scalar& a, const Scalar& b) noexcept {
		if (a <= b)
			return a;
		return b;
	}
	template<typename Scalar,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
		inline constexpr const Scalar& max(const Scalar& a, const Scalar& b) noexcept {
		if (a > b)
			return a;
		return b;
	}
	template<typename Scalar,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
		inline constexpr bool close(const Scalar& a, const Scalar& b, const Scalar& eps = Scalar(1e-5)) noexcept {
		return ((a - eps) <= b) && ((a + eps) >= b);
	}
	template<typename T>
	inline constexpr const T square(const T& a) noexcept {
		return a * a;
	}
	template<int width>
	inline constexpr int at(int y, int x) {
		return x + y * width;
	}
	template<size_t width>
	inline constexpr size_t at(size_t y, size_t x) {
		return x + y * width;
	}
	//This function does not handle limits well
	//This function is also stable
	template<typename T, typename U>
	inline constexpr T lerp(T a, T b, U t) {
		assert(t >= 0.0f && t <= 1.0f);
		return (1 - t) * a + t * b;
	}
}
#endif