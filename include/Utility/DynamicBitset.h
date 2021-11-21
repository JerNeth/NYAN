#ifndef UTVECTOR_H
#define UTVECTOR_H
#pragma once
#include <stdlib.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <bit>
#include <assert.h>
namespace Utility {
	
	class DynamicBitset {
		static constexpr size_t occupancy_size = (sizeof(size_t) * 8u);
		static constexpr size_t full_mask = ~static_cast<size_t>(0u);
		static constexpr size_t none_mask = static_cast<size_t>(0u);
		static constexpr size_t single_mask = static_cast<size_t>(1u);
	public:
		DynamicBitset() = default;
		~DynamicBitset() {
			if (m_occupancy)
				free(m_occupancy);
		}
		DynamicBitset(DynamicBitset& other) noexcept :
			m_size(other.m_size) 
		{
			m_occupancy = static_cast<size_t*>(malloc(m_size));
			memcpy(m_occupancy, other.m_occupancy, m_size);
		}
		DynamicBitset(DynamicBitset&& other) noexcept : 
			m_occupancy(other.m_occupancy),
			m_size(other.m_size) 
		{
			other.m_occupancy = nullptr;
			other.m_size = 0;
		}
		DynamicBitset& operator=(DynamicBitset& other) noexcept
		{
			if (this != &other) {
				m_size = other.m_size;
				m_occupancy = static_cast<size_t*>(malloc(m_size));
				memcpy(m_occupancy, other.m_occupancy, m_size);
			}
			return *this;
		}
		DynamicBitset& operator=(DynamicBitset&& other) noexcept
		{
			if (this != &other) {
				m_occupancy = other.m_occupancy;
				m_size = other.m_size;
				other.m_occupancy = nullptr;
				other.m_size = 0;
			}
			return *this;
		}
		void reserve(size_t new_capacity) {
			if (new_capacity > capacity()) {
				auto new_size = new_capacity / occupancy_size;
				new_size += (new_size * occupancy_size != new_capacity);
				auto data = static_cast<size_t*>(realloc(m_occupancy, new_size * sizeof(size_t)));
				if (!data)
					throw std::runtime_error("Could not realloc");
				m_occupancy = data;
				memset(m_occupancy + m_size, 0, (new_size - m_size)*sizeof(size_t));
				m_size = new_size;
			}
		}
		size_t find_empty() const noexcept
		{
			assert(m_occupancy != nullptr);
			size_t bucket = 0;
			for (; (m_occupancy[bucket] & full_mask) == full_mask; bucket++) {
				assert(bucket < m_size);
			}
			size_t idx = bucket * sizeof(size_t) * 8;
			for (; test(idx); idx++) {

			}
			return idx;
		}
		size_t count() const noexcept {
			size_t ret = 0;
			assert(m_occupancy);
			for (size_t bucket = 0; bucket < m_size; bucket++) {
				ret += std::popcount(m_occupancy[bucket]);
			}
			return ret;
		}
		void clear() noexcept {
			for (size_t bucket = 0; bucket < m_size; bucket++) {
				m_occupancy[bucket] = size_t(0);
			}
		}
		bool test(size_t idx) const noexcept  {
			assert(m_occupancy);
			assert(idx < capacity());
			return (m_occupancy[idx / occupancy_size] >> (idx % occupancy_size)) & single_mask;
		}
		void set(size_t idx) noexcept {
			assert(m_occupancy);
			assert(idx < capacity());
			m_occupancy[idx / occupancy_size] |= single_mask << (idx % occupancy_size);
		}
		void clear(size_t idx) noexcept {
			assert(m_occupancy);
			assert(idx < capacity());
			m_occupancy[idx / occupancy_size] &= ~(single_mask << (idx % occupancy_size));
		}
		void toggle(size_t idx) noexcept {
			assert(m_occupancy);
			assert(idx < capacity());
			m_occupancy[idx / occupancy_size] ^= single_mask << (idx % occupancy_size);
		}
		constexpr size_t capacity() const noexcept  {
			return m_size * sizeof(size_t)*8;
		}
	private:

