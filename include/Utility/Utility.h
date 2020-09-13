#pragma once
#include <chrono>
#include <memory>
#include <bitset>
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
	typedef uint64_t HashValue;
	struct Hasher {
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
	
	inline uint32_t fast_log2(uint32_t num) {
		double ff = static_cast<double>(num | 1);
		uint32_t tmp;
		std::memcpy(&tmp, reinterpret_cast<const char*>(&ff)+sizeof(uint32_t), sizeof(uint32_t));
		return (tmp >> 20) - 1023;
	}
	template<typename T, size_t bucketSize>
	class ListBucket {
		using Bucket = ListBucket<T, bucketSize>;
	public:
		ListBucket(size_t zero_id_) : zero_id(zero_id_)
		{
			bucket = reinterpret_cast<T*>(malloc(sizeof(T)*bucketSize));
		}
		ListBucket(ListBucket&) = delete;
		ListBucket(ListBucket&& other) {
			other.next.swap(next);
			size = other.size;
			bucket = other.bucket;
			prev = other.prev;
			other.size = 0;
			other.bucket = nullptr;
			other.prev = nullptr;
		}
		ListBucket& operator=(ListBucket&) = delete;
		ListBucket& operator=(ListBucket&& other) {
			other.next.swap(next);
			size = other.size;
			bucket = other.bucket;
			prev = other.prev;
			other.size = 0;
			other.bucket = nullptr;
			other.prev = nullptr;
		}
		~ListBucket() {
			if (bucket) {
				for (size_t i = 0; i < bucketSize; i++)
					if(occupancy[i])
						bucket[i].~T();
				free(bucket);
			}
		}
		size_t insert(T t) {
			size_t i;
			for (i = 0; i < bucketSize; i++) {
				if (!occupancy.test(i)) {
					bucket[i] = t;
					occupancy.set(i);
					return zero_id + i;
				}
			}
			throw std::runtime_error("How?");
		}
		template<class... Args>
		size_t emplace(Args&&... args) {
			size_t i;
			for (i = 0; i < bucketSize; i++) {
				if (!occupancy.test(i)) {
					new(bucket + i) T(std::forward<Args>(args)...);
					occupancy.set(i);
					return zero_id + i;
				}
			}
			throw std::runtime_error("How?");
		}
		void delete_object(size_t id) {
			auto local_id = id - zero_id;
			assert(local_id < bucketSize);
			occupancy.reset(local_id);
			bucket[local_id].~T();
			std::memset(bucket + local_id, 0, sizeof(T));
		}
		T* get(size_t id) const{
			auto local_id = id - zero_id;
			assert(local_id < bucketSize);
			if (occupancy.test(local_id))
				return &bucket[local_id];
			else
				return nullptr;
		}
		bool full() {
			return occupancy.all();
		}
		ListBucket* get_next() {
			if (!next)
				next = std::make_unique<Bucket>(zero_id + bucketSize);
			return next.get();
		}
		const size_t zero_id;
		void print() {
			std::cout << "[";
			for (int i = 0; i < bucketSize; i++) {
				if (!occupancy[i])
					continue;
				std::cout << i + zero_id << ": " << bucket[i] << " ";
			}
			std::cout << "]\n";
			if (next)
				next->print();
		}
	private:
		std::bitset<bucketSize> occupancy = 0;
		T *bucket = nullptr;
		std::unique_ptr<Bucket> next = nullptr;
	};
	/// <summary>
	/// Linked Bucked List
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T, size_t bucketSize = 16>
	class LinkedBucketList {
		
		using Bucket = ListBucket<T, bucketSize>;
	public:
		LinkedBucketList() {
			
		}
		void destroy() {
			head.reset(nullptr);
		}
		size_t insert(T t) {
			if (!head) {
				head = std::make_unique< Bucket>(0);
			}
			Bucket* current = head.get();
			while (current->full()) {
				current = current->get_next();
			}
			return current->insert(t);
		}
		template<class... Args>
		size_t emplace(Args&&... args) {
			if (!head) {
				head = std::make_unique< Bucket>(0);
			}
			Bucket* current = head.get();
			while (current->full()) {
				current = current->get_next();
			}
			return current->emplace(std::forward<Args>(args)...);
		}
		void delete_object(size_t id) {
			if (!head)
				return;
			Bucket* current = head.get();
			while (id >= (current->zero_id + bucketSize)) {
				current = current->get_next();
				if (current == nullptr)
					return;
			}
			current->delete_object(id);
		}
		T* get(size_t id) const {
			if (!head)
				return nullptr;
			Bucket* current = head.get();
			while (id >= (current->zero_id+ bucketSize)) {
				current = current->get_next();
				if (current == nullptr)
					return nullptr;
			}
			return current->get(id);
		}
		void print() {
			std::cout << "LinkedBucketList [" << '\n';
			if (head)
				head->print();
			std::cout << "]" << '\n';
		}
	private:
		std::unique_ptr<Bucket> head = nullptr;
	};
}