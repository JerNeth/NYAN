#ifndef VKIMAGE_H
#define VKIMAGE_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
#include <Util>
#include "LinAlg.h"
#include <optional>

namespace vulkan {
	constexpr std::array<const char*,9> ImageLayoutNames{
		"VK_IMAGE_LAYOUT_UNDEFINED",
		"VK_IMAGE_LAYOUT_GENERAL",
		"VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL",
		"VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL",
		"VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL",
		"VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL",
		"VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL",
		"VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL",
		"VK_IMAGE_LAYOUT_PREINITIALIZED"
	};
	
	inline uint32_t format_block_size(VkFormat format) {
		switch (format) {
			case VK_FORMAT_R4G4_UNORM_PACK8	 :
			case VK_FORMAT_R8_UNORM			 :
			case VK_FORMAT_R8_SNORM			 :
			case VK_FORMAT_R8_USCALED		 :
			case VK_FORMAT_R8_SSCALED		 :
			case VK_FORMAT_R8_UINT			 :
			case VK_FORMAT_R8_SINT			 :
			case VK_FORMAT_R8_SRGB			 :
			case VK_FORMAT_S8_UINT:
				return 1;
			case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
			case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
			case VK_FORMAT_R5G6B5_UNORM_PACK16:
			case VK_FORMAT_B5G6R5_UNORM_PACK16:
			case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
			case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
			case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
			case VK_FORMAT_R8G8_UNORM:
			case VK_FORMAT_R8G8_SNORM:
			case VK_FORMAT_R8G8_USCALED:
			case VK_FORMAT_R8G8_SSCALED:
			case VK_FORMAT_R8G8_UINT:
			case VK_FORMAT_R8G8_SINT:
			case VK_FORMAT_R8G8_SRGB:
			case VK_FORMAT_R16_UNORM:
			case VK_FORMAT_R16_SNORM:
			case VK_FORMAT_R16_USCALED:
			case VK_FORMAT_R16_SSCALED:
			case VK_FORMAT_R16_UINT:
			case VK_FORMAT_R16_SINT:
			case VK_FORMAT_R16_SFLOAT:
			case VK_FORMAT_R10X6_UNORM_PACK16:
			case VK_FORMAT_R12X4_UNORM_PACK16:
			case VK_FORMAT_D16_UNORM:
				return 2;
			case VK_FORMAT_R8G8B8_UNORM	   :
			case VK_FORMAT_R8G8B8_SNORM	   :
			case VK_FORMAT_R8G8B8_USCALED  :
			case VK_FORMAT_R8G8B8_SSCALED  :
			case VK_FORMAT_R8G8B8_UINT	   :
			case VK_FORMAT_R8G8B8_SINT	   :
			case VK_FORMAT_R8G8B8_SRGB	   :
			case VK_FORMAT_B8G8R8_UNORM	   :
			case VK_FORMAT_B8G8R8_SNORM	   :
			case VK_FORMAT_B8G8R8_USCALED  :
			case VK_FORMAT_B8G8R8_SSCALED  :
			case VK_FORMAT_B8G8R8_UINT	   :
			case VK_FORMAT_B8G8R8_SINT	   :
			case VK_FORMAT_B8G8R8_SRGB	   :
			case VK_FORMAT_D16_UNORM_S8_UINT:
				return 3;							  
			case VK_FORMAT_R8G8B8A8_UNORM			  :
			case VK_FORMAT_R8G8B8A8_SNORM			  :
			case VK_FORMAT_R8G8B8A8_USCALED			  :
			case VK_FORMAT_R8G8B8A8_SSCALED			  :
			case VK_FORMAT_R8G8B8A8_UINT			  :
			case VK_FORMAT_R8G8B8A8_SINT			  :
			case VK_FORMAT_R8G8B8A8_SRGB			  :
			case VK_FORMAT_B8G8R8A8_UNORM			  :
			case VK_FORMAT_B8G8R8A8_SNORM			  :
			case VK_FORMAT_B8G8R8A8_USCALED			  :
			case VK_FORMAT_B8G8R8A8_SSCALED			  :
			case VK_FORMAT_B8G8R8A8_UINT			  :
			case VK_FORMAT_B8G8R8A8_SINT			  :
			case VK_FORMAT_B8G8R8A8_SRGB			  :
			case VK_FORMAT_A8B8G8R8_UNORM_PACK32	  :
			case VK_FORMAT_A8B8G8R8_SNORM_PACK32	  :
			case VK_FORMAT_A8B8G8R8_USCALED_PACK32	  :
			case VK_FORMAT_A8B8G8R8_SSCALED_PACK32	  :
			case VK_FORMAT_A8B8G8R8_UINT_PACK32		  :
			case VK_FORMAT_A8B8G8R8_SINT_PACK32		  :
			case VK_FORMAT_A8B8G8R8_SRGB_PACK32		  :
			case VK_FORMAT_A2R10G10B10_UNORM_PACK32	  :
			case VK_FORMAT_A2R10G10B10_SNORM_PACK32	  :
			case VK_FORMAT_A2R10G10B10_USCALED_PACK32 :
			case VK_FORMAT_A2R10G10B10_SSCALED_PACK32 :
			case VK_FORMAT_A2R10G10B10_UINT_PACK32	  :
			case VK_FORMAT_A2R10G10B10_SINT_PACK32	  :
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32	  :
			case VK_FORMAT_A2B10G10R10_SNORM_PACK32	  :
			case VK_FORMAT_A2B10G10R10_USCALED_PACK32 :
			case VK_FORMAT_A2B10G10R10_SSCALED_PACK32 :
			case VK_FORMAT_A2B10G10R10_UINT_PACK32	  :
			case VK_FORMAT_A2B10G10R10_SINT_PACK32	  :
			case VK_FORMAT_R16G16_UNORM				  :
			case VK_FORMAT_R16G16_SNORM				  :
			case VK_FORMAT_R16G16_USCALED			  :
			case VK_FORMAT_R16G16_SSCALED			  :
			case VK_FORMAT_R16G16_UINT				  :
			case VK_FORMAT_R16G16_SINT				  :
			case VK_FORMAT_R16G16_SFLOAT			  :
			case VK_FORMAT_R32_UINT					  :
			case VK_FORMAT_R32_SINT					  :
			case VK_FORMAT_R32_SFLOAT				  :
			case VK_FORMAT_B10G11R11_UFLOAT_PACK32	  :
			case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32	  :
			case VK_FORMAT_R10X6G10X6_UNORM_2PACK16	  :
			case VK_FORMAT_R12X4G12X4_UNORM_2PACK16	  :
			case VK_FORMAT_G8B8G8R8_422_UNORM		  :
			case VK_FORMAT_B8G8R8G8_422_UNORM		  :
			case VK_FORMAT_X8_D24_UNORM_PACK32:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
				return 4;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return 5;
			case VK_FORMAT_R16G16B16_UNORM	  :
			case VK_FORMAT_R16G16B16_SNORM	  :
			case VK_FORMAT_R16G16B16_USCALED  :
			case VK_FORMAT_R16G16B16_SSCALED  :
			case VK_FORMAT_R16G16B16_UINT	  :
			case VK_FORMAT_R16G16B16_SINT	  :
			case VK_FORMAT_R16G16B16_SFLOAT	  :
				return 6;
			case VK_FORMAT_R16G16B16A16_UNORM:
			case VK_FORMAT_R16G16B16A16_SNORM:
			case VK_FORMAT_R16G16B16A16_USCALED:
			case VK_FORMAT_R16G16B16A16_SSCALED:
			case VK_FORMAT_R16G16B16A16_UINT:
			case VK_FORMAT_R16G16B16A16_SINT:
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R32G32_UINT:
			case VK_FORMAT_R32G32_SINT:
			case VK_FORMAT_R32G32_SFLOAT:
			case VK_FORMAT_R64_UINT:
			case VK_FORMAT_R64_SINT:
			case VK_FORMAT_R64_SFLOAT:
			case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
			case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
			case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
			case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
			case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
			case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
			case VK_FORMAT_G16B16G16R16_422_UNORM:
			case VK_FORMAT_B16G16R16G16_422_UNORM:
				return 8;
			case VK_FORMAT_R32G32B32_UINT:
			case VK_FORMAT_R32G32B32_SFLOAT:
			case VK_FORMAT_R32G32B32_SINT:
				return 12;
			case VK_FORMAT_R32G32B32A32_UINT:
			case VK_FORMAT_R32G32B32A32_SINT:
			case VK_FORMAT_R32G32B32A32_SFLOAT:
			case VK_FORMAT_R64G64_UINT:
			case VK_FORMAT_R64G64_SINT:
			case VK_FORMAT_R64G64_SFLOAT:
				return 16;
			case VK_FORMAT_R64G64B64_UINT:
			case VK_FORMAT_R64G64B64_SINT:
			case VK_FORMAT_R64G64B64_SFLOAT:
				return 24;
			case VK_FORMAT_R64G64B64A64_UINT:
			case VK_FORMAT_R64G64B64A64_SINT:
			case VK_FORMAT_R64G64B64A64_SFLOAT:
				return 32;
			case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
			case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
			case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
			case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
			case VK_FORMAT_BC4_UNORM_BLOCK:
			case VK_FORMAT_BC4_SNORM_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
			case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
			case VK_FORMAT_EAC_R11_UNORM_BLOCK:
			case VK_FORMAT_EAC_R11_SNORM_BLOCK:
				return 8;
			case VK_FORMAT_BC2_UNORM_BLOCK:
			case VK_FORMAT_BC2_SRGB_BLOCK:
			case VK_FORMAT_BC3_UNORM_BLOCK:
			case VK_FORMAT_BC3_SRGB_BLOCK:
			case VK_FORMAT_BC5_UNORM_BLOCK:
			case VK_FORMAT_BC5_SNORM_BLOCK:
			case VK_FORMAT_BC6H_UFLOAT_BLOCK:
			case VK_FORMAT_BC6H_SFLOAT_BLOCK:
			case VK_FORMAT_BC7_UNORM_BLOCK:
			case VK_FORMAT_BC7_SRGB_BLOCK:
			case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
			case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
				return 16;
			default:
				assert(false);
				return 0;
		}
	}
	inline std::pair<uint32_t, uint32_t> format_to_block_size(VkFormat format) {
		switch (format) {
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
		case VK_FORMAT_BC4_UNORM_BLOCK:
		case VK_FORMAT_BC4_SNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
		case VK_FORMAT_EAC_R11_UNORM_BLOCK:
		case VK_FORMAT_EAC_R11_SNORM_BLOCK:
		case VK_FORMAT_BC2_UNORM_BLOCK:
		case VK_FORMAT_BC2_SRGB_BLOCK:
		case VK_FORMAT_BC3_UNORM_BLOCK:
		case VK_FORMAT_BC3_SRGB_BLOCK:
		case VK_FORMAT_BC5_UNORM_BLOCK:
		case VK_FORMAT_BC5_SNORM_BLOCK:
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:
		case VK_FORMAT_BC7_UNORM_BLOCK:
		case VK_FORMAT_BC7_SRGB_BLOCK:
		case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
			return { 4, 4 };
		default:
			return { 1, 1 };
		}
	}
	inline uint32_t calculate_mip_levels(uint32_t width, uint32_t height = 0, uint32_t depth = 0)
	{
		uint32_t size = Math::max(Math::max(width, height), depth);
		uint32_t mipLevels = 0;
		while (size)
		{
			mipLevels++;
			size >>= 1;
		}
		return mipLevels;
	}
	struct MipInfo {
		struct MipLevelInfo {
			uint32_t offset;
			uint32_t width;
			uint32_t height;
			uint32_t depth;
			uint32_t blockWidth; //Image encompassing width
			uint32_t blockHeight;//Image encompassing height
			uint32_t blockCountX; //Amount of Blocks in X
			uint32_t blockCountY; //Amount of Blocks in Y
		};
		std::array<MipLevelInfo, 16> mipLevels;
		uint32_t mipLevelCount;
		size_t size;
		MipInfo(VkFormat format, uint32_t width, uint32_t height, uint32_t layers, uint32_t mipLevelCount_, uint32_t depth = 1) {
			assert(!(depth != 1) || (layers == 1)); //logical implication
			assert(!(height == 1) || (depth == 1)); //logical implication
			assert(!(layers != 1) || (depth == 1)); //logical implication
			auto stride = format_block_size(format);
			auto [blockSizeX, blockSizeY] = format_to_block_size(format);
			mipLevelCount = mipLevelCount_;
			if (mipLevelCount == 0)
				mipLevelCount = calculate_mip_levels(width, height, depth);

			uint32_t offset = 0;
			for (uint32_t level = 0; level < mipLevelCount; level++) {
				offset = (offset + 15ull) & ~15ull;
				uint32_t width_ = Math::max(width >> level, 1u);
				uint32_t height_ = Math::max(height >> level, 1u);
				uint32_t depth_ = Math::max(depth >> level, 1u);

				uint32_t blockCountX = (width_ + blockSizeX - 1) / blockSizeX;
				uint32_t blockCountY = (height_ + blockSizeY - 1) / blockSizeY;

				mipLevels[level].offset = offset;
				mipLevels[level].width = width_;
				mipLevels[level].height = height_;
				mipLevels[level].depth = depth_;
				mipLevels[level].blockWidth = blockCountX * blockSizeX;
				mipLevels[level].blockHeight = blockCountY * blockSizeY;
				mipLevels[level].blockCountX = blockCountX;
				mipLevels[level].blockCountY = blockCountY;

				offset += blockCountX * blockCountY * stride * depth_ * layers;
			}
			size = offset;
		}
	};
	using AllocationHandle = Utility::ObjectHandle<Allocation, Utility::Pool<Allocation>>;
	using ImageHandle = Utility::ObjectHandle<Image, Utility::LinkedBucketList<Image>>;
	using ImageViewHandle = Utility::ObjectHandle<ImageView, Utility::LinkedBucketList<ImageView>>;
	struct ImageInfo {
		enum class Flags {
			GenerateMips,
			IsCube,
			ConcurrentGraphics,
			ConcurrentAsyncCompute,
			ConcurrentAsyncGraphics,
			ConcurrentAsyncTransfer,
			Size
		};
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		bool isSwapchainImage = false;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		VkImageUsageFlags usage = 0;
		VkImageType type = VK_IMAGE_TYPE_2D;
		VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkImageCreateFlags flags{};
		Utility::bitset<static_cast<size_t>(Flags::Size), Flags> createFlags;
		static bool is_depth_or_stencil_format(VkFormat format) {
			switch (format) {
			case(VK_FORMAT_D16_UNORM):
			case(VK_FORMAT_D16_UNORM_S8_UINT):
			case(VK_FORMAT_D24_UNORM_S8_UINT):
			case(VK_FORMAT_S8_UINT):
			case(VK_FORMAT_D32_SFLOAT):
			case(VK_FORMAT_D32_SFLOAT_S8_UINT):
			case VK_FORMAT_X8_D24_UNORM_PACK32:
				return true;

			default:
				return false;
			}
		}
		static ImageInfo immutable_2d_image(uint32_t width, uint32_t height, VkFormat format, bool mipmapped = false ) {
			ImageInfo info;
			info.width = width;
			info.height = height;
			info.depth = 1u;
			info.format = format;
			info.mipLevels = mipmapped? 0u : 1u;
			info.arrayLayers = 1u;
			info.type = VK_IMAGE_TYPE_2D;
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.flags = 0;
			if(mipmapped)
				info.createFlags.set(Flags::GenerateMips);
			info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			return info;
		}
		static ImageInfo render_target(uint32_t width, uint32_t height, VkFormat format, uint32_t arrayLayers = 1u) {
			ImageInfo info;
			info.width = width;
			info.height = height;
			info.depth = 1u;
			info.format = format;
			info.mipLevels = 1u;
			info.arrayLayers = arrayLayers;
			info.type = VK_IMAGE_TYPE_2D;
			//if (is_depth_or_stencil_format(format))
			//	info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			//else
			//	info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			//info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.layout = VK_IMAGE_LAYOUT_GENERAL;
			return info;
		}
		static inline VkImageAspectFlags format_to_aspect_mask(VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_UNDEFINED:
				return 0;

			case VK_FORMAT_S8_UINT:
				return VK_IMAGE_ASPECT_STENCIL_BIT;

			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;

			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_X8_D24_UNORM_PACK32:
				return VK_IMAGE_ASPECT_DEPTH_BIT;

			default:
				return VK_IMAGE_ASPECT_COLOR_BIT;
			}
		}
		static inline bool is_depth(VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_X8_D24_UNORM_PACK32:
				return true;
			default:
				return false;
			}
		}
		static inline bool is_stencil(VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_S8_UINT:
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return true;
			default:
				return false;
			}
		}

		VkImageViewType view_type() const noexcept {
			switch (type) {
			case VK_IMAGE_TYPE_1D:
				assert(width >= 1);
				assert(height == 1);
				assert(depth == 1);
				assert(samples == VK_SAMPLE_COUNT_1_BIT);
				if (arrayLayers > 1)
					return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
				else
					return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			case VK_IMAGE_TYPE_2D:
				assert(width >= 1);
				assert(height >= 1);
				assert(depth == 1);
				if ((flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (arrayLayers % 6) == 0) {
					assert(width == height);
					if (arrayLayers > 6)
						return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
					else
						return VK_IMAGE_VIEW_TYPE_CUBE;
				}
				else {
					if (arrayLayers > 1)
						return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
					else
						return VK_IMAGE_VIEW_TYPE_2D;
				}
			case VK_IMAGE_TYPE_3D:
				assert(width >= 1);
				assert(height >= 1);
				assert(depth >= 1);
				return VK_IMAGE_VIEW_TYPE_3D;
			default:
				assert(false);
				return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
			}
		}
		bool generate_mips() const {
			return createFlags.test(Flags::GenerateMips);
		}
		bool concurrent_queue() const {
			return createFlags.any_of(Flags::ConcurrentGraphics, Flags::ConcurrentAsyncCompute,
				Flags::ConcurrentAsyncGraphics, Flags::ConcurrentAsyncTransfer);
		}
	};
	
	
	struct ImageViewCreateInfo {
		Image* image = nullptr;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
		VkComponentMapping swizzle{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
		};
		VkImageAspectFlags aspectMask = 0;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 1;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;
	};
	class ImageView : public Utility::UIDC {
	public:
		ImageView(LogicalDevice& parent, const ImageViewCreateInfo & info);
		ImageView(ImageView& other) = delete;
		ImageView(ImageView&& other) noexcept;
		ImageView& operator=(ImageView&) = delete;
		ImageView& operator=(ImageView&&) = delete;
		~ImageView() noexcept;
		VkImageView get_image_view() const noexcept {
			return m_vkHandle;
		}
		VkFormat get_format() const noexcept {
			return m_info.format;
		}
		Image* get_image() const noexcept {
			return m_info.image;
		}
		uint32_t get_base_mip_level() const noexcept {
			return m_info.baseMipLevel;
		}
		void set_debug_label(const char* name) noexcept;
	private:
		LogicalDevice& r_device;
		VkImageView m_vkHandle;
		ImageViewCreateInfo m_info;
	};
	class Image : public Utility::UIDC {
	public:
		Image(LogicalDevice& parent, VkImage image, const ImageInfo& info, const std::vector< AllocationHandle>& allocations = {}, uint32_t mipTail = 0);
		Image(Image&) = delete;
		Image(Image&&) noexcept;
		Image& operator=(Image&) = delete;
		Image& operator=(Image&& other) noexcept;
		~Image() noexcept;
		void append_allocations(const std::vector< AllocationHandle>& allocations);
		void drop_allocations(uint32_t count);

		ImageView* get_view() noexcept;
		const ImageView* get_view() const noexcept;
		ImageView* get_stencil_view() noexcept;
		const ImageView* get_stencil_view() const noexcept;
		ImageView* get_depth_view() noexcept;
		const ImageView* get_depth_view() const noexcept;
		ImageView* change_view_mip_level(uint32_t mip);

		void set_debug_label(const char* name) noexcept;

		uint32_t get_available_mip() const noexcept;
		void set_available_mip(uint32_t mip) noexcept;
		bool is_sparse() const noexcept;
		uint32_t get_mip_tail() const noexcept;
		uint32_t get_width(uint32_t mipLevel = 0) const noexcept;
		uint32_t get_height(uint32_t mipLevel = 0) const noexcept;
		uint32_t get_depth(uint32_t mipLevel = 0) const noexcept;
		VkImage get_handle() const noexcept;
		VkImageUsageFlags get_usage() const noexcept;
		const ImageInfo& get_info() const;
		VkFormat get_format() const;
		void set_optimal(bool optimal = true) noexcept;
		bool is_optimal() const noexcept;
		void disown_image() noexcept;
		void set_layout(VkImageLayout format) noexcept;
		void disown() noexcept;
		VkPipelineStageFlags get_stage_flags() const noexcept;
		VkAccessFlags get_access_flags() const noexcept;
		void set_stage_flags(VkPipelineStageFlags flags)  noexcept;
		void set_access_flags(VkAccessFlags flags) noexcept;
		void set_single_mip_tail(bool mipTail) noexcept;
		bool is_being_resized() const noexcept;
		void set_being_resized(bool resized) noexcept;
		static inline VkPipelineStageFlags possible_stages_from_image_usage(VkImageUsageFlags usage)
		{
			VkPipelineStageFlags flags{};

			if (usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
				flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			if (usage & (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
				flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			if (usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
				flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			if (usage & (VK_IMAGE_USAGE_STORAGE_BIT)) {
				flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			}
			if (usage & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
				flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
				flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			}
			return flags;
		}

		static inline VkAccessFlags possible_access_from_image_layout(VkImageLayout layout) {
			switch (layout) {
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				return VK_ACCESS_TRANSFER_WRITE_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				return VK_ACCESS_TRANSFER_READ_BIT;

			default:
				return ~0u;
			}
		}
		static inline VkAccessFlags possible_access_from_image_usage(VkImageUsageFlags usage) {
			VkPipelineStageFlags flags{};

			if (usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
				flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			}
			if (usage & (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
				flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			}
			if (usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
				flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			}
			if (usage & (VK_IMAGE_USAGE_STORAGE_BIT)) {
				flags |= VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
			}
			if (usage & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
				flags |= VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
				flags |= VK_ACCESS_SHADER_READ_BIT;
			}
			return flags;
		}

	private:
		LogicalDevice& r_device;
		bool m_optimal = true;
		bool m_ownsImage = true;
		VkImage m_vkHandle = VK_NULL_HANDLE;
		VkPipelineStageFlags m_stageFlags{};
		VkAccessFlags m_accessFlags{};
		ImageInfo m_info;
		std::vector<AllocationHandle> m_allocations;
		ImageViewHandle m_view;
		std::optional<ImageViewHandle> m_stencilView;
		std::optional<ImageViewHandle> m_depthView;
		uint32_t m_availableMip = 0;
		uint32_t m_mipTail = 0;
		bool m_singleMipTail = false; //If false => first <Layer> allocations are mip tails, otherwise first allocation only
		bool m_isBeingResized = false;
	};
}

#endif //VKIMAGE_H