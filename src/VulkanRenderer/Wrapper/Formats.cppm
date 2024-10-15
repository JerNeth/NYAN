module;

//#include <utility>

#include "volk.h"

export module NYANVulkan:Formats;
import std;

export namespace nyan::vulkan
{
	enum class Format : uint8_t {
        UNDEFINED = VK_FORMAT_UNDEFINED,
        R4G4_UNORM_PACK8 = VK_FORMAT_R4G4_UNORM_PACK8,
        R4G4B4A4_UNORM_PACK16 = VK_FORMAT_R4G4B4A4_UNORM_PACK16,
        B4G4R4A4_UNORM_PACK16 = VK_FORMAT_B4G4R4A4_UNORM_PACK16,
        R5G6B5_UNORM_PACK16 = VK_FORMAT_R5G6B5_UNORM_PACK16, 
        B5G6R5_UNORM_PACK16 = VK_FORMAT_B5G6R5_UNORM_PACK16, 
        R5G5B5A1_UNORM_PACK16 = VK_FORMAT_R5G5B5A1_UNORM_PACK16,
        B5G5R5A1_UNORM_PACK16 = VK_FORMAT_B5G5R5A1_UNORM_PACK16,
        A1R5G5B5_UNORM_PACK16 = VK_FORMAT_A1R5G5B5_UNORM_PACK16, 
        R8_UNORM = VK_FORMAT_R8_UNORM,
        R8_SNORM = VK_FORMAT_R8_SNORM,
        R8_USCALED = VK_FORMAT_R8_USCALED,
        R8_SSCALED = VK_FORMAT_R8_SSCALED,
        R8_UINT = VK_FORMAT_R8_UINT,
        R8_SINT = VK_FORMAT_R8_SINT,
        R8_SRGB = VK_FORMAT_R8_SRGB,
        R8G8_UNORM = VK_FORMAT_R8G8_UNORM,
        R8G8_SNORM = VK_FORMAT_R8G8_SNORM,
        R8G8_USCALED = VK_FORMAT_R8G8_USCALED,
        R8G8_SSCALED = VK_FORMAT_R8G8_SSCALED,
        R8G8_UINT = VK_FORMAT_R8G8_UINT,
        R8G8_SINT = VK_FORMAT_R8G8_SINT,
        R8G8_SRGB = VK_FORMAT_R8G8_SRGB,
        R8G8B8_UNORM = VK_FORMAT_R8G8B8_UNORM,
        R8G8B8_SNORM = VK_FORMAT_R8G8B8_SNORM,
        R8G8B8_USCALED = VK_FORMAT_R8G8B8_USCALED,
        R8G8B8_SSCALED = VK_FORMAT_R8G8B8_SSCALED,
        R8G8B8_UINT = VK_FORMAT_R8G8B8_UINT,
        R8G8B8_SINT = VK_FORMAT_R8G8B8_SINT,
        R8G8B8_SRGB = VK_FORMAT_R8G8B8_SRGB,
        B8G8R8_UNORM = VK_FORMAT_B8G8R8_UNORM,
        B8G8R8_SNORM = VK_FORMAT_B8G8R8_SNORM,
        B8G8R8_USCALED = VK_FORMAT_B8G8R8_USCALED,
        B8G8R8_SSCALED = VK_FORMAT_B8G8R8_SSCALED,
        B8G8R8_UINT = VK_FORMAT_B8G8R8_UINT,
        B8G8R8_SINT = VK_FORMAT_B8G8R8_SINT,
        B8G8R8_SRGB = VK_FORMAT_B8G8R8_SRGB,
        R8G8B8A8_UNORM = VK_FORMAT_R8G8B8A8_UNORM,
        R8G8B8A8_SNORM = VK_FORMAT_R8G8B8A8_SNORM,
        R8G8B8A8_USCALED = VK_FORMAT_R8G8B8A8_USCALED,
        R8G8B8A8_SSCALED = VK_FORMAT_R8G8B8A8_SSCALED,
        R8G8B8A8_UINT = VK_FORMAT_R8G8B8A8_UINT,
        R8G8B8A8_SINT = VK_FORMAT_R8G8B8A8_SINT,
        R8G8B8A8_SRGB = VK_FORMAT_R8G8B8A8_SRGB,
        B8G8R8A8_UNORM = VK_FORMAT_B8G8R8A8_UNORM,
        B8G8R8A8_SNORM = VK_FORMAT_B8G8R8A8_SNORM,
        B8G8R8A8_USCALED = VK_FORMAT_B8G8R8A8_USCALED,
        B8G8R8A8_SSCALED = VK_FORMAT_B8G8R8A8_SSCALED,
        B8G8R8A8_UINT = VK_FORMAT_B8G8R8A8_UINT,
        B8G8R8A8_SINT = VK_FORMAT_B8G8R8A8_SINT,
        B8G8R8A8_SRGB = VK_FORMAT_B8G8R8A8_SRGB,
        A8B8G8R8_UNORM_PACK32 = VK_FORMAT_A8B8G8R8_UNORM_PACK32,
        A8B8G8R8_SNORM_PACK32 = VK_FORMAT_A8B8G8R8_SNORM_PACK32,
        A8B8G8R8_USCALED_PACK32 = VK_FORMAT_A8B8G8R8_USCALED_PACK32,
        A8B8G8R8_SSCALED_PACK32 = VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
        A8B8G8R8_UINT_PACK32 = VK_FORMAT_A8B8G8R8_UINT_PACK32,
        A8B8G8R8_SINT_PACK32 = VK_FORMAT_A8B8G8R8_SINT_PACK32,
        A8B8G8R8_SRGB_PACK32 = VK_FORMAT_A8B8G8R8_SRGB_PACK32,
        A2R10G10B10_UNORM_PACK32 = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        A2R10G10B10_SNORM_PACK32 = VK_FORMAT_A2R10G10B10_SNORM_PACK32,
        A2R10G10B10_USCALED_PACK32 = VK_FORMAT_A2R10G10B10_USCALED_PACK32,
        A2R10G10B10_SSCALED_PACK32 = VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
        A2R10G10B10_UINT_PACK32 = VK_FORMAT_A2R10G10B10_UINT_PACK32,
        A2R10G10B10_SINT_PACK32 = VK_FORMAT_A2R10G10B10_SINT_PACK32,
        A2B10G10R10_UNORM_PACK32 = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        A2B10G10R10_SNORM_PACK32 = VK_FORMAT_A2B10G10R10_SNORM_PACK32,
        A2B10G10R10_USCALED_PACK32 = VK_FORMAT_A2B10G10R10_USCALED_PACK32,
        A2B10G10R10_SSCALED_PACK32 = VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
        A2B10G10R10_UINT_PACK32 = VK_FORMAT_A2B10G10R10_UINT_PACK32,
        A2B10G10R10_SINT_PACK32 = VK_FORMAT_A2B10G10R10_SINT_PACK32,
        R16_UNORM = VK_FORMAT_R16_UNORM,
        R16_SNORM = VK_FORMAT_R16_SNORM,
        R16_USCALED = VK_FORMAT_R16_USCALED,
        R16_SSCALED = VK_FORMAT_R16_SSCALED,
        R16_UINT = VK_FORMAT_R16_UINT,
        R16_SINT = VK_FORMAT_R16_SINT,
        R16_SFLOAT = VK_FORMAT_R16_SFLOAT,
        R16G16_UNORM = VK_FORMAT_R16G16_UNORM,
        R16G16_SNORM = VK_FORMAT_R16G16_SNORM,
        R16G16_USCALED = VK_FORMAT_R16G16_USCALED,
        R16G16_SSCALED = VK_FORMAT_R16G16_SSCALED,
        R16G16_UINT = VK_FORMAT_R16G16_UINT,
        R16G16_SINT = VK_FORMAT_R16G16_SINT,
        R16G16_SFLOAT = VK_FORMAT_R16G16_SFLOAT,
        R16G16B16_UNORM = VK_FORMAT_R16G16B16_UNORM,
        R16G16B16_SNORM = VK_FORMAT_R16G16B16_SNORM,
        R16G16B16_USCALED = VK_FORMAT_R16G16B16_USCALED,
        R16G16B16_SSCALED = VK_FORMAT_R16G16B16_SSCALED,
        R16G16B16_UINT = VK_FORMAT_R16G16B16_UINT,
        R16G16B16_SINT = VK_FORMAT_R16G16B16_SINT,
        R16G16B16_SFLOAT = VK_FORMAT_R16G16B16_SFLOAT,
        R16G16B16A16_UNORM = VK_FORMAT_R16G16B16A16_UNORM,
        R16G16B16A16_SNORM = VK_FORMAT_R16G16B16A16_SNORM,
        R16G16B16A16_USCALED = VK_FORMAT_R16G16B16A16_USCALED,
        R16G16B16A16_SSCALED = VK_FORMAT_R16G16B16A16_SSCALED,
        R16G16B16A16_UINT = VK_FORMAT_R16G16B16A16_UINT,
        R16G16B16A16_SINT = VK_FORMAT_R16G16B16A16_SINT,
        R16G16B16A16_SFLOAT = VK_FORMAT_R16G16B16A16_SFLOAT,
        R32_UINT = VK_FORMAT_R32_UINT,
        R32_SINT = VK_FORMAT_R32_SINT,
        R32_SFLOAT = VK_FORMAT_R32_SFLOAT,
        R32G32_UINT = VK_FORMAT_R32G32_UINT,
        R32G32_SINT = VK_FORMAT_R32G32_SINT,
        R32G32_SFLOAT = VK_FORMAT_R32G32_SFLOAT,
        R32G32B32_UINT = VK_FORMAT_R32G32B32_UINT,
        R32G32B32_SINT = VK_FORMAT_R32G32B32_SINT,
        R32G32B32_SFLOAT = VK_FORMAT_R32G32B32_SFLOAT,
        R32G32B32A32_UINT = VK_FORMAT_R32G32B32A32_UINT,
        R32G32B32A32_SINT = VK_FORMAT_R32G32B32A32_SINT,
        R32G32B32A32_SFLOAT = VK_FORMAT_R32G32B32A32_SFLOAT,
        R64_UINT = VK_FORMAT_R64_UINT,
        R64_SINT = VK_FORMAT_R64_SINT,
        R64_SFLOAT = VK_FORMAT_R64_SFLOAT,
        R64G64_UINT = VK_FORMAT_R64G64_UINT,
        R64G64_SINT = VK_FORMAT_R64G64_SINT,
        R64G64_SFLOAT = VK_FORMAT_R64G64_SFLOAT,
        R64G64B64_UINT = VK_FORMAT_R64G64B64_UINT,
        R64G64B64_SINT = VK_FORMAT_R64G64B64_SINT,
        R64G64B64_SFLOAT = VK_FORMAT_R64G64B64_SFLOAT,
        R64G64B64A64_UINT = VK_FORMAT_R64G64B64A64_UINT,
        R64G64B64A64_SINT = VK_FORMAT_R64G64B64A64_SINT,
        R64G64B64A64_SFLOAT = VK_FORMAT_R64G64B64A64_SFLOAT,
        B10G11R11_UFLOAT_PACK32 = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
        E5B9G9R9_UFLOAT_PACK32 = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
        D16_UNORM = VK_FORMAT_D16_UNORM,
        X8_D24_UNORM_PACK32 = VK_FORMAT_X8_D24_UNORM_PACK32,
        D32_SFLOAT = VK_FORMAT_D32_SFLOAT,
        S8_UINT = VK_FORMAT_S8_UINT,
        D16_UNORM_S8_UINT = VK_FORMAT_D16_UNORM_S8_UINT,
        D24_UNORM_S8_UINT = VK_FORMAT_D24_UNORM_S8_UINT,
        D32_SFLOAT_S8_UINT = VK_FORMAT_D32_SFLOAT_S8_UINT,
        BC1_RGB_UNORM_BLOCK = VK_FORMAT_BC1_RGB_UNORM_BLOCK,
        BC1_RGB_SRGB_BLOCK = VK_FORMAT_BC1_RGB_SRGB_BLOCK,
        BC1_RGBA_UNORM_BLOCK = VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
        BC1_RGBA_SRGB_BLOCK = VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
        BC2_UNORM_BLOCK = VK_FORMAT_BC2_UNORM_BLOCK,
        BC2_SRGB_BLOCK = VK_FORMAT_BC2_SRGB_BLOCK,
        BC3_UNORM_BLOCK = VK_FORMAT_BC3_UNORM_BLOCK,
        BC3_SRGB_BLOCK = VK_FORMAT_BC3_SRGB_BLOCK,
        BC4_UNORM_BLOCK = VK_FORMAT_BC4_UNORM_BLOCK,
        BC4_SNORM_BLOCK = VK_FORMAT_BC4_SNORM_BLOCK,
        BC5_UNORM_BLOCK = VK_FORMAT_BC5_UNORM_BLOCK,
        BC5_SNORM_BLOCK = VK_FORMAT_BC5_SNORM_BLOCK,
        BC6H_UFLOAT_BLOCK = VK_FORMAT_BC6H_UFLOAT_BLOCK,
        BC6H_SFLOAT_BLOCK = VK_FORMAT_BC6H_SFLOAT_BLOCK,
        BC7_UNORM_BLOCK = VK_FORMAT_BC7_UNORM_BLOCK,
        BC7_SRGB_BLOCK = VK_FORMAT_BC7_SRGB_BLOCK,
        ETC2_R8G8B8_UNORM_BLOCK = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
        ETC2_R8G8B8_SRGB_BLOCK = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
        ETC2_R8G8B8A1_UNORM_BLOCK = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
        ETC2_R8G8B8A1_SRGB_BLOCK = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
        ETC2_R8G8B8A8_UNORM_BLOCK = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
        ETC2_R8G8B8A8_SRGB_BLOCK = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
        EAC_R11_UNORM_BLOCK = VK_FORMAT_EAC_R11_UNORM_BLOCK,
        EAC_R11_SNORM_BLOCK = VK_FORMAT_EAC_R11_SNORM_BLOCK,
        EAC_R11G11_UNORM_BLOCK = VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
        EAC_R11G11_SNORM_BLOCK = VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
        ASTC_4x4_UNORM_BLOCK = VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
        ASTC_4x4_SRGB_BLOCK = VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
        ASTC_5x4_UNORM_BLOCK = VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
        ASTC_5x4_SRGB_BLOCK = VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
        ASTC_5x5_UNORM_BLOCK = VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
        ASTC_5x5_SRGB_BLOCK = VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
        ASTC_6x5_UNORM_BLOCK = VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
        ASTC_6x5_SRGB_BLOCK = VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
        ASTC_6x6_UNORM_BLOCK = VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
        ASTC_6x6_SRGB_BLOCK = VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
        ASTC_8x5_UNORM_BLOCK = VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
        ASTC_8x5_SRGB_BLOCK = VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
        ASTC_8x6_UNORM_BLOCK = VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
        ASTC_8x6_SRGB_BLOCK = VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
        ASTC_8x8_UNORM_BLOCK = VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
        ASTC_8x8_SRGB_BLOCK = VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
        ASTC_10x5_UNORM_BLOCK = VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
        ASTC_10x5_SRGB_BLOCK = VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
        ASTC_10x6_UNORM_BLOCK = VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
        ASTC_10x6_SRGB_BLOCK = VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
        ASTC_10x8_UNORM_BLOCK = VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
        ASTC_10x8_SRGB_BLOCK = VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
        ASTC_10x10_UNORM_BLOCK = VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
        ASTC_10x10_SRGB_BLOCK = VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
        ASTC_12x10_UNORM_BLOCK = VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
        ASTC_12x10_SRGB_BLOCK = VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
        ASTC_12x12_UNORM_BLOCK = VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
        ASTC_12x12_SRGB_BLOCK = VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
	};

