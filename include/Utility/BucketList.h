#pragma once
#ifndef BUCKETLIST_H
#define BUCKETLIST_H
#include <iostream>
#include <bitset>
#include <assert.h>
#include <array>
#include <memory>
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
		//size_t reserve() {
		//	size_t i;
		//	for (i = 0; i < bucketSize; i++) {
		//		if (!occupancy.test(i)) {
		//			occupancy.set(i);
		//			return zero_id + i;
		//		}
		//	}
		//	throw std::runtime_error("How?");
		//}
		//template<class... Args>
		//void emplace_reserved(size_t i, Args&&... args) {
		//	new(reinterpret_cast<T*>(m_storage.data()) + i) T(std::forward<Args>(args)...);
		//}
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
		size_t first() const noexcept {
			for (uint32_t i = 0; i < bucketSize; i++) {
				if (occupancy.test(i))
					return zero_id+ i;
			}
			return last();
		}
		size_t last() const noexcept {
			return zero_id + bucketSize;
		}
		bool contains(size_t id) const noexcept {
			auto local_id = id - zero_id;
			return occupancy.test(local_id);
		}
		template<typename Functor>
		void for_each(Functor functor) {
			for (uint32_t i = 0; i < bucketSize; i++) {
				if (occupancy.test(i))
					functor(get(zero_id + i));
			}
		}
	public:
		const size_t zero_id = 0;
	private:
		std::bitset<bucketSize> occupancy = 0;
		static_assert(sizeof(std::byte) == 1u);
		std::array<std::byte, sizeof(T)* bucketSize> m_storage{};
		std::unique_ptr<Bucket> next = nullptr;
	};
	/// <summary>
	/// Linked Bucked List
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T, size_t bucketSize>
	class LinkedBucketList {

		using Bucket = ListBucket<T, bucketSize>;
	public:
		//template<typename T, size_t bucketSize = 16>
		//struct Iterator {
		//	using iterator_category = std::forward_iterator_tag;
		//	using difference_type = size_t;
		//	using value_type = T;
		//	using pointer = T*;
		//	using reference = T&; 
		//	Iterator(size_t id, LinkedBucketList<T, bucketSize> parent) : m_id(id), r_parent(parent) {

		//	}
		//	size_t m_id;
		//	mutable LinkedBucketList<T, bucketSize> r_parent;
		//	reference operator*() const { return r_parent.get(m_id); }
		//	pointer operator->() { return r_parent.get_ptr(m_id); }

		//	// Prefix increment
		//	Iterator& operator++() { auto last = r_parent.last(); while (!r_parent.contains(++m_id)&& m_id != last); return *this; }

		//	// Postfix increment
		//	Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

		//	friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_id == b.m_id; };
		//	friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_id != b.m_id; };
		//};
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
		[[nodiscard]] size_t insert(const T& t) {
			if (!head) {
				head = std::make_unique< Bucket>(0);
			}
			Bucket* current = head.get();
			while (current->full()) {
				current = current->get_next();
			}
			return current->insert(t);
		}
		[[nodiscard]] size_t insert(T&& t) {
			if (!head) {
				head = std::make_unique< Bucket>(0);
			}
			Bucket* current = head.get();
			while (current->full()) {
				current = current->get_next();
			}
			return current->insert(std::move(t));
		}
		//[[nodiscard]] size_t reserve() {
		//	if (!head) {
		//		head = std::make_unique< Bucket>(0);
		//	}
		//	Bucket* current = head.get();
		//	while (current->full()) {
		//		current = current->get_next();
		//	}
		//	return current->emplace(std::forward<Args>(args)...);
		//}
		//template<class... Args>
		//[[nodiscard]] size_t emplace_reserved_intrusive(size_t id, Args&&... args) {
		//	if (!head) {
		//		head = std::make_unique< Bucket>(0);
		//	}
		//	Bucket* current = head.get();
		//	while (id >= (current->zero_id + bucketSize)) {
		//		current = current->get_next_non_filling();
		//		if (current == nullptr)
		//			return;
		//	}
		//	current->remove(id);
		//	return current->emplace(std::forward<Args>(args)...);
		//}
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
		[[nodiscard]] ObjectHandle<T, LinkedBucketList<T, bucketSize>> emplace(Args&&... args) {
			return ObjectHandle<T, LinkedBucketList<T, bucketSize>>(emplace_intrusive(std::forward<Args>(args)...), this);
		}
		void remove(size_t id) {
			if (!head)
				return;
			Bucket* current = head.get();
			while (id >= (current->zero_id + bucketSize)) {
				current = current->get_next_non_filling();
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
				current = current->get_next_non_filling();
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
				current = current->get_next_non_filling();
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
				current = current->get_next_non_filling();
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
				current = current->get_next_non_filling();
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
		bool contains(size_t id) const noexcept {
			if (!head)
				return false;
			Bucket* current = head.get();
			while (id >= (current->zero_id + bucketSize)) {
				current = current->get_next_non_filling();
				if (current == nullptr)
					return false;
			}
			return current->contains(id);
		}
		/*size_t first() const noexcept {
			if (!head)
				return 1;
			Bucket* current = head.get();
			while (current != nullptr) {
				if (auto val = current->first(); val != current->last())
					return val;
				current = current->get_next();
				if (current == nullptr)
					return current->zero_id + bucketSize;
			}
		}
		size_t last() const noexcept {
			if (!head)
				return 1;
			Bucket* current = head.get();
			while (current != nullptr) {
				Bucket* prev = current;
				current = current->get_next();
				if (current == nullptr)
					return prev->zero_id + bucketSize;
			}
		}*/
		/*Iterator<T, bucketSize> begin() {
			return Iterator<T, bucketSize>(first(), *this);
		}
		Iterator<T, bucketSize> end() {
			return Iterator<T, bucketSize>(last(), *this);
		}*/
		template<typename Functor>
		void for_each(Functor functor) {
			if (!head)
				return;
			Bucket* current = head.get();
			while (current != nullptr) {
				current->for_each(functor);
				current = current->get_next_non_filling();
			}
			return;
		}
	private:
		std::unique_ptr<Bucket> head = nullptr;
	};
	
}
#endif