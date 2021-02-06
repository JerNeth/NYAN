#ifndef UTPOOL_H
#define UTPOOL_H
#pragma once
#include "DynamicBitset.h"
namespace Utility {
	template<typename T>
	class PoolHandle;
	template<typename T>
	class Pool {
	public:
		Pool() = default;
		~Pool() noexcept {
			if (m_data) {
				for (size_t i = 0; i < m_size; i++) {
					if (m_occupancy.test(i)) {
						m_data[i].~T();
					}
				}
				free(m_data);
			}
		}
		Pool(Pool&) = delete;
		Pool(Pool&& other) noexcept :
			m_data(other.m_data),
			m_occupancy(other.m_occupancy),
			m_size(other.m_size),
			m_capacity(other.m_capacity)
		{
			other.m_data = nullptr;
			other.m_size = 0;
			other.m_capacity = 0;
		}
		void reserve(size_t new_capacity) {
			if (new_capacity > m_capacity) {
				auto data = static_cast<T*>(realloc(m_data, new_capacity * sizeof(T)));
				if (!data)
					throw std::runtime_error("Could not realloc");
				m_data = data;
				m_occupancy.reserve(new_capacity);
				m_capacity = new_capacity;

			}
		}
		Pool& operator=(Pool&) = delete;
		Pool& operator=(Pool&& other) noexcept {
			if (this != &other) {
				*this = Pool(other);
			}
		}
		template<class... Args>
		[[nodiscard]] size_t emplace_intrusive(Args&&... args) noexcept {
			size_t idx = 0;
			if (m_size >= m_capacity) {
				size_t new_capacity = m_capacity + 1;
				//hardcoded growth factor for now
				reserve(new_capacity + new_capacity / 2);
				idx = m_size;
			}
			else {
				idx = m_occupancy.find_empty();
			}
			new (m_data + idx) T(std::forward<Args>(args)...);
			m_occupancy.set(idx);
			m_size++;
			return idx;
		}
		template<class... Args>
		[[nodiscard]] PoolHandle<T> emplace(Args&&... args) noexcept {
			return PoolHandle<T>(emplace_intrusive(std::forward<Args>(args)...), this);
		}
		
		void clear() noexcept {
			if (m_data) {
				for (size_t i = 0; i < m_size; i++) {
					if (m_occupancy.test(i)) {
						m_data[i].~T();
					}
				}
			}
			m_size = 0;
			m_occupancy.clear();
		}
		void remove(size_t idx) noexcept {
			if (m_data) {
				if (m_occupancy.test(idx)) {
					m_data[idx].~T();
					m_occupancy.clear(idx);
					m_size--;
				}
			}
		}
		T* get_ptr(size_t idx) noexcept {
			assert(idx < m_capacity);
			if (m_data && m_occupancy.test(idx)) {
				return &m_data[idx];
			}
			return nullptr;
		}
		const T* get_ptr(size_t idx) const noexcept {
			assert(idx < m_capacity);
			if (m_data && m_occupancy.test(idx)) {
				return &m_data[idx];
			}
			return nullptr;
		}
		T& get(size_t idx) {
			assert(idx < m_capacity);
			if (m_data && m_occupancy.test(idx)) {
				return m_data[idx];
			}
			throw std::exception("Invalid idx");
		}
		const T& get(size_t idx) const {
			assert(idx < m_capacity);
			if (m_data && m_occupancy.test(idx)) {
				return m_data[idx];
			}
			throw std::exception("Invalid idx");
		}
		size_t size() const noexcept {
			return m_size;
		}
	private:
		T* m_data = nullptr;
		DynamicBitset m_occupancy;
		size_t m_size = 0;
		size_t m_capacity = 0;
	};
	template<typename T>
	class PoolHandle {
		friend class Pool<T>;
		PoolHandle(size_t id, Pool<T>* pool) : m_id(id), ptr_pool(pool) {

		}
	public:
		~PoolHandle() {
			if (ptr_count) {
				// I don't like it but I don't have a better solution TODO
				if (ptr_count == reinterpret_cast<size_t*>(~0ull))
					return;
				(*ptr_count)--;
				if (*ptr_count == 0) {
					delete ptr_count;
					ptr_pool->remove(m_id);
				}
			}
			else {
				ptr_pool->remove(m_id);
			}
		}
		PoolHandle(const PoolHandle& other) :
			m_id(other.m_id),
			ptr_pool(other.ptr_pool)
		{
			if (!other.ptr_count) {
				other.ptr_count = new size_t(1);
			}
			ptr_count = other.ptr_count;
			(*ptr_count)++;
		}
		PoolHandle(PoolHandle& other) :
			m_id(other.m_id),
			ptr_pool(other.ptr_pool)
		{
			if (!other.ptr_count) {
				other.ptr_count = new size_t(1);
			}
			ptr_count = other.ptr_count;
			(*ptr_count)++;
		}
		PoolHandle(PoolHandle&& other) :
			m_id(other.m_id),
			ptr_pool(other.ptr_pool),
			ptr_count(other.ptr_count)
		{
			// I don't like it but I don't have a better solution TODO
			if(this != &other)
				other.ptr_count = reinterpret_cast<size_t*>(~0ull);
		}
		PoolHandle& operator=(PoolHandle& other)
		{
			if (this != &other) {
				m_id=other.m_id;
				ptr_pool = other.ptr_pool;
				if (!other.ptr_count) {
					other.ptr_count = new size_t(1);
				}
				ptr_count = other.ptr_count;
				(*ptr_count)++;
			}
			return *this;
		}
		PoolHandle& operator=(PoolHandle&& other)
		{
			if (this != &other) {
				m_id = other.m_id;
				ptr_pool = other.ptr_pool;
				ptr_count = other.ptr_count;
				other.ptr_count = reinterpret_cast<size_t*>(~0ull);
			}
			return *this;
		}
		void remove() {
			ptr_pool->remove(m_id);
		}
		T* operator->() {
			return ptr_pool->get_ptr(m_id);
		}
		T& operator*() {
			return ptr_pool->get(m_id);
		}
		operator T* () {
			return ptr_pool->get_ptr(m_id);
		}
	private:
		size_t m_id;
		Pool<T>* ptr_pool = nullptr; //List this handle refers to
		mutable size_t* ptr_count = nullptr;
	};
}

#endif // UTPOOL_H!