		size_t* m_occupancy = nullptr;
		size_t m_size = 0;
		
	};
	template<size_t bitSize, typename T = size_t> // typename for indices e.g. enums
	class bitset {
		using bitType = std::conditional_t< bitSize <= 8, uint8_t, std::conditional_t < bitSize <= 16, uint16_t, std::conditional_t < bitSize <= 32, uint32_t,  uint64_t>>> ;
		static constexpr size_t bitsPerWord = (sizeof(bitType) * 8);
		static constexpr size_t typeSize  = bitSize / bitsPerWord + (bitSize % bitsPerWord != 0);
		//static_assert(std::is_convertible<T, size_t>::value);
	public:
		bitset() {
			m_data[0] = 0;
		}
		bitset(bitType t) {
			m_data[0] = t;
		}

		bitset(const bitType& t) {
			m_data[0] = t;
		}
		bool test(T _idx) const {
			const size_t idx = static_cast<size_t>(_idx);
			assert(idx < bitSize);
			auto& word = m_data[idx / bitsPerWord];
			const auto bit = 1u << idx % bitsPerWord;
			return static_cast<decltype(bit)>(word) & bit;
		}
		template<class Head, class... Tail>
		using are_same = std::conjunction<std::is_same<Head, Tail>...>;
		template<typename... Tail, class = std::enable_if_t<are_same<T, Tail...>::value, void>>
		bool any_of(Tail... args) const noexcept {
			bitset flags;
			flags = (flags | ... | args);
			for (size_t i = 0; i < typeSize; i++) {
				bitType tmp = flags.m_data[i] & m_data[i];
				if (tmp != 0)
					return true;
			}
			return false;
		}
		template<typename... Tail, class = std::enable_if_t<are_same<T, Tail...>::value, void>>
		bitset get_and_clear(Tail... args) noexcept {
			bitset flags;
			flags = (flags | ... | args);
			for (size_t i = 0; i < typeSize; i++) {
				bitType tmp = m_data[i];
				m_data[i] &= ~(flags.m_data[i]);
				flags.m_data[i] &= tmp;
			}
			return flags;
		}
		bitset& set() noexcept {
			for (size_t i = 0; i < typeSize; i++) {
				m_data[i] = static_cast<bitType>(~bitType{0});
			}
			return *this;
		}
		bitset& set(T _idx) noexcept {
			const size_t idx = static_cast<size_t>(_idx);
			assert(idx < bitSize);
			auto& word = m_data[idx / bitsPerWord];
			const auto bit = 1u << (idx % bitsPerWord);
			word |= static_cast<bitType>(bit);
			return *this;
		}
		bitset& reset() noexcept {
			for (size_t i = 0; i < typeSize; i++) {
				m_data[i] = 0u;
			}
			return *this;
		}
		bitset& reset(T _idx) noexcept {
			const size_t idx = static_cast<size_t>(_idx);
			assert(idx < bitSize);
			auto& word = m_data[idx / bitsPerWord];
			const auto bit = 1u << (idx % bitsPerWord);
			word &= ~bit;
			return *this;
		}
		operator bool() const noexcept {
			return any();
		}
		size_t count() const noexcept {
			size_t ret = 0;
			for (size_t i = 0; i < typeSize; i++) {
				ret += std::popcount(m_data[i]);
			}
			return ret;
		}
		unsigned long long to_ullong() const {
			if constexpr (bitSize == 0)
				return 0;
			else {
				return m_data[0];
			}
		}
		unsigned long to_ulong() const {
			if constexpr (bitSize == 0)
				return 0;
			else {
				return m_data[0];
			}
		}
		[[nodiscard]] constexpr size_t size() const noexcept {
			return bitSize;
		}
		[[nodiscard]] bool any() const noexcept {
			for (size_t i = 0; i < typeSize; i++) {
				if (m_data[i] != 0)
					return true;
			}
			return false;
		}
		[[nodiscard]] bool none() const noexcept {
			for (size_t i = 0; i < typeSize; i++) {
				if (m_data[i] != 0)
					return false;
			}
			return true;
		}
		[[nodiscard]] bool all() const noexcept {
			if constexpr (bitSize == 0)
				return true;

			constexpr bool no_padding = (bitSize % bitsPerWord == 0);
			for (size_t i = 0; i < typeSize - !no_padding; i++) {
				if (m_data[i] != ~bitType{0u})
					return false;
			}
			return no_padding || m_data[typeSize - 1] == (static_cast<bitType>(1) << (bitSize % bitsPerWord)) - 1 ;
		}
		bitset& flip() noexcept {
			for (size_t i = 0; i < typeSize; i++) {
				m_data[i] = ~m_data[i];
			}
			return *this;
		}
		bitset operator~() const noexcept {
			return bitset(*this).flip();
		}
		bitset& operator^=(const bitset& rhs) {
			for (size_t i = 0; i < typeSize; i++) {
				m_data[i] ^= rhs.m_data[i];
			}
			return *this;
		}
		bitset& operator&=(const bitset& rhs) {
			for (size_t i = 0; i < typeSize; i++) {
				m_data[i] &= rhs.m_data[i];
			}
			return *this;
		}
		bitset& operator|=(const bitset& rhs) {
			for (size_t i = 0; i < typeSize; i++) {
				m_data[i] |= rhs.m_data[i];
			}
			return *this;
		}		
		bool operator==(const bitset& rhs) const noexcept {
			if constexpr (bitSize == 0)
				return true;
			return memcmp(m_data.data(), rhs.m_data.data(), typeSize * sizeof(bitType)) == 0;
		}
	private:
		std::array<bitType, typeSize> m_data{};
	};
	
