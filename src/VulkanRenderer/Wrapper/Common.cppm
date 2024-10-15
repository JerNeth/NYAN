module;

//#include <array>
//#include <bit>
//#include <optional>

#include "magic_enum.hpp"

#include "volk.h"

export module NYANVulkan:Common;
import std;

import NYANData;

export namespace nyan::vulkan
{

	class Image;

	[[nodiscard]] constexpr uint32_t version_variant(uint32_t version) noexcept {
		return VK_API_VERSION_VARIANT(version);
	}
	[[nodiscard]] constexpr uint32_t version_major(uint32_t version) noexcept {
		return VK_API_VERSION_MAJOR(version);
	}
	[[nodiscard]] constexpr uint32_t version_minor(uint32_t version) noexcept {
		return VK_API_VERSION_MINOR(version);
	}
	[[nodiscard]] constexpr uint32_t version_patch(uint32_t version) noexcept {
		return VK_API_VERSION_PATCH(version);
	}
	struct Version {
		uint32_t variant : 3;
		uint32_t major : 7;
		uint32_t minor : 10;
		uint32_t patch : 12;
		constexpr Version(uint32_t value) noexcept :
			variant(version_variant(value)),
			major(version_major(value)),
			minor(version_minor(value)),
			patch(version_patch(value))
		{

		}

		[[nodiscard]] bool operator<(Version other) const noexcept {
			static_assert(sizeof(Version) == sizeof(uint32_t));
			return std::bit_cast<uint32_t>(*this) < std::bit_cast<uint32_t>(other);
		}

		[[nodiscard]] bool operator>(Version other) const noexcept {
			static_assert(sizeof(Version) == sizeof(uint32_t));
			return std::bit_cast<uint32_t>(*this) > std::bit_cast<uint32_t>(other);
		}
	};

	constexpr Version version11 = VK_API_VERSION_1_1;
	constexpr Version version12 = VK_API_VERSION_1_2;
	constexpr Version version13 = VK_API_VERSION_1_3;


	constexpr size_t maxOutputAttachments = 8; //In practice 8 is most common device limit (maxFragmentOutputAttachments)
	enum class LoadOp : uint8_t {
		Load,
		Clear,
		DontCare
	};
	enum class StoreOp : uint8_t {
		Store,
		DontCare
	};
	struct ClearValue {
		ClearValue() noexcept {};
		ClearValue(uint32_t x, uint32_t y = 0, uint32_t z = 0, uint32_t w = 0) noexcept : values({ x, y, z, w }) {};
		ClearValue(float x, float y = 0.f, float z = 0.f, float w = 0.f) noexcept :
			values({ std::bit_cast<uint32_t>(x), std::bit_cast<uint32_t>(y), std::bit_cast<uint32_t>(z), std::bit_cast<uint32_t>(w) }) {};
		ClearValue(int32_t x, int32_t y = 0, int32_t z = 0, int32_t w = 0) noexcept :
			values({std::bit_cast<uint32_t>(x), std::bit_cast<uint32_t>(y), std::bit_cast<uint32_t>(z), std::bit_cast<uint32_t>(w)}) {};
		std::array<uint32_t, 4> values{};

		operator VkClearValue() const noexcept {
			return VkClearValue{ .color {.uint32{values[0], values[1],values[2],values[3]}} };
		}
	};

	enum class Samples : uint8_t
	{
		C1 = VK_SAMPLE_COUNT_1_BIT,
		C2 = VK_SAMPLE_COUNT_2_BIT,
		C4 = VK_SAMPLE_COUNT_4_BIT,
		C8 = VK_SAMPLE_COUNT_8_BIT,
		C16 = VK_SAMPLE_COUNT_16_BIT,
		C32 = VK_SAMPLE_COUNT_32_BIT,
		C64 = VK_SAMPLE_COUNT_64_BIT,
	};
	enum class Layout : uint8_t
	{
		Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
		General = VK_IMAGE_LAYOUT_GENERAL,
		ColorAttachment = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		DepthStencilAttachment = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		DepthStencilReadOnly = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		ShaderReadOnly = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		TransferSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		TransferDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		Preinitialized = VK_IMAGE_LAYOUT_PREINITIALIZED,
		ReadOnly, //1_3 up
		Attachment, //1_3 up
		PresentSrc, // Swapchain
		VideoDecodeDst, //VK_KHR_video_decode_queue
		VideoDecodeSrc, //VK_KHR_video_decode_queue
		VideoDecodeDpb, //VK_KHR_video_decode_queue
		RenderingLocalRead, //VK_KHR_dynamic_rendering_local_read
		VideoEncodeDst, //VK_KHR_video_decode_queue
		VideoEncodeSrc, //VK_KHR_video_decode_queue
		VideoEncodeDpb, //VK_KHR_video_decode_queue
		AttachmentFeedbackLoop //VK_KHR_video_decode_queue
	};

