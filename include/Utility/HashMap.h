#pragma once
#ifndef HASHMAP_H
#define HASHMAP_H
#include <optional>
#include <new>
#include "Hash.h"
#include "Pool.h"
namespace Utility {
	template<typename T, size_t bucketSize = (std::hardware_constructive_interference_size / (sizeof(HashValue) + sizeof(T)))>
	struct alignas(std::hardware_constructive_interference_size) HashBucket {
		HashBucket() {
			std::memset(data.data(), 0, data.size() * sizeof(std::pair<HashValue, T>));
		}
		~HashBucket() {
			for (size_t i = 0; i < data.size(); i++) {
				if ((data[i].first == 0x0ull) && (data[i].second == 0x0ull))
					data[i].second.~T();
			}
		}
		std::array<std::pair<HashValue, T>, bucketSize> data;
		size_t get_first_empty() const{
			for (size_t i = 0; i < data.size(); i++) {
				if ((data[i].first == 0x0ull) && (data[i].second == 0x0ull))
					return i;
			}
			return data.size();
		}
		std::optional<T> get(HashValue hash) {
			for (size_t i = 0; i < data.size(); i++) {
				if (data[i].first == hash)
					return data[i].second;
			}
			return std::nullopt;
		}
		void remove(HashValue hash) {
			for (size_t i = 0; i < data.size(); i++) {
				if (data[i].first == hash)
					std::memset(&data[i], 0, sizeof(std::pair<HashValue, T>));
			}
		}
		std::pair<HashValue, T>& operator[](size_t idx) {
			return data[idx];
		}
		size_t end() const {
			return data.size();
		}
		size_t begin() const{
			return 0;
		}
		bool contains(HashValue hash) {
			for (size_t i = 0; i < data.size(); i++)
				if (data[i].first == hash)
					return true;
			return false;
		}
	};
	template<typename T, size_t hashBucketSize = (std::hardware_constructive_interference_size / (sizeof(HashValue) + sizeof(T)))>
	class HashMap {
	private:
		constexpr size_t capacity() {
			return 1ull << size;
		}
		constexpr size_t mask() {
			return capacity() - 1ull;
		}
		constexpr size_t mod(size_t idx) {
			return idx & mask();
		}
	public:
		HashMap() {
			data = new HashBucket<T>[1];
		}
		~HashMap() {
			if (data)
				delete[] data;
		}
		HashMap(HashMap& other) = delete;
		void operator=(HashMap& other) = delete;
		void clear() {
			if (data) {
				std::memset(data, 0, sizeof(HashBucket<T>) * capacity());
			}
		}
		std::optional<T> get(HashValue hash) {
			size_t idx = mod(hash);
			return data[idx].get(hash);
		}
		bool try_insert(HashValue hash, const T& value) {
			size_t idx = mod(hash);
			if (auto bucket_idx = data[idx].get_first_empty(); bucket_idx != data[idx].end()) {
				data[idx][bucket_idx] = std::make_pair(hash, value);
				return true;
			}
			return false;
		}
		
		void insert(HashValue hash, const T& value) {
			//Check if bucket is full
			if (!try_insert(hash, value)) {
				bool succeded = true;
				auto oldSize = size;
				do {
					auto oldData = data;
					data = new HashBucket<T>[1ull << ++size];
					for (size_t i = 0; i < (1ull << oldSize); i++) {
						for (size_t j = 0; j < oldData[i].end(); j++) {
							auto [tmpHash, tmpData] = oldData[i][j];
							if ((tmpHash != 0ull) || (tmpData != 0ull)) {
								succeded &= try_insert(tmpHash, tmpData);
								if (!succeded)
									goto cleanup;
							}
						}
					}
					succeded &= try_insert(hash, value);
					if (!succeded) {
					cleanup:
						delete[] data;
						data = oldData;
					}
				} while (!succeded);
			}
		}
		void remove(HashValue hash) {
			size_t idx = mod(hash);
			data[idx].remove(hash);
		}
		bool contains(HashValue hash) {
			size_t idx = mod(hash);
			return data[idx].contains(hash);
		}
	private:
		HashBucket<T, hashBucketSize>* data = nullptr;
		size_t size = 0;
	};
	template<typename T>
	class OwningHashMapHandle;
	template<typename T>
	class OwningHashMap {
	public:

