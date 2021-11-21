#ifndef LINALGUTIL_H
#define LINALGUTIL_H
#pragma once
#include <type_traits>

namespace Math {
	template<typename T>
	concept ScalarT = std::is_arithmetic<T>::value;

	template<ScalarT S, ScalarT B>
	inline constexpr const S& min(const S& a, const B& b) noexcept {
		//Assuming IEEE-754
		//If either is NaN, > returns FALSE => min(a,b) gives you the opposite result compared to max(a,b)
		if (!(a > b))
			return a;
		return b;
	}
	template<ScalarT S, ScalarT B>
	inline constexpr const S& max(const S& a, const B& b) noexcept {
		//Assuming IEEE-754
		//If either is NaN, > returns FALSE
		if (a > b)
			return a;
		return b;
	}
	/*
	If val is NaN => returns min
	*/
	template<ScalarT S>
	inline constexpr const S& clamp(const S& val, const S& min, const S& max) noexcept {
		return Math::max(Math::min(val, max), min);
	}
	template<ScalarT S>
		inline constexpr bool close(const S& a, const S& b, const S& eps = S(1e-5)) noexcept {
		return ((a - eps) <= b) && ((a + eps) >= b);
	}
	template<ScalarT T>
	inline constexpr const T square(const T& a) noexcept {
		return a * a;
	}
	template<int height>
	inline constexpr int at(int x, int y) {
		return x*height + y;
	}
	template<size_t height>
	inline constexpr size_t at(size_t x, size_t y) {
		return x * height + y;
	}
	//This function does not handle limits well
	//This function is also stable
	template<typename T, ScalarT U>
	inline constexpr T lerp(T a, T b, U t) {
		assert(t >= 0.0f && t <= 1.0f);
		//return (1 - t) * a + t * b;
		return a + t * (b - a);
	}
}
#endif