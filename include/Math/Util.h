#ifndef LINALGUTIL_H
#define LINALGUTIL_H
#pragma once
#include <type_traits>

namespace Math {
	template<typename T>
	concept ScalarT = std::is_arithmetic<T>::value;

	template<ScalarT S, ScalarT B>
	inline constexpr auto min(const S& a, const B& b) noexcept -> decltype(a + b)  {
		//Assuming IEEE-754
		//If either is NaN, > returns FALSE => min(a,b) gives you the opposite result compared to max(a,b)
		if (!(a > b))
			return a;
		return b;
	}
	template<ScalarT S, ScalarT B>
	inline constexpr auto max(const S& a, const B& b) noexcept-> decltype(a + b) {
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
	inline constexpr S clamp(const S& val, const S& min, const S& max) noexcept {
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
	template<int size, bool column_major = true>
	inline constexpr int at(int x, int y) {
		if constexpr (column_major) {
			return x * size + y;
		}
		else {
			return y * size + x;
		}
	}
	template<size_t size, bool column_major = true>
	inline constexpr size_t at(size_t x, size_t y) {
		if constexpr (column_major) {
			return x * size + y;
		}
		else {
			return y * size + x;
		}
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