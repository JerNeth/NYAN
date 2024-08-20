module;

#include <bit>
#include <expected>
#include <span>

#include "magic_enum.hpp"

#include "volk.h"

export module NYANVulkanWrapper:Swapchain;
import :DeletionQueue;
import :Error;
import :Object;
import :Queue;

export namespace nyan::vulkan::wrapper
{
	//Create dependend on PhysicalDevice and SurfaceKHR
	//Present (multiple) swapchain(s) on a (present capable) queue
	class Swapchain : public Object<VkSwapchainKHR>
	{
	};
}