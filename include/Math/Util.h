#ifndef LINALGUTIL_H
#define LINALGUTIL_H
#pragma once
#include <type_traits>
#include <cmath>
#include <algorithm>

namespace Math {

	struct half;
	template<typename T>
	concept Unsigned = std::is_unsigned<T>::value;
	template<typename T>
	concept Signed = std::is_signed<T>::value;
	template<Unsigned T>
	struct unorm;
	template<Signed T>
	struct snorm;

	template<typename T>
	concept ScalarT = std::is_arithmetic<T>::value || std::is_same_v<T, Math::half> || std::is_same_v<T, Math::unorm<uint8_t>> || std::is_same_v<T, Math::unorm<uint16_t>> || std::is_same_v<T, Math::unorm<uint32_t>> || std::is_same_v<T, Math::unorm<uint64_t>>
		|| std::is_same_v<T, Math::snorm<int8_t>> || std::is_same_v<T, Math::snorm<int16_t>> || std::is_same_v<T, Math::snorm<int32_t>> || std::is_same_v<T, Math::snorm<int64_t>>;

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

	struct half {
	private:
		static constexpr uint32_t floatMantissaBits = 23u;
		static constexpr uint32_t floatExpBits = 8u;
		static constexpr uint32_t floatBits = floatMantissaBits + floatExpBits + 1;
		static constexpr uint32_t floatSignBits = 31u;
		static constexpr uint32_t floatSignBit = 1u << floatSignBits;
		static constexpr uint32_t floatExpMax = (1u << floatExpBits) - 1u;
		static constexpr uint32_t floatExpBias = floatExpMax >> 1;
		static constexpr uint32_t floatInf = floatExpMax << floatMantissaBits;
		static constexpr uint32_t halfMantissaBits = 10u;
		static constexpr uint32_t halfExpBits = 5u;
		static constexpr uint32_t halfBits = halfMantissaBits + halfExpBits + 1;
		static constexpr uint32_t halfSignBits = 15u;
		static constexpr uint32_t halfSignBit = 1 << halfSignBits;
		static constexpr uint32_t halfExpMax = (1 << halfExpBits) - 1;
		static constexpr uint32_t halfExpBias = halfExpMax >> 1;
		static constexpr uint32_t halfInf = halfExpMax << halfMantissaBits;
		static constexpr uint32_t halfQnan = (halfInf | (halfInf >> 1));
		static constexpr uint32_t mantissaDiff = (floatMantissaBits - halfMantissaBits);
		static constexpr uint32_t bitDiff = (floatBits - halfBits);
	public:
		constexpr half() : data(0) {
		}
		constexpr half(float f) {
			constexpr auto bias = std::bit_cast<float>(halfExpBias << floatMantissaBits);
			f *= bias;
			auto bits = std::bit_cast<uint32_t>(f);
			auto sign = bits & floatSignBit;
			bits ^= sign;
			auto isNan = static_cast<uint32_t>(-static_cast<int32_t>(floatInf < bits));
			bits >>= mantissaDiff;
			bits ^= static_cast<uint32_t>(-static_cast<int32_t>(halfInf < bits)) & (halfInf ^ bits);
			bits ^= isNan & (halfQnan ^ bits);
			bits |= sign >> bitDiff;
			data = static_cast<uint16_t>(bits);
		}
		operator float() const {
			constexpr auto bias = std::bit_cast<float>((2u * floatExpBias - halfExpBias) << floatMantissaBits);
			uint32_t bits = data;
			auto sign = bits & halfSignBit;
			bits ^= sign;
			auto isNorm = bits < halfInf;
			bits = (sign << bitDiff) | (bits << mantissaDiff);
			auto val = std::bit_cast<float>(bits) * bias;
			bits = std::bit_cast<uint32_t>(val);
			bits |= -!isNorm & floatInf;
			return std::bit_cast<float>(bits);
		}
		uint16_t data;
	};
	//template<Unsigned T, size_t size>
	//constexpr Vec<T, size> unormVec(const Vec<float, size>& vec) {
	//	//c = convertFloatToUInt(f * (2^b-1), b)
	//	Vec<T, size> ret;
	//	float max = static_cast<float>(T(-1));
	//	for (size_t i = 0; i < size; i++) {
	//		auto temp = clamp(vec[i], 0.f, 1.f);
	//		ret[i] = static_cast<T>(temp * max);
	//	}
	//	return ret;
	//}
	//template<Signed T, size_t size>
	//constexpr Vec<T, size> snormVec(const Vec<float, size>& vec) {
	//	//See https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html#fundamentals-fixedfpconv
	//	//f = max(c/2^(b-1)-1, -1.0) for int => float
	//	//c = convertFloatToInt(f * (2^(b-1)-1), b)
	//	Vec<T, size> ret;
	//	float max = static_cast<float>((1ull << (sizeof(T) * 8ull - 1ull)) - 1ull);
	//	for (size_t i = 0; i < size; i++) {
	//		//NaN not correctly handled
	//		auto temp = clamp(vec[i], -1.f, 1.f);
	//		ret[i] = static_cast<T>(temp * max);
	//	}
	//	return ret;
	//}
	template<Unsigned T>
	struct unorm {
	private:
	public:
		constexpr unorm() : data(0) {
		}
		constexpr unorm(float f) {
			static constexpr auto max = static_cast<float>(std::numeric_limits<T>::max());
			data = std::clamp(static_cast<T>(std::round(f * max)), std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
		}
		operator float() const {
			static constexpr auto max = 1.f / static_cast<float>(std::numeric_limits<T>::max());
			return static_cast<float>(data) * max;
		}
		T data;
	};
	template<Signed T>
	struct snorm {
	private:
	public:
		//https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html#fundamentals-fixedfpconv
		constexpr snorm() : data(0) {
		}
		constexpr snorm(float f) {
			static constexpr auto max = static_cast<float>(static_cast<T>(-1) >> 1);
			data = static_cast<T>(std::round(Math::clamp(f , -1.0f, 1.0f) * max));
		}
		operator float() const {
			static constexpr auto max = 1.f / static_cast<float>(static_cast<T>(-1) >> 1);
			return std::max(static_cast<float>(data) * max, -1.0f);
		}
		T data;
	};

}
#endif