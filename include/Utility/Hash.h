#pragma once
#ifndef HASH_H
#define HASH_H
#include <cstdint>
namespace Utility {
	typedef uint64_t HashValue;
	struct Hasher {
		Hasher() : hash(0xcbf29ce484222325ull){
		}
		Hasher(HashValue init) : hash(init){

		}
		template<typename T>
		HashValue operator()(const T& t) {
			constexpr const HashValue prime = 0x100000001b3ull;
			const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&t);
			for (size_t i = 0; i < sizeof(T); i++) {
				hash ^= static_cast<HashValue>(bytes[i]);
				hash *= prime;
			}
			return hash;
		}
		HashValue operator()() {
			return hash;
		}
	private:
		HashValue hash = 0xcbf29ce484222325ull;
	};
	template<typename T>
	struct Hash {
		HashValue operator()(const T& t) const {
			const HashValue prime = 0x100000001b3ull;
			HashValue hash = 0xcbf29ce484222325ull;
			const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&t);
			for (size_t i = 0; i < sizeof(T); i++) {
				hash ^= static_cast<HashValue>(bytes[i]);
				hash *= prime;
			}
			return hash;
		}
	};
	template<typename T>
	struct DataHash {
		HashValue operator()(const T* t, size_t size) const {
			const HashValue prime = 0x100000001b3ull;
			HashValue hash = 0xcbf29ce484222325ull;
			const uint8_t* bytes = reinterpret_cast<const uint8_t*>(t);
			for (size_t i = 0; i < size; i++) {
				hash ^= static_cast<HashValue>(bytes[i]);
				hash *= prime;
			}
			return hash;
		}
	};
	template<typename T>
	struct VectorHash {
		HashValue operator()(std::vector<T> data) const {
			const HashValue prime = 0x100000001b3ull;
			HashValue hash = 0xcbf29ce484222325ull;
			for (size_t i = 0; i < data.size(); i++) {
				size_t hashInner = std::hash<T>()(data[i]);
				const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&hashInner);
				for (size_t j = 0; j < sizeof(size_t); j++) {
					hash ^= static_cast<HashValue>(bytes[j]);
					hash *= prime;
				}
			}
			return hash;
		}
	};
}
#endif