    //Limited subsets with most prominent hardware support (on windows)

    //TODO: Decide if non-blendable formats are supposed to be in this enum
    enum class RenderFormat : uint8_t {
        UNDEFINED = std::to_underlying(Format::UNDEFINED),
        R5G6B5_UNORM_PACK16 = std::to_underlying(Format::R5G6B5_UNORM_PACK16),  //Not (widely) supported as Storage Image
        A1R5G5B5_UNORM_PACK16 = std::to_underlying(Format::A1R5G5B5_UNORM_PACK16), //Not (widely) supported as Storage Image
        R8_UNORM = std::to_underlying(Format::R8_UNORM),
        R8_SNORM = std::to_underlying(Format::R8_SNORM),
        R8_UINT = std::to_underlying(Format::R8_UINT),
        R8_SINT = std::to_underlying(Format::R8_SINT),
        R8G8_UNORM = std::to_underlying(Format::R8G8_UNORM),
        R8G8_SNORM = std::to_underlying(Format::R8G8_SNORM),
        R8G8_UINT = std::to_underlying(Format::R8G8_UINT),
        R8G8_SINT = std::to_underlying(Format::R8G8_SINT),
        R8G8B8A8_UNORM = std::to_underlying(Format::R8G8B8A8_UNORM),
        R8G8B8A8_SNORM = std::to_underlying(Format::R8G8B8A8_SNORM),
        R8G8B8A8_UINT = std::to_underlying(Format::R8G8B8A8_UINT),
        R8G8B8A8_SINT = std::to_underlying(Format::R8G8B8A8_SINT),
        R8G8B8A8_SRGB = std::to_underlying(Format::R8G8B8A8_SRGB), //Not supported as Storage Image
        B8G8R8A8_UNORM = std::to_underlying(Format::B8G8R8A8_UNORM),
        B8G8R8A8_SRGB = std::to_underlying(Format::B8G8R8A8_SRGB), //Not supported as Storage Image
        A8B8G8R8_UNORM_PACK32 = std::to_underlying(Format::A8B8G8R8_UNORM_PACK32),
        A8B8G8R8_SNORM_PACK32 = std::to_underlying(Format::A8B8G8R8_SNORM_PACK32),
        A8B8G8R8_UINT_PACK32 = std::to_underlying(Format::A8B8G8R8_UINT_PACK32),
        A8B8G8R8_SINT_PACK32 = std::to_underlying(Format::A8B8G8R8_SINT_PACK32),
        A8B8G8R8_SRGB_PACK32 = std::to_underlying(Format::A8B8G8R8_SRGB_PACK32), //Not supported as Storage Image
        A2B10G10R10_UINT_PACK32 = std::to_underlying(Format::A2B10G10R10_UINT_PACK32),
        A2B10G10R10_UNORM_PACK32 = std::to_underlying(Format::A2B10G10R10_UNORM_PACK32),  //Not (widely) supported as Storage Image
        R16_UNORM = std::to_underlying(Format::R16_UNORM),
        R16_SNORM = std::to_underlying(Format::R16_SNORM),
        R16_UINT = std::to_underlying(Format::R16_UINT),
        R16_SINT = std::to_underlying(Format::R16_SINT),
        R16_SFLOAT = std::to_underlying(Format::R16_SFLOAT),
        R16G16_UNORM = std::to_underlying(Format::R16G16_UNORM),
        R16G16_SNORM = std::to_underlying(Format::R16G16_SNORM),
        R16G16_UINT = std::to_underlying(Format::R16G16_UINT),
        R16G16_SINT = std::to_underlying(Format::R16G16_SINT),
        R16G16_SFLOAT = std::to_underlying(Format::R16G16_SFLOAT),
        R16G16B16A16_UNORM = std::to_underlying(Format::R16G16B16A16_UNORM),
        R16G16B16A16_SNORM = std::to_underlying(Format::R16G16B16A16_SNORM),
        R16G16B16A16_UINT = std::to_underlying(Format::R16G16B16A16_UINT),
        R16G16B16A16_SINT = std::to_underlying(Format::R16G16B16A16_SINT),
        R16G16B16A16_SFLOAT = std::to_underlying(Format::R16G16B16A16_SFLOAT),
        R32_UINT = std::to_underlying(Format::R32_UINT),
        R32_SINT = std::to_underlying(Format::R32_SINT),
        R32_SFLOAT = std::to_underlying(Format::R32_SFLOAT),
        R32G32_UINT = std::to_underlying(Format::R32G32_UINT),
        R32G32_SINT = std::to_underlying(Format::R32G32_SINT),
        R32G32_SFLOAT = std::to_underlying(Format::R32G32_SFLOAT),
        R32G32B32A32_UINT = std::to_underlying(Format::R32G32B32A32_UINT),
        R32G32B32A32_SINT = std::to_underlying(Format::R32G32B32A32_SINT),
        R32G32B32A32_SFLOAT = std::to_underlying(Format::R32G32B32A32_SFLOAT),
        B10G11R11_UFLOAT_PACK32 = std::to_underlying(Format::B10G11R11_UFLOAT_PACK32),
    };

