#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
namespace vulkan
{
	class Surface
	{
	public:
		explicit Surface(VkSurfaceKHR surface);
		operator VkSurfaceKHR() const noexcept;
		[[nodiscard]] const VkSurfaceKHR& get_handle() const noexcept;
	private:
		VkSurfaceKHR m_handle;
	};
}