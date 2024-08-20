module;

#include <bit>
#include <expected>
#include <span>

#include "magic_enum.hpp"

#include "volk.h"

export module NYANVulkanWrapper:Surface;
import :DeletionQueue;
import :Error;
import :Object;

export namespace nyan::vulkan::wrapper
{
	class Surface : public Object<VkSurfaceKHR>
	{
	};
}