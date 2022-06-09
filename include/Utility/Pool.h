#ifndef UTPOOL_H
#define UTPOOL_H
#pragma once
#include "DynamicBitset.h"
#include <assert.h>
namespace Utility {
	template<typename T, typename Container>
	class ObjectHandle;
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
		[[nodiscard]] ObjectHandle<T, Pool<T>> emplace(Args&&... args) noexcept {
			return ObjectHandle<T, Pool<T>>(emplace_intrusive(std::forward<Args>(args)...), this);
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
			throw std::range_error{ "Invalid idx" };
		}
		const T& get(size_t idx) const {
			assert(idx < m_capacity);
			if (m_data && m_occupancy.test(idx)) {
				return m_data[idx];
			}
			throw std::range_error{ "Invalid idx" };
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

	constexpr size_t invalidObjectId = ~0ull;
	template<typename T, typename Container>
	class ObjectHandle {
	public:
		ObjectHandle() = delete;
		ObjectHandle(size_t id, Container* container) : 
			m_id(id), 
			ptr_container(container) 
		{
			assert(ptr_container);
		}
		~ObjectHandle() 
		{
			destructor();
		}
		ObjectHandle(const ObjectHandle& other) :
			m_id(other.m_id),
			ptr_container(other.ptr_container)
		{
			if (!other.ptr_count) {
				other.ptr_count = new size_t(1);
			}
			ptr_count = other.ptr_count;
			assert(ptr_count != nullptr);
			(*ptr_count)++;
		}
		ObjectHandle(ObjectHandle& other) :
			m_id(other.m_id),
			ptr_container(other.ptr_container)
		{
			if (!other.ptr_count) {
				other.ptr_count = new size_t(1);
			}
			ptr_count = other.ptr_count;

			assert(ptr_count != nullptr);
			(*ptr_count)++;
		}
		ObjectHandle(ObjectHandle&& other) noexcept :
			m_id(other.m_id),
			ptr_container(other.ptr_container),
			ptr_count(other.ptr_count)
		{
			if (this != &other) {
				other.ptr_container = nullptr;
				other.ptr_count = nullptr;
			}
		}
		ObjectHandle& operator=(const ObjectHandle& other)
		{
			if (this != &other) {
				destructor();
				m_id=other.m_id;
				ptr_container = other.ptr_container;
				if (!other.ptr_count) {
					other.ptr_count = new size_t(1);
				}
				ptr_count = other.ptr_count;

				assert(ptr_count != nullptr);
				(*ptr_count)++;
			}
			return *this;
		}
		ObjectHandle& operator=(ObjectHandle&& other) noexcept
		{
			if (this != &other) {
				destructor();
				m_id = other.m_id;
				ptr_container = other.ptr_container;
				ptr_count = other.ptr_count;
				other.ptr_container = nullptr;
				other.ptr_count = nullptr;
			}
			return *this;
		}
		operator const T& () const noexcept {
			assert(ptr_container);
			return ptr_container->get(m_id);
		}
		operator T& () noexcept {
			assert(ptr_container);
			return ptr_container->get(m_id);
		}
		void remove() {
			assert(ptr_container);
			ptr_container->remove(m_id);
		}
		T* operator->() {
			assert(ptr_container);
			return ptr_container->get_ptr(m_id);
		}
		T& operator*() {
			assert(ptr_container);
			return ptr_container->get(m_id);
		}
		const T* operator->() const {
			assert(ptr_container);
			return ptr_container->get_ptr(m_id);
		}
		const T& operator*() const {
			assert(ptr_container);
			return ptr_container->get(m_id);
		}
		operator bool() const {
			return ptr_container != nullptr;
		}
		operator T* () {
			assert(ptr_container);
			return ptr_container->get_ptr(m_id);
		}
		operator const T* const () {
			assert(ptr_container);
			return ptr_container->get_ptr(m_id);
		}
	private:
		void destructor() {
			if (ptr_count != nullptr) {
				(*ptr_count)--;
				if (*ptr_count == 0) {
					delete ptr_count;
					assert(ptr_container);
					ptr_container->remove(m_id);
				}
			}
			else {
				if(ptr_container)
					ptr_container->remove(m_id);
			}
		}
		size_t m_id = invalidObjectId;
		Container* ptr_container = nullptr; //List this handle refers to
		mutable size_t* ptr_count = nullptr;
	};
}

#endif // UTPOOL_H!