	template<size_t bitSize, typename T>
	inline bitset<bitSize, T> operator!=(const bitset<bitSize, T>& lhs, const bitset<bitSize, T>& rhs) {
		return !(lhs == rhs);
	}
	template<size_t bitSize, typename T>
	inline bitset<bitSize, T> operator^(const bitset<bitSize, T>& lhs, const bitset<bitSize, T>& rhs) {
		bitset<bitSize, T> ret = lhs;
		return ret ^= rhs;
	}
	template<size_t bitSize, typename T>
	inline bitset<bitSize, T> operator&(const bitset<bitSize, T>& lhs, const bitset<bitSize, T>& rhs) {
		bitset<bitSize, T> ret = lhs;
		return ret &= rhs;
	}
	template<size_t bitSize, typename T>
	inline bitset<bitSize, T> operator|(const bitset<bitSize, T>& lhs, const bitset<bitSize, T>& rhs) {
		bitset<bitSize, T> ret = lhs;
		return ret |= rhs;
	}
	template<size_t bitSize, typename T>
	inline bitset<bitSize, T> operator|(const bitset<bitSize, T>& lhs, const T& rhs) {
		bitset<bitSize, T> ret;
		ret.set(rhs);
		return ret |= lhs;
	}
	template<typename T, size_t size, size_t Tsize = 4>
	class bitarray {
	public:
		void set(T t, size_t idx) noexcept {
			if (idx & 0x1) {
				m_data[idx / 2] &= std::byte(0xf0u);
				m_data[idx / 2] |= static_cast<std::byte>(static_cast<uint32_t>(t)<< 4u);
			}
			else {
				m_data[idx / 2] &= std::byte(0x0fu);
				m_data[idx / 2] |= static_cast<std::byte>(static_cast<uint32_t>(t) & 0xfu);
			}
		}
		T get(size_t idx) const noexcept {
			if (idx & 0x1) {
				return static_cast<T>(static_cast<uint32_t>(m_data[idx / 2]) >> 4u);
			}
			else {
				//even steven, meaning lower nibble
				return static_cast<T>(static_cast<uint32_t>(m_data[idx / 2]) & 0xfu);
			}
		}
	private:
		static_assert(Tsize == 4, "Only nibbles supported currently");
		std::array<std::byte, size / 2> m_data;
	};
	
}
#endif // UTVECTOR_H!
