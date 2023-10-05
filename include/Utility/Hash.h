#pragma once
#ifndef HASH_H
#define HASH_H
#include <cstdint>
#include <vector>
namespace Utility {
	using HashValue = uint64_t;
	struct Hasher {
		constexpr Hasher() : hash(0xcbf29ce484222325ull){
		}
		constexpr Hasher(HashValue init) : hash(init){

		}
		template<typename T>
		constexpr HashValue operator()(const T& t) {
			static constexpr HashValue prime = 0x100000001b3ull;
			const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&t);
			for (size_t i = 0; i < sizeof(T); i++) {
				hash ^= static_cast<HashValue>(bytes[i]);
				hash *= prime;
			}
			return hash;
		}
		template<>
		constexpr HashValue operator()(const std::string& t) {
			constexpr HashValue prime = 0x100000001b3ull;
			for (size_t i = 0; i < t.size(); i++) {
				hash ^= static_cast<HashValue>(t[i]);
				hash *= prime;
			}
			return hash;
		}
		template<>
		constexpr HashValue operator()(const uint32_t& t) {
			constexpr HashValue prime = 0x100000001b3ull;
			hash ^= static_cast<HashValue>(t);
			hash *= prime;

			return hash;
		}
		template<>
		constexpr HashValue operator()(const int32_t& t) {
			constexpr HashValue prime = 0x100000001b3ull;
			hash ^= static_cast<HashValue>(t);
			hash *= prime;

			return hash;
		}
		template<>
		constexpr HashValue operator()(const int64_t& t) {
			constexpr HashValue prime = 0x100000001b3ull;
			hash ^= static_cast<HashValue>(t);
			hash *= prime;

			return hash;
		}
		template<>
		constexpr HashValue operator()(const HashValue& t) {
			constexpr HashValue prime = 0x100000001b3ull;
			hash ^= t;
			hash *= prime;
			return hash;
		}
		constexpr HashValue operator()() {
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
	template<>
	struct Hash<std::string> {
		HashValue operator()(const std::string& string) const {
			const HashValue prime = 0x100000001b3ull;
			HashValue hash = 0xcbf29ce484222325ull;
			const char* bytes = string.data();
			for (size_t i = 0; i < string.size(); i++) {
				hash ^= static_cast<HashValue>(bytes[i]);
				hash *= prime;
			}
			return hash;
		}
	};
	struct DataHash {
		template<typename T>
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
	struct DataHasher {
		DataHasher() : hash(0xcbf29ce484222325ull) {
		}
		DataHasher(HashValue init) : hash(init) {

		}
		HashValue operator()() const {
			return hash;
		}
		HashValue operator()(const T* t, size_t size) {
			const HashValue prime = 0x100000001b3ull;
			const uint8_t* bytes = reinterpret_cast<const uint8_t*>(t);
			for (size_t i = 0; i < size; i++) {
				hash ^= static_cast<HashValue>(bytes[i]);
				hash *= prime;
			}
			return hash;
		}
	private:
		HashValue hash = 0xcbf29ce484222325ull;
	};
	template<typename T>
	struct VectorHash {
		HashValue operator()(const std::vector<T>& data) const {
			const HashValue prime = 0x100000001b3ull;
			HashValue hash = 0xcbf29ce484222325ull;
			const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.data());
			for (size_t i = 0; i < data.size() * sizeof(T); i++) {
				hash ^= static_cast<HashValue>(bytes[i]);
				hash *= prime;
			}
			return hash;
		}
	};
}
#endif