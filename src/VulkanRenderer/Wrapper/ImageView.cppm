module;

//#include <bit>
//#include <expected>
//#include <span>

#include "magic_enum/magic_enum.hpp"

#include "volk.h"

export module NYANVulkan:ImageView;
import std;
import NYANData;
import :Formats;
import :DeletionQueue;
import :Error;
import :Object;

export namespace nyan::vulkan
{
	class LogicalDevice;
	class Image;
	class ImageView
	{
	public:
		enum class AspectFlags : uint8_t{
			Color,
			Depth,
			Stencil,
		};
		using Aspect = nyan::bitset<magic_enum::enum_count<AspectFlags>(), AspectFlags>;
	public:

		ImageView(const ImageView&) noexcept = default;
		ImageView(ImageView&& other) noexcept = default;

		ImageView& operator=(const ImageView&) noexcept = default; //Currently not technically necessary since we manage the handle lifetime externally
		ImageView& operator=(ImageView&& other) noexcept = default;

		struct Params {
			VkImage image;
			Format format{ Format::UNDEFINED };
			VkImageViewType type;
			Aspect aspect{ AspectFlags::Color };
			uint32_t level = 0;
			uint32_t levelCount = VK_REMAINING_MIP_LEVELS;
			uint32_t layer = 0;
			uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS;
		};

		[[nodiscard("must handle potential error")]] static std::expected<ImageView, Error> create(LogicalDevice& device, Params params) noexcept;

		[[nodiscard]] VkImageView get_handle() const noexcept;
	private:
		ImageView(VkImageView handle) noexcept;

		VkImageView m_handle;
	};
}