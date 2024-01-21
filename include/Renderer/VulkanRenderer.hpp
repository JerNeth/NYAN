#pragma once
#include <expected>

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