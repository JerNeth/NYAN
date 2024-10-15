module;
//
//#include <atomic>
//#include <stdlib.h>

#include <GLFW/glfw3.h>


export module NYANGLFW:Allocator;
import std;
import NYANAssert;
import :ModuleSettings;


void* allocate(size_t size, void* user) noexcept;
void* reallocate(void* block, size_t size, void* user) noexcept;
void deallocate(void* block, void* user) noexcept;

export namespace nyan::glfw
{
	class Allocator
	{
	public:
		void* allocate(size_t size) noexcept;
		void* reallocate(void* block, size_t size) noexcept;
		void deallocate(void* block) noexcept;

		[[nodiscard]] uint64_t allocated() const noexcept;
		[[nodiscard]] uint64_t allocations() const noexcept;
		const GLFWallocator* get_allocator() const noexcept;
		Allocator() noexcept;
	private:

		GLFWallocator m_allocator{};
		std::atomic_uint64_t m_allocated{ 0 };
		std::atomic_uint64_t m_allocations { 0 };
	};
}

namespace nyan::glfw
{

	void* Allocator::allocate(size_t size) noexcept
	{
		m_allocated += size;
		m_allocations++;
		return std::malloc(size);
	}
	void* Allocator::reallocate(void* block, size_t size) noexcept
	{
		return std::realloc(block, size);
	}
	void Allocator::deallocate(void* block) noexcept
	{
		std::free(block);
	}
	uint64_t Allocator::allocated() const noexcept
	{
		return m_allocated;
	}
	uint64_t Allocator::allocations() const noexcept
	{
		return m_allocations;
	}
	const GLFWallocator* Allocator::get_allocator() const noexcept
	{
		return &m_allocator;
	}
	Allocator::Allocator() noexcept :
		m_allocator(GLFWallocator{
			.allocate{::allocate},
			.reallocate{::reallocate},
			.deallocate{::deallocate},
			.user{this}
			})
	{
	}
}

void* allocate(size_t size, void* user) noexcept
{
	return static_cast<nyan::glfw::Allocator*>(user)->allocate(size);
}

void* reallocate(void* block, size_t size, void* user) noexcept
{
	return static_cast<nyan::glfw::Allocator*>(user)->reallocate(block, size);
}

void deallocate(void* block, void* user) noexcept
{
	static_cast<nyan::glfw::Allocator*>(user)->deallocate(block);
}