	[[nodiscard]] constexpr uint32_t convert_layout(Layout layout) noexcept
	{
		switch (layout) {
		case Layout::Undefined:
		case Layout::General:
		case Layout::ColorAttachment:
		case Layout::DepthStencilAttachment:
		case Layout::DepthStencilReadOnly:
		case Layout::ShaderReadOnly:
		case Layout::TransferSrc:
		case Layout::TransferDst:
		case Layout::Preinitialized:
			return static_cast<uint32_t>(layout);
		case Layout::ReadOnly:  //1_3 up
			return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		case Layout::Attachment:  //1_3 up
			return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		case Layout::PresentSrc:  // Swapchain
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		case Layout::VideoDecodeDst:
			return VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR;
		case Layout::VideoDecodeSrc:
			return VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR;
		case Layout::VideoDecodeDpb:
			return VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR;
		case Layout::RenderingLocalRead:  //VK_KHR_dynamic_rendering_local_read
			return VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;
		case Layout::VideoEncodeDst:  //VK_KHR_video_decode_queue
			return VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR;
		case Layout::VideoEncodeSrc:  //VK_KHR_video_decode_queue
			return VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR;
		case Layout::VideoEncodeDpb:  //VK_KHR_video_decode_queue
			return VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR;
		case Layout::AttachmentFeedbackLoop: //VK_KHR_video_decode_queue
			return VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
		default:
			std::unreachable(); //UB, should never happen since the switch should cover all enums
		}
	}
	struct RenderingInfo {
		struct Attachment {
			enum class ResolveMode : uint8_t {
				None = VK_RESOLVE_MODE_NONE,
				SampleZero = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT,
				Average = VK_RESOLVE_MODE_AVERAGE_BIT,
				Min = VK_RESOLVE_MODE_MIN_BIT,
				Max = VK_RESOLVE_MODE_MAX_BIT
			};
			const Image& image;
			const Image* resolveImage{ nullptr };
			ResolveMode resolveMode{ ResolveMode::None };
			LoadOp loadOp{LoadOp::Load};
			StoreOp storeOp{StoreOp::Store };
			ClearValue clearValue{};
			Layout layout{ Layout::ColorAttachment };
		};
		StaticVector<Attachment, maxOutputAttachments> colorAttachments;	//8 is most common limit
		std::optional<Attachment> depthAttachment;
		std::optional<Attachment> stencilAttachment;

	};


	enum class PresentMode : uint8_t
	{
		Immediate,
		Mailbox,
		Fifo,
		FifoRelaxed,
		SharedDemandRefresh,
		SharedContinuousRefresh
	};

	using PresentModes = nyan::bitset<magic_enum::enum_count<PresentMode>(), PresentMode>;

