#pragma once
#ifndef BITS_H
#define BITS_H
namespace Utility {
	inline constexpr uint32_t bit_width(uint64_t value) {
		if (value >= 1) {
			uint64_t mask = 1ull << 63ull;
			uint32_t result = 64;
			while (mask != 0) {
				if (value & mask)
					return result;
				mask >>= 1;
				--result;
			}
		}
		return 0;
	}
	inline constexpr uint32_t bit_pos(uint64_t value) {
		return bit_width(value) - 1;
	}

	inline uint32_t fast_log2(uint32_t num) {
		double ff = static_cast<double>(num | 1);
		uint32_t tmp;
		std::memcpy(&tmp, reinterpret_cast<const char*>(&ff) + sizeof(uint32_t), sizeof(uint32_t));
		return (tmp >> 20) - 1023;
	}
	template <typename T, typename C>
	inline void for_each_bit(const C& bitset, const T& func) {
		for (uint32_t i = 0; i < bitset.size(); i++) {
			if (bitset.test(i))
				func(i);
		}
	}

	template <typename T, typename C>
	inline void for_each_bitrange(const C& bitset, const T& func) {
		if (bitset.all()) {
			func(0, bitset.size());
			return;
		}
		uint32_t first = 0;
		bool ones = false;
		for (size_t i = 0; i < bitset.size(); i++) {
			if (bitset.test(i)) {
				if (!ones) {
					first = i;
					ones = true;
				}
			}
			else {
				if (ones) {
					ones = false;
					func(first, i - first);
				}
			}
		}
	}

}
#endif