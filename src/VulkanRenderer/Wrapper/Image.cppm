module;

//#include <bit>
//#include <expected>
//#include <span>

#include "magic_enum/magic_enum.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"

export module NYANVulkan:Image;
import std;
import NYANData;
import :Allocator;
import :Common;
import :DeletionQueue;
import :Error;
import :Formats;
import :ImageView;
import :Object;
import :Queue;

export namespace nyan::vulkan
{
	class LogicalDevice;
	class Image : public Object<VkImage>
	{
	public:


		struct Options 
		{
			Queue::FamilyIndex::Group queueFamilies {};
			ImageUsage usage;
		};
		enum class Type : uint8_t
		{
			Texture,
			Staging,
			RenderTarget,
			Swapchain
		};


	public:	
		Image(Image&) = delete;
		Image(Image&& other) noexcept;

		Image& operator=(Image&) = delete;
		Image& operator=(Image&& other) noexcept;

		~Image() noexcept;

		[[nodiscard]] uint16_t get_width() const noexcept;
		[[nodiscard]] uint16_t get_height() const noexcept;
		[[nodiscard]] Queue::FamilyIndex::Group get_queue_families() const noexcept;
		[[nodiscard]] uint16_t get_layers() const noexcept;
		[[nodiscard]] uint16_t get_mip_levels() const noexcept;
		[[nodiscard]] Samples get_samples() const noexcept;
		[[nodiscard]] Type get_type() const noexcept;
		[[nodiscard]] const ImageView& get_image_view(size_t idx) const noexcept;
		[[nodiscard]] Format get_format() const noexcept;
		[[nodiscard]] ImageUsage get_usage() const noexcept;

	protected:
		struct Data
		{
			ImageUsage usage;
			Object::Location location;
			Queue::FamilyIndex::Group queueFamilies;
			VmaAllocation allocation{ VK_NULL_HANDLE };
			uint16_t width, height;
			uint16_t layers, mipLevels;
			Samples samples;
			Type type;
			Format format;
			StaticVector<ImageView, 4> views{};
		};
		struct Parameters {
			const LogicalDeviceWrapper& device;
			DeletionQueue& deletionQueue;
			Allocator& allocator;
			VkImage handle;
			Data data;
		};
		Image(Parameters parameters) noexcept;

		DeletionQueue& r_deletionQueue;
		Allocator& r_allocator;
		Data m_data;
	private:

	};

	class Texture : public Image
	{
	public:
		[[nodiscard("must handle potential error")]] static std::expected<Texture, Error> create(LogicalDevice& device) noexcept;
	private:
		Texture() noexcept;
	};

	class StagingImage : public Image
	{
	public:
		[[nodiscard("must handle potential error")]] static std::expected<StagingImage, Error> create(LogicalDevice& device) noexcept;
	private:
		StagingImage() noexcept;
	};

	class RenderTarget : public Image
	{
	public:
		struct RenderTargetOptions {
			Options options{ };
			uint16_t width {1};
			uint16_t height {1};
			Samples samples {Samples::C1};
			RenderFormat format{ RenderFormat::UNDEFINED };
			bool dedicatedAllocation{ false };
		};
		[[nodiscard("must handle potential error")]] static std::expected<RenderTarget, Error> create(LogicalDevice& device, RenderTargetOptions options) noexcept;
	protected:
		RenderTarget(Parameters parameters) noexcept;
	};

	class SwapchainImage : public RenderTarget
	{
	public:
		struct SwapchainImageOptions {
			VkImage handle{ VK_NULL_HANDLE };
			Options options{ };
			uint16_t width{ 1 };
			uint16_t height{ 1 };
			Format format{ Format::UNDEFINED };
		};
		[[nodiscard("must handle potential error")]] static std::expected<SwapchainImage, Error> create(LogicalDevice& device, SwapchainImageOptions options) noexcept;
	private:
		SwapchainImage(Parameters parameters) noexcept;
	};
}
