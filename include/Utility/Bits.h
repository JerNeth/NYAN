#pragma once
#ifndef BITS_H
#define BITS_H
#include <cstdint>
#include <bit>
#include <cassert>
namespace Utility {
	constexpr inline uint32_t bit_width(uint64_t value) {
		return static_cast<uint32_t>(std::bit_width(value));
	}
	constexpr inline uint32_t bit_pos(uint64_t value) {
		return value? bit_width(value) - 1 : 0;
	}

	inline constexpr uint32_t fast_log2(uint32_t num) {
		auto integer = std::bit_cast<uint64_t>(static_cast<double>(num | 1));
		//Use exponent of double
		return (integer >> (20+32)) - 1023;
	}
	template <typename T, typename C>
	inline void for_each_bit(const C& bitset, const T& func) {
		for (size_t i = 0; i < bitset.size(); i++) {
			if (bitset.test(i))
				func(i);
		}
	}

	template <typename T, typename C>
	inline void for_each_bitrange(const C& bitset, const T& func) {
		if (bitset.all()) {
			func(0, static_cast<uint32_t>(bitset.size()));
			return;
		}
		uint32_t first = 0;
		bool ones = false;
		for (uint32_t i = 0; i < bitset.size(); i++) {
			if (bitset.test(i)) {
				if (!ones) {
					first = i;
					ones = true;
				}
			}
			else {
				if (ones) {
					ones = false;
					func(first, static_cast<uint32_t>(i - first));
				}
			}
		}
	}
	template<typename T>
	constexpr inline T align_up(T size, T alignment) {
		assert(std::has_single_bit(alignment));
		alignment -= 1;
		return (size + alignment) & ~alignment;
	}

}
#endif