	[[nodiscard]] constexpr uint32_t convert_present_mode(PresentMode presentMode) noexcept
	{
		switch (presentMode) {
			using enum PresentMode;
		case Immediate:
		case Mailbox:
		case Fifo:
		case FifoRelaxed:
			static_assert(VK_PRESENT_MODE_IMMEDIATE_KHR == static_cast<uint32_t>(PresentMode::Immediate));
			static_assert(VK_PRESENT_MODE_MAILBOX_KHR == static_cast<uint32_t>(PresentMode::Mailbox));
			static_assert(VK_PRESENT_MODE_FIFO_KHR == static_cast<uint32_t>(PresentMode::Fifo));
			static_assert(VK_PRESENT_MODE_FIFO_RELAXED_KHR == static_cast<uint32_t>(PresentMode::FifoRelaxed));
			return static_cast<uint32_t>(presentMode);
		case SharedDemandRefresh:
			return static_cast<uint32_t>(VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
		case SharedContinuousRefresh:
			return static_cast<uint32_t>(VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);
		default:
			std::unreachable(); //UB, should never happen since the switch should cover all enums
		}
	}
	[[nodiscard]] constexpr PresentMode convert_present_mode(uint32_t presentMode) noexcept
	{
		switch (presentMode) {
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
		case VK_PRESENT_MODE_MAILBOX_KHR:
		case VK_PRESENT_MODE_FIFO_KHR:
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			static_assert(VK_PRESENT_MODE_IMMEDIATE_KHR == static_cast<uint32_t>(PresentMode::Immediate));
			static_assert(VK_PRESENT_MODE_MAILBOX_KHR == static_cast<uint32_t>(PresentMode::Mailbox));
			static_assert(VK_PRESENT_MODE_FIFO_KHR == static_cast<uint32_t>(PresentMode::Fifo));
			static_assert(VK_PRESENT_MODE_FIFO_RELAXED_KHR == static_cast<uint32_t>(PresentMode::FifoRelaxed));
			return static_cast<PresentMode>(presentMode);
		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
			return PresentMode::SharedDemandRefresh;
		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
			return PresentMode::SharedContinuousRefresh;
		default:
			std::unreachable(); //UB, should never happen since the switch should cover all enums
		}
	}
	enum class ColorSpace : uint8_t
	{
		SRGBNonLinear,
		DisplayP3NonLinear,
		ExtendedSRGBLinear,
		DisplayP3Linear,
		DCIP3NonLinear,
		BT709Linear,
		BT709NonLinear,
		BT2020Linear,
		HDR10ST2084,
		Dolbyvision, //deprecated
		HDR10HLG,
		AdobeRGBLinear,
		AdobeRGBNonLinear,
		PassThrough,
		ExtendedSRGBNonLinear,
		DisplayNative,
	};

	[[nodiscard]] constexpr std::underlying_type_t<VkColorSpaceKHR > convert_color_space(ColorSpace colorSpace) noexcept
	{
		switch (colorSpace) {
			using enum ColorSpace;
		case SRGBNonLinear:
			return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		case DisplayP3NonLinear:
			return VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT;
		case ExtendedSRGBLinear:
			return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
		case DisplayP3Linear:
			return VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT;
		case DCIP3NonLinear:
			return VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT;
		case BT709Linear:
			return VK_COLOR_SPACE_BT709_LINEAR_EXT;
		case BT709NonLinear:
			return VK_COLOR_SPACE_BT709_NONLINEAR_EXT;
		case BT2020Linear:
			return VK_COLOR_SPACE_BT2020_LINEAR_EXT;
		case HDR10ST2084:
			return VK_COLOR_SPACE_HDR10_ST2084_EXT;
		case Dolbyvision: //deprecated
			return VK_COLOR_SPACE_DOLBYVISION_EXT;
		case HDR10HLG:
			return VK_COLOR_SPACE_HDR10_HLG_EXT;
		case AdobeRGBLinear:
			return VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT;
		case AdobeRGBNonLinear:
			return VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT;
		case PassThrough:
			return VK_COLOR_SPACE_PASS_THROUGH_EXT;
		case ExtendedSRGBNonLinear:
			return VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT;
		case DisplayNative:
			return VK_COLOR_SPACE_DISPLAY_NATIVE_AMD;
		default:
			std::unreachable(); //UB, should never happen since the switch should cover all enums
		}
	}

	[[nodiscard]] constexpr ColorSpace convert_color_space(std::underlying_type_t<VkColorSpaceKHR > colorSpace) noexcept
	{
		switch (colorSpace) {
			using enum ColorSpace;
		case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:			
			return SRGBNonLinear;
		case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
			return DisplayP3NonLinear;
		case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
			return ExtendedSRGBLinear;
		case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:
			return DisplayP3Linear;
		case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:
			return DCIP3NonLinear;
		case VK_COLOR_SPACE_BT709_LINEAR_EXT:
			return BT709Linear;
		case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
			return BT709NonLinear;
		case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
			return BT2020Linear;
		case VK_COLOR_SPACE_HDR10_ST2084_EXT:
			return HDR10ST2084;
		case VK_COLOR_SPACE_DOLBYVISION_EXT:
			return Dolbyvision;
		case VK_COLOR_SPACE_HDR10_HLG_EXT:
			return HDR10HLG;
		case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:
			return AdobeRGBLinear;
		case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:
			return AdobeRGBNonLinear;
		case VK_COLOR_SPACE_PASS_THROUGH_EXT:
			return PassThrough;
		case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
			return ExtendedSRGBNonLinear;
		case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:
			return DisplayNative;
		default:
			std::unreachable(); //UB, should never happen since the switch should cover all enums
		}
	}


	enum class ImageUsageFlags : uint8_t
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

	using ImageUsage = nyan::bitset<magic_enum::enum_count<ImageUsageFlags>(), ImageUsageFlags>;


	[[nodiscard]] constexpr std::underlying_type_t<VkImageUsageFlagBits> convert_image_usage_flags(ImageUsageFlags usage) noexcept
	{
		switch (usage) {
			using enum ImageUsageFlags;
		case TransferSrc:
			return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		case TransferDst:
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case Sampled:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case Storage:
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case ColorAttachment:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case DepthStencilAttachment:
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		case TransientAttachment:
			return VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		case InputAttachment:
			return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		case VideoDecodeDst:
			return VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
		case VideoDecodeSrc:
			return VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
		case VideoDecodeDPB:
			return VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
		case FragmentDensityMap:
			return VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
		case FragmentShadingRateAttachment:
			return VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
		case HostTransfer:
			return VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT;
		case VideoEncodeDst:
			return VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
		case VideoEncodeSrc:
			return VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
		case VideoEncodeDPB:
			return VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
		default:
			std::unreachable(); //UB, should never happen since the switch should cover all enums
		}
	}


	[[nodiscard]] constexpr auto convert_image_usage(ImageUsage usage) noexcept
	{
		decltype(VkImageUsageFlags{}) usageFlags{};
		usage.for_each([&usageFlags](ImageUsageFlags usage) {usageFlags |= convert_image_usage_flags(usage); });
		return usageFlags;
	}

	[[nodiscard]] constexpr ImageUsageFlags convert_image_usage_flags(std::underlying_type_t<VkImageUsageFlagBits> usage) noexcept
	{
		switch (usage) {
			using enum ImageUsageFlags;
		case VK_IMAGE_USAGE_TRANSFER_SRC_BIT:
			return TransferSrc;
		case VK_IMAGE_USAGE_TRANSFER_DST_BIT:
			return TransferDst;
		case VK_IMAGE_USAGE_SAMPLED_BIT:
			return Sampled;
		case VK_IMAGE_USAGE_STORAGE_BIT:
			return Storage;
		case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:
			return ColorAttachment;
		case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT:
			return DepthStencilAttachment;
		case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT:
			return TransientAttachment;
		case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT:
			return InputAttachment;
		case VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR:
			return VideoDecodeDst;
		case VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR:
			return VideoDecodeSrc;
		case VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR:
			return VideoDecodeDPB;
		case VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT:
			return FragmentDensityMap;
		case VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR:
			return FragmentShadingRateAttachment;
		case VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT:
			return HostTransfer;
		case VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR:
			return VideoEncodeDst;
		case VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR:
			return VideoEncodeSrc;
		case VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR:
			return VideoEncodeDPB;
		default:
			std::unreachable(); //UB, should never happen since the switch should cover all enums
		}
	}

	[[nodiscard]] constexpr auto convert_image_usage(VkImageUsageFlags usage) noexcept
	{
		ImageUsage usageFlags{};
		for (auto idx = std::countr_zero(usage); usage; usage &= (usage - 1)) { //turns of rightmost bit (Hacker's Delight)
			//idx = std::countr_zero(usage);
			usageFlags.set(convert_image_usage_flags(static_cast<VkImageUsageFlagBits>(usage & -usage)));
		}
		return usageFlags;
	}
}