#pragma once
#ifndef HASHMAP_H
#define HASHMAP_H
#include "Hash.h"
#include "Pool.h"

#include <optional>
#include <new>
#include <unordered_map>
#include <algorithm>

// https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
// Since Clang and GCC (<12) do not support it
#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
// 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
constexpr std::size_t hardware_constructive_interference_size
= 2 * sizeof(std::max_align_t);
constexpr std::size_t hardware_destructive_interference_size
= 2 * sizeof(std::max_align_t);
#endif
namespace Utility {
	
	template<typename T, size_t bucketSize = (hardware_constructive_interference_size / (sizeof(HashValue) + sizeof(T)))>
	struct HashBucket {
		HashBucket() {
			memset(data.data(), 0, data.size() * sizeof(std::pair<HashValue, T>));
		}
		~HashBucket() {
			for (size_t i = 0; i < data.size(); i++) {
				if (occupancy.test(i)) {
					data[i].second.~T();
				}
			}
		}
		Utility::bitset<bucketSize> occupancy;
		std::array<std::pair<HashValue, T>, bucketSize> data;
		size_t get_first_empty() const {
			for (size_t i = 0; i < data.size(); i++) {
				if (!occupancy.test(i))
					return i;
			}
			return data.size();
		}
		std::optional<T> get(HashValue hash) {
			for (size_t i = 0; i < data.size(); i++) {
				if (data[i].first == hash) {
					assert(occupancy.test(i));
					return data[i].second;
				}
			}
			return std::nullopt;
		}
		void remove(HashValue hash) {
			for (size_t i = 0; i < data.size(); i++) {
				if (data[i].first == hash) {
					assert(occupancy.test(i));
					data[i].second.~T();
					occupancy.reset(i);
					memset(&data[i], 0, sizeof(std::pair<HashValue, T>));
					return;
				}
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
		bool contains_direct(size_t idx) {
			return occupancy.test(idx);
		}
	};
	template<typename T, size_t hashBucketSize = std::max((hardware_constructive_interference_size / (sizeof(HashValue) + sizeof(T))), 1ull)>
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
				memset(data, 0, sizeof(HashBucket<T>) * capacity());
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
				data[idx].occupancy.set(bucket_idx);
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
					succeded = true;
					auto oldData = data;
					assert(size - oldSize < 3); //Otherwise is probably a leak
					data = new HashBucket<T>[1ull << ++size];
					for (size_t i = 0; i < (1ull << oldSize); i++) {
						for (size_t j = 0; j < oldData[i].end(); j++) {
							if (!oldData[i].contains_direct(j))
								continue;
							auto [tmpHash, tmpData] = oldData[i][j];
							succeded &= try_insert(tmpHash, tmpData);
							if (!succeded)
								goto cleanup;
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

	template<typename Key, typename Value>
	class NonInvalidatingMap {
	public:
		template<class... Args>
		Value& emplace(const Key& key, Args&&... args) {
			if (auto val = m_hashMap.find(key); val != m_hashMap.end()) {
				m_storage.remove(val->second);
			}
			auto id = m_storage.emplace_intrusive(std::forward<Args>(args)...);
			m_hashMap[key] = id;
			return m_storage.get(id);
		}
		Value& get(const Key& key) {
			auto id = m_hashMap.find(key);
			assert(id != m_hashMap.end());
			return m_storage.get(id->second);
		}
		const Value& get(const Key& key) const {
			auto id = m_hashMap.find(key);
			assert(id != m_hashMap.end());
			return m_storage.get(id->second);
		}
		void remove(const Key& key)  {
			auto id = m_hashMap.find(key);
			if (id == m_hashMap.end()) return;
			m_storage.remove(id->second);
		}
		Value& operator[](const Key& key) {
			return get(key);
		}
		Value& get_direct(size_t id) {
			return m_storage.get(id);
		}
		const Value& get_direct(size_t id) const {
			return m_storage.get(id);
		}
		template<typename Functor>
		void for_each(Functor functor) {
			m_storage.for_each(functor);
		}
		bool contains(const Key& key) const {
			return m_hashMap.contains(key);
		}
	private:
		//KeyHashMap<Key, size_t> m_hashMap;
		std::unordered_map<Key, size_t> m_hashMap;
		LinkedBucketList<Value, 16> m_storage;
	};
}
#endif