#include "VulkanWrapper/Allocator.hpp"

#include "VulkanWrapper/Instance.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/PhysicalDevice.hpp"

namespace vulkan
{
	vulkan::Allocator::Allocator(Allocator&& other) noexcept :
		VulkanObject(std::move(other))
	{

	}

	vulkan::Allocator& vulkan::Allocator::operator=(Allocator&& other) noexcept
	{
		if(this != &other)
		{
			std::swap(m_handle, other.m_handle);
		}
		return *this;
	}

	vulkan::Allocator::~Allocator()
	{
		if (m_handle != VK_NULL_HANDLE)
			vmaDestroyAllocator(m_handle);

	}

	std::expected<vulkan::Allocator, vulkan::Error> vulkan::Allocator::create(const vulkan::Instance& instance, vulkan::LogicalDevice& device,
		VmaAllocatorCreateFlags createFlags) noexcept
	{
		::VmaAllocator allocator;
		if (auto result = device.get_device().vmaCreateAllocator(createFlags, device.get_physical_device(), device.get_allocator(),
			instance, device.get_physical_device().get_properties().apiVersion, &allocator); result != VK_SUCCESS) {
			return std::unexpected{vulkan::Error{result}};
		}
		return vulkan::Allocator{device, allocator};
	}

	vulkan::Allocator::Allocator(vulkan::LogicalDevice& device, ::VmaAllocator handle) noexcept :
		VulkanObject(device, handle)
	{
	}
}