		template<class... Args>
		[[nodiscard]] OwningHashMapHandle<T> emplace(HashValue hash,Args&&... args) noexcept {
			m_hashMap.insert(hash, m_dataPool.emplace_intrusive(std::forward<Args>(args)...));
			return OwningHashMapHandle(hash, this);
		}
		void remove(HashValue hash) noexcept {
			auto val = m_hashMap.get(hash);
			if (val) {
				m_dataPool.remove(*val);
			}
		}
		T* get_ptr(HashValue hash) {
			auto val = m_hashMap.get(hash);
			if (val) {
				return m_dataPool.get_ptr(*val);
			}
			return nullptr;
		}
		T& get(HashValue hash) {
			auto val = m_hashMap.get(hash);
			if (val) {
				return m_dataPool.get(*val);
			}
			throw std::runtime_error("Invalid hashvalue");
		}
		void clear() {
			m_dataPool.clear();
			m_hashMap.clear();
		}
		size_t size() {
			return m_dataPool.size();
		}
		bool contains(HashValue hash) {
			return m_hashMap.contains(hash);
		}
	private:
		Pool<T> m_dataPool;
		HashMap<size_t> m_hashMap;
	};
	template<typename T>
	class OwningHashMapHandle {
		friend class OwningHashMap<T>;
		OwningHashMapHandle(HashValue id, OwningHashMap<T>* pool) : m_id(id), ptr_hashmap(pool) {

		}
	public:
		OwningHashMapHandle() = default;
		~OwningHashMapHandle() {
			if (ptr_count) {
				if (ptr_count == reinterpret_cast<size_t*>(~0ull))
					return;
				(*ptr_count)--;
				if (*ptr_count == 0) {
					delete ptr_count;
					ptr_hashmap->remove(m_id);
				}
			}
			else {
				ptr_hashmap->remove(m_id);
			}
		}
		OwningHashMapHandle(OwningHashMapHandle& other) :
			m_id(other.m_id),
			ptr_hashmap(other.ptr_hashmap)
		{
			if (!other.ptr_count) {
				other.ptr_count = new size_t(1);
			}
			ptr_count = other.ptr_count;
			(*ptr_count)++;
		}
		OwningHashMapHandle(OwningHashMapHandle&& other) noexcept :
			m_id(other.m_id),
			ptr_hashmap(other.ptr_hashmap),
			ptr_count(other.ptr_count)
		{
			// I don't like it but I don't have a better solution
			if (this != &other)
				other.ptr_count = reinterpret_cast<size_t*>(~0ull);
		}
		OwningHashMapHandle& operator=(OwningHashMapHandle& other)
		{
			if (this != &other) {
				m_id = other.m_id;
				ptr_hashmap = other.ptr_hashmap;
				if (!other.ptr_count) {
					other.ptr_count = new size_t(1);
				}
				ptr_count = other.ptr_count;
				(*ptr_count)++;
			}
			return *this;
		}
		OwningHashMapHandle& operator=(OwningHashMapHandle&& other)
		{
			if (this != &other) {
				m_id = other.m_id;
				ptr_hashmap = other.ptr_hashmap;
				ptr_count = other.ptr_count;
				other.ptr_count = reinterpret_cast<size_t*>(~0ull);
			}
			return *this;
		}
		operator bool() {
			return ptr_hashmap;
		}
		void remove() {
			ptr_hashmap->remove(m_id);
		}
		T* operator->() {
			return ptr_hashmap->get_ptr(m_id);
		}
		T& operator*() {
			return ptr_hashmap->get(m_id);
		}
		explicit operator T* () {
			return ptr_hashmap->get_ptr(m_id);
		}

	private:
		HashValue m_id;
		OwningHashMap<T>* ptr_hashmap = nullptr; //Hashmap this handle refers to
		size_t* ptr_count = nullptr;
	};
}
#endif