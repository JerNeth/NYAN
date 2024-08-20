module;

#include <bit>
#include <expected>
#include <span>

#include "magic_enum.hpp"

#include "volk.h"

export module NYANVulkanWrapper:Image;
import NYANData;
import :Allocator;
import :DeletionQueue;
import :Error;
import :Object;
import :Queue;

export namespace nyan::vulkan::wrapper
{
	class Image : public Object<VkImage>
	{
	public:
		enum class UsageFlags : uint8_t
		{
			TransferSrc,
			TransferDst,
			Sampled,
			Storage,
			ColorAttachment,
			DepthStencilAttachment,
			TransientAttachment,
			InputAttachment,
			VideoDecodeDst,
			VideoDecodeSrc,
			VideoDecodeDPB,
			FragmentDensityMap,
			FragmentShadingRateAttachment,
			HostTransfer,
			VideoEncodeDst,
			VideoEncodeSrc,
			VideoEncodeDPB,
			//AttachmentFeedbackLoop = std::countr_zero(static_cast<uint32_t>(VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)),
			//InvocationMask = std::countr_zero(static_cast<uint32_t>(VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI)),
			//SampleWeight = std::countr_zero(static_cast<uint32_t>(VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM)),
			//SampleBlockMatch = std::countr_zero(static_cast<uint32_t>(VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM)),
		};

		using Usage = nyan::util::data::bitset<magic_enum::enum_count<UsageFlags>(), UsageFlags>;
		//using Usage = magic_enum::containers::bitset<UsageFlags>;

		struct Options {
			Queue::FamilyIndexGroup queueFamilies = {};
			bool dedicatedAllocation{ false };
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

	protected:
		struct Data
		{
			Usage usage;
			Object::Location location;
			Type type;
			Queue::FamilyIndexGroup queueFamilies;
			VmaAllocation allocation{ VK_NULL_HANDLE };
		};
		Image(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkImage handle, Data data) noexcept;

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
		[[nodiscard("must handle potential error")]] static std::expected<RenderTarget, Error> create(LogicalDevice& device) noexcept;
	private:
		RenderTarget() noexcept;
	};

	class SwapchainImage : public RenderTarget
	{
	public:
		[[nodiscard("must handle potential error")]] static std::expected<SwapchainImage, Error> create(LogicalDevice& device) noexcept;
	private:
		SwapchainImage() noexcept;
	};
}
