#ifndef LINALGUTIL_H
#define LINALGUTIL_H
#pragma once
namespace bla {
	template<typename Scalar,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
		inline constexpr const Scalar& min(const Scalar& a, const Scalar& b) noexcept {
		if ((a <=> b) < 0)
			return a;
		return b;
	}
	template<typename Scalar,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
		inline constexpr const Scalar& max(const Scalar& a, const Scalar& b) noexcept {
		//Three way comparison, why? because it's new
		//No really, no reason to do this
		if ((a <=> b) > 0)
			return a;
		return b;
	}
	template<typename Scalar,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
		inline constexpr const bool close(const Scalar& a, const Scalar& b, const Scalar& eps = Scalar(1e-5)) noexcept {
		return ((a - eps) <= b) && ((a + eps) >= b);
	}
	template<typename T>
	inline constexpr const T square(const T& a) noexcept {
		return a * a;
	}
	template<int width>
	inline constexpr const int at(int y, int x) {
		return x + y * width;
	}
	template<size_t width>
	inline constexpr const size_t at(size_t y, size_t x) {
		return x + y * width;
	}
}
#endif