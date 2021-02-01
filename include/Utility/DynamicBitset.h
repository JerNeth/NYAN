#ifndef UTVECTOR_H
#define UTVECTOR_H
#pragma once
#include <stdlib.h>
#include <cstddef>
namespace Utility {
	constexpr size_t occupancy_size = (sizeof(size_t) * 8u);
	constexpr size_t full_mask = ~static_cast<size_t>(0u);
	constexpr size_t none_mask = static_cast<size_t>(0u);
	constexpr size_t single_mask = static_cast<size_t>(1u);
	class DynamicBitset {
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
			std::memcpy(m_occupancy, other.m_occupancy, m_size);
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
				std::memcpy(m_occupancy, other.m_occupancy, m_size);
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
			size_t idx = idx = bucket * sizeof(size_t) * 8;
			for (; test(idx); idx++) {

			}
			return idx;
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
	
}
#endif // UTVECTOR_H!