    enum class DepthStencilFormat : uint8_t {
        UNDEFINED = std::to_underlying(Format::UNDEFINED),
        D16_UNORM = std::to_underlying(Format::D16_UNORM),
        D32_SFLOAT = std::to_underlying(Format::D32_SFLOAT),
        D32_SFLOAT_S8_UINT = std::to_underlying(Format::D32_SFLOAT_S8_UINT),
        //D24_UNORM_S8_UINT, //not supported on AMD
        //S8_UINT, //not supported on Intel
    };
    [[nodiscard]] constexpr bool is_depth_stencil(Format format) noexcept {
        return format == Format::D16_UNORM ||
            format == Format::D32_SFLOAT ||
            format == Format::D32_SFLOAT_S8_UINT;
    }
    //(Here, mainly due to limited Accel structure support for other formats, but the use of most other formats in this context eludes me)
    enum class VertexFormat : uint8_t {
        UNDEFINED = std::to_underlying(Format::UNDEFINED),
        R16G16_SNORM = std::to_underlying(Format::R16G16_SNORM),
        R16G16_SFLOAT = std::to_underlying(Format::R16G16_SFLOAT),
        R16G16B16A16_SNORM = std::to_underlying(Format::R16G16B16A16_SNORM),
        R16G16B16A16_SFLOAT = std::to_underlying(Format::R16G16B16A16_SFLOAT),
        R32G32_SFLOAT = std::to_underlying(Format::R32G32_SFLOAT),
        R32G32B32_SFLOAT = std::to_underlying(Format::R32G32B32_SFLOAT),
    };
    [[nodiscard]] constexpr size_t format_byte_size(Format format) noexcept {
        switch(format) {
            using enum Format;
            case UNDEFINED:
                return 0;
            case R4G4_UNORM_PACK8:
                return 1;
            case R4G4B4A4_UNORM_PACK16:
            case B4G4R4A4_UNORM_PACK16:
            case R5G6B5_UNORM_PACK16:
            case B5G6R5_UNORM_PACK16:
            case R5G5B5A1_UNORM_PACK16:
            case B5G5R5A1_UNORM_PACK16:
            case A1R5G5B5_UNORM_PACK16:
                return 2;
            case R8_UNORM:
            case R8_SNORM:
            case R8_USCALED:
            case R8_SSCALED:
            case R8_UINT:
            case R8_SINT:
            case R8_SRGB:
                return 1;
            case R8G8_UNORM:
            case R8G8_SNORM:
            case R8G8_USCALED:
            case R8G8_SSCALED:
            case R8G8_UINT:
            case R8G8_SINT:
            case R8G8_SRGB:
                return 2;
            case R8G8B8_UNORM:
            case R8G8B8_SNORM:
            case R8G8B8_USCALED:
            case R8G8B8_SSCALED:
            case R8G8B8_UINT:
            case R8G8B8_SINT:
            case R8G8B8_SRGB:
            case B8G8R8_UNORM:
            case B8G8R8_SNORM:
            case B8G8R8_USCALED:
            case B8G8R8_SSCALED:
            case B8G8R8_UINT:
            case B8G8R8_SINT:
            case B8G8R8_SRGB:
                return 3;
            case R8G8B8A8_UNORM:
            case R8G8B8A8_SNORM:
            case R8G8B8A8_USCALED:
            case R8G8B8A8_SSCALED:
            case R8G8B8A8_UINT:
            case R8G8B8A8_SINT:
            case R8G8B8A8_SRGB:
            case B8G8R8A8_UNORM:
            case B8G8R8A8_SNORM:
            case B8G8R8A8_USCALED:
            case B8G8R8A8_SSCALED:
            case B8G8R8A8_UINT:
            case B8G8R8A8_SINT:
            case B8G8R8A8_SRGB:
            case A8B8G8R8_UNORM_PACK32:
            case A8B8G8R8_SNORM_PACK32:
            case A8B8G8R8_USCALED_PACK32:
            case A8B8G8R8_SSCALED_PACK32:
            case A8B8G8R8_UINT_PACK32:
            case A8B8G8R8_SINT_PACK32:
            case A8B8G8R8_SRGB_PACK32:
            case A2R10G10B10_UNORM_PACK32:
            case A2R10G10B10_SNORM_PACK32:
            case A2R10G10B10_USCALED_PACK32:
            case A2R10G10B10_SSCALED_PACK32:
            case A2R10G10B10_UINT_PACK32:
            case A2R10G10B10_SINT_PACK32:
            case A2B10G10R10_UNORM_PACK32:
            case A2B10G10R10_SNORM_PACK32:
            case A2B10G10R10_USCALED_PACK32:
            case A2B10G10R10_SSCALED_PACK32:
            case A2B10G10R10_UINT_PACK32:
            case A2B10G10R10_SINT_PACK32:
                return 4;
            case R16_UNORM:
            case R16_SNORM:
            case R16_USCALED:
            case R16_SSCALED:
            case R16_UINT:
            case R16_SINT:
            case R16_SFLOAT:
                return 2;
            case R16G16_UNORM:
            case R16G16_SNORM:
            case R16G16_USCALED:
            case R16G16_SSCALED:
            case R16G16_UINT:
            case R16G16_SINT:
            case R16G16_SFLOAT:
                return 4;
            case R16G16B16_UNORM:
            case R16G16B16_SNORM:
            case R16G16B16_USCALED:
            case R16G16B16_SSCALED:
            case R16G16B16_UINT:
            case R16G16B16_SINT:
            case R16G16B16_SFLOAT:
                return 6;
            case R16G16B16A16_UNORM:
            case R16G16B16A16_SNORM:
            case R16G16B16A16_USCALED:
            case R16G16B16A16_SSCALED:
            case R16G16B16A16_UINT:
            case R16G16B16A16_SINT:
            case R16G16B16A16_SFLOAT:
                return 8;
            case R32_UINT:
            case R32_SINT:
            case R32_SFLOAT:
                return 4;
            case R32G32_UINT:
            case R32G32_SINT:
            case R32G32_SFLOAT:
                return 8;
            case R32G32B32_UINT:
            case R32G32B32_SINT:
            case R32G32B32_SFLOAT:
                return 12;
            case R32G32B32A32_UINT:
            case R32G32B32A32_SINT:
            case R32G32B32A32_SFLOAT:
                return 16;
            case R64_UINT:
            case R64_SINT:
            case R64_SFLOAT:
                return 8;
            case R64G64_UINT:
            case R64G64_SINT:
            case R64G64_SFLOAT:
                return 16;
            case R64G64B64_UINT:
            case R64G64B64_SINT:
            case R64G64B64_SFLOAT:
                return 24;
            case R64G64B64A64_UINT:
            case R64G64B64A64_SINT:
            case R64G64B64A64_SFLOAT:
                return 32;
            case B10G11R11_UFLOAT_PACK32:
            case E5B9G9R9_UFLOAT_PACK32:
                return 4;
            case D16_UNORM:
                return 2;
            case X8_D24_UNORM_PACK32:
            case D32_SFLOAT:
                return 4;
            case S8_UINT:
                return 1;
            case D16_UNORM_S8_UINT:
                return 3;
            case D24_UNORM_S8_UINT:
                return 4;
            case D32_SFLOAT_S8_UINT:
                return 5;
            case BC1_RGB_UNORM_BLOCK:
            case BC1_RGB_SRGB_BLOCK:
            case BC1_RGBA_UNORM_BLOCK:
            case BC1_RGBA_SRGB_BLOCK:
                return 8;
            case BC2_UNORM_BLOCK:
            case BC2_SRGB_BLOCK:
            case BC3_UNORM_BLOCK:
            case BC3_SRGB_BLOCK:
                return 16;
            case BC4_UNORM_BLOCK:
            case BC4_SNORM_BLOCK:
                return 8;
            case BC5_UNORM_BLOCK:
            case BC5_SNORM_BLOCK:
            case BC6H_UFLOAT_BLOCK:
            case BC6H_SFLOAT_BLOCK:
            case BC7_UNORM_BLOCK:
            case BC7_SRGB_BLOCK:
                return 16;
            case ETC2_R8G8B8_UNORM_BLOCK:
            case ETC2_R8G8B8_SRGB_BLOCK:
            case ETC2_R8G8B8A1_UNORM_BLOCK:
            case ETC2_R8G8B8A1_SRGB_BLOCK:
                return 8;
            case ETC2_R8G8B8A8_UNORM_BLOCK:
            case ETC2_R8G8B8A8_SRGB_BLOCK:
                return 16;
            case EAC_R11_UNORM_BLOCK:
            case EAC_R11_SNORM_BLOCK:
                return 8;
            case EAC_R11G11_UNORM_BLOCK:
            case EAC_R11G11_SNORM_BLOCK:
            case ASTC_4x4_UNORM_BLOCK:
            case ASTC_4x4_SRGB_BLOCK:
            case ASTC_5x4_UNORM_BLOCK:
            case ASTC_5x4_SRGB_BLOCK:
            case ASTC_5x5_UNORM_BLOCK:
            case ASTC_5x5_SRGB_BLOCK:
            case ASTC_6x5_UNORM_BLOCK:
            case ASTC_6x5_SRGB_BLOCK:
            case ASTC_6x6_UNORM_BLOCK:
            case ASTC_6x6_SRGB_BLOCK:
            case ASTC_8x5_UNORM_BLOCK:
            case ASTC_8x5_SRGB_BLOCK:
            case ASTC_8x6_UNORM_BLOCK:
            case ASTC_8x6_SRGB_BLOCK:
            case ASTC_8x8_UNORM_BLOCK:
            case ASTC_8x8_SRGB_BLOCK:
            case ASTC_10x5_UNORM_BLOCK:
            case ASTC_10x5_SRGB_BLOCK:
            case ASTC_10x6_UNORM_BLOCK:
            case ASTC_10x6_SRGB_BLOCK:
            case ASTC_10x8_UNORM_BLOCK:
            case ASTC_10x8_SRGB_BLOCK:
            case ASTC_10x10_UNORM_BLOCK:
            case ASTC_10x10_SRGB_BLOCK:
            case ASTC_12x10_UNORM_BLOCK:
            case ASTC_12x10_SRGB_BLOCK:
            case ASTC_12x12_UNORM_BLOCK:
            case ASTC_12x12_SRGB_BLOCK:
                return 16;
            default:
                std::unreachable();
        }
    }

    [[nodiscard]] constexpr size_t format_byte_size(RenderFormat format) noexcept {
        return format_byte_size(static_cast<Format>(format));
    }
    [[nodiscard]] constexpr size_t format_byte_size(DepthStencilFormat format) noexcept {
        return format_byte_size(static_cast<Format>(format));
    }
    [[nodiscard]] constexpr size_t format_byte_size(VertexFormat format) noexcept {
        return format_byte_size(static_cast<Format>(format));
    }
}