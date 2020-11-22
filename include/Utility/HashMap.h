#pragma once
#ifndef HASHMAP_H
#define HASHMAP_H
#include <optional>
#include <new>
#include "Hash.h"
namespace Utility {

	template<typename T>
	struct alignas(std::hardware_constructive_interference_size) HashBucket {
		
		std::array<std::pair<HashValue, T>,std::hardware_constructive_interference_size / (sizeof(HashValue) + sizeof(T))> data;
		size_t get_first_empty() const{
			for (size_t i = 0; i < data.size(); i++) {
				if (data[i].second == nullptr)
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
	};
	template<typename T>
	class HashMap {
	private:

	public:
		HashMap() {
			size = 0;
			data = new HashBucket<T>[1];
			std::memset(data, 0, sizeof(HashBucket<T>));
		}
		~HashMap() {
			if (data)
				delete[] data;
		}
		HashMap(HashMap& other) = delete;
		void operator=(HashMap& other) = delete;
		void clear() {
			std::memset(data, 0, sizeof(HashBucket<T>) * (1ull << size));
		}
		std::optional<T> get(HashValue hash) {
			size_t idx = hash & ((1ull << size) - 1ull);
			return data[idx].get(hash);
		}
		bool try_insert(HashValue hash, T value) {
			size_t idx = hash & ((1ull << size) - 1ull);
			if (auto bucket_idx = data[idx].get_first_empty(); bucket_idx != data[idx].end()) {
				data[idx][bucket_idx] = std::make_pair(hash, value);
				return true;
			}
			return false;
		}
		void insert(HashValue hash, T value){
			//Check if bucket is full
			if(!try_insert(hash, value)) {
				bool succeded = true;
				auto oldSize = size;
				do {
					auto oldData = data;
					data = new HashBucket<T>[1ull << ++size];
					std::memset(data, 0, sizeof(HashBucket<T>) * 1 << size);
					for (size_t i = 0; i < (1ull << oldSize); i++) {
						for (size_t j = 0; j < oldData[i].end(); j++) {
							auto [tmpHash, tmpData] = oldData[i][j];
							if (tmpData != nullptr) {
								succeded &= try_insert(tmpHash, tmpData);
								if(!succeded)
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
			size_t idx = hash & ((1ull << size) - 1ull);
			data[idx].remove(hash);
		}
	private:
		HashBucket<T>* data;
		size_t size;
	};
	template<typename T>
	class TemporaryHashMap {
	public:

	private:
		std::vector<T> data;
		HashMap<T*> hashMap;
	};
}
#endif