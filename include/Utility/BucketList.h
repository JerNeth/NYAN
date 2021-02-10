#pragma once
#ifndef BUCKETLIST_H
#define BUCKETLIST_H
#include <iostream>
#include <bitset>
namespace Utility {
	template<typename T, typename Container>
	class ObjectHandle;
	template<typename T, size_t bucketSize>
	class ListBucket {
		using Bucket = ListBucket<T, bucketSize>;
	public:
		ListBucket(size_t zero_id_) : zero_id(zero_id_)
		{
		}
		ListBucket(ListBucket&) = delete;
		//Should not be used since we do not want to invalidate any pointers
		//Could be avoided with an extra allocation
		ListBucket(ListBucket&& other) = delete;
		ListBucket& operator=(ListBucket&) = delete;
		ListBucket& operator=(ListBucket&& other) = delete;
		~ListBucket() {
			for (size_t i = 0; i < bucketSize; i++)
				if (occupancy[i])
					reinterpret_cast<T*>(m_storage.data())[i].~T();
		}
		void clear() {
			for (size_t i = 0; i < bucketSize; i++)
				if (occupancy[i])
					reinterpret_cast<T*>(m_storage.data())[i].~T();
			occupancy.reset();
		}
		size_t insert(const T& t) {
			size_t i;
			for (i = 0; i < bucketSize; i++) {
				if (!occupancy.test(i)) {
					new (&reinterpret_cast<T*>(m_storage.data())[i]) T(t);
					occupancy.set(i);
					return zero_id + i;
				}
			}
			throw std::runtime_error("How?");
		}
		size_t insert(T&& t) {
			size_t i;
			for (i = 0; i < bucketSize; i++) {
				if (!occupancy.test(i)) {
					new (&reinterpret_cast<T*>(m_storage.data())[i]) T(std::move(t));
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
					new(reinterpret_cast<T*>(m_storage.data()) + i) T(std::forward<Args>(args)...);
					occupancy.set(i);
					return zero_id + i;
				}
			}
			throw std::runtime_error("How?");
		}
		void remove(size_t id) {
			auto local_id = id - zero_id;
			assert(local_id < bucketSize);
			occupancy.reset(local_id);
			reinterpret_cast<T*>(m_storage.data())[local_id].~T();
			//std::memset(bucket + local_id, 0, sizeof(T));
		}
		const T* get_ptr(size_t id) const {
			auto local_id = id - zero_id;
			assert(local_id < bucketSize);
			if (occupancy.test(local_id))
				return &reinterpret_cast<const T*>(m_storage.data())[local_id];
			else
				return nullptr;
		}
		T* get_ptr(size_t id) {
			auto local_id = id - zero_id;
			assert(local_id < bucketSize);
			if (occupancy.test(local_id))
				return &reinterpret_cast<T*>(m_storage.data())[local_id];
			else
				return nullptr;
		}
		const T& get(size_t id) const {
			auto local_id = id - zero_id;
			assert(local_id < bucketSize);
			if (occupancy.test(local_id))
				return reinterpret_cast<const T*>(m_storage.data())[local_id];
			else
				throw std::runtime_error("invalid id");
		}
		T& get(size_t id) {
			auto local_id = id - zero_id;
			assert(local_id < bucketSize);
			if (occupancy.test(local_id))
				return reinterpret_cast<T*>(m_storage.data())[local_id];
			else
				throw std::runtime_error("invalid id");
		}
		bool full() {
			return occupancy.all();
		}
		bool empty() {
			return occupancy.none();
		}
		ListBucket* get_next() {
			if (!next)
				next = std::make_unique<Bucket>(zero_id + bucketSize);
			return next.get();
		}
		ListBucket* get_next_non_filling() const {
			return next.get();
		}
		
		void print() {
			std::cout << "[";
			for (int i = 0; i < bucketSize; i++) {
				if (!occupancy[i])
					continue;
				std::cout << i + zero_id << ": " << reinterpret_cast<T*>(m_storage.data())[i] << " ";
			}
			std::cout << "]\n";
			if (next)
				next->print();
		}
	public:
		const size_t zero_id = 0;
	private:
		std::bitset<bucketSize> occupancy = 0;
		static_assert(sizeof(std::byte) == 1u);
		std::array<std::byte, sizeof(T)* bucketSize> m_storage;
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
		LinkedBucketList(LinkedBucketList&& other) :
			head(other.head.release())
		{

		}
		LinkedBucketList& operator=(const LinkedBucketList& other) = delete;
		LinkedBucketList& operator=(LinkedBucketList&& other) {
			head = other.head.release();
		}
		~LinkedBucketList() {

		}
		/// <summary>
		/// Clears the list without deallocating
		/// </summary>
		void clear() {
			Bucket* current = head.get();
			while (current) {
				current->clear();
				current = current->get_next_non_filling();
			}
		}
		void destroy() {
			head.reset(nullptr);
		}
		size_t insert(const T& t) {
			if (!head) {
				head = std::make_unique< Bucket>(0);
			}
			Bucket* current = head.get();
			while (current->full()) {
				current = current->get_next();
			}
			return current->insert(t);
		}
		size_t insert(T&& t) {
			if (!head) {
				head = std::make_unique< Bucket>(0);
			}
			Bucket* current = head.get();
			while (current->full()) {
				current = current->get_next();
			}
			return current->insert(std::move(t));
		}
		template<class... Args>
		[[nodiscard]] size_t emplace_intrusive(Args&&... args) {
			if (!head) {
				head = std::make_unique< Bucket>(0);
			}
			Bucket* current = head.get();
			while (current->full()) {
				current = current->get_next();
			}
			return current->emplace(std::forward<Args>(args)...);
		}
		template<class... Args>
		[[nodiscard]] ObjectHandle<T, LinkedBucketList<T>> emplace(Args&&... args) {
			return ObjectHandle<T, LinkedBucketList<T>>(emplace_intrusive(std::forward<Args>(args)...), this);
		}
		void remove(size_t id) {
			if (!head)
				return;
			Bucket* current = head.get();
			while (id >= (current->zero_id + bucketSize)) {
				current = current->get_next();
				if (current == nullptr)
					return;
			}
			current->remove(id);
		}
		const T* get_ptr(size_t id) const {
			if (!head)
				return nullptr;
			Bucket* current = head.get();
			while (id >= (current->zero_id + bucketSize)) {
				current = current->get_next();
				if (current == nullptr)
					return nullptr;
			}
			return current->get_ptr(id);
		}
		T* get_ptr(size_t id) {
			if (!head)
				return nullptr;
			Bucket* current = head.get();
			while (id >= (current->zero_id + bucketSize)) {
				current = current->get_next();
				if (current == nullptr)
					return nullptr;
			}
			return current->get_ptr(id);
		}
		const T& get(size_t id) const {
			if (!head)
				throw std::runtime_error("invalid id");
			Bucket* current = head.get();
			while (id >= (current->zero_id + bucketSize)) {
				current = current->get_next();
				if (current == nullptr)
					throw std::runtime_error("invalid id");
			}
			return current->get(id);
		}
		T& get(size_t id) {
			if (!head)
				throw std::runtime_error("invalid id");
			Bucket* current = head.get();
			while (id >= (current->zero_id + bucketSize)) {
				current = current->get_next();
				if (current == nullptr)
					throw std::runtime_error("invalid id");
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
#endif