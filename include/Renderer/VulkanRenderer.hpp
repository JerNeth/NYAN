#pragma once
#include <expected>

#include "GLFWWrapper/Library.hpp"
#include "GLFWWrapper/Monitor.hpp"
#include "GLFWWrapper/Window.hpp"

#include "Renderer/RendererError.hpp"

namespace renderer
{
	class VulkanRenderer
	{
	public:
		VulkanRenderer();
		static std::expected<VulkanRenderer, Error> create() noexcept;
	private:
	};
}