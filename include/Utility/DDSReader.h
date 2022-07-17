#pragma once
#ifndef DDSREADER_H
#define DDSREADER_H
#include <filesystem>
#include "VulkanForwards.h"
#include "ImageReader.h"
namespace Utility {
	//Really don't want to write this but did not really find a good implementation
	//Reference https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-reference
	enum class DXGI_FORMAT {
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_R32G32B32A32_TYPELESS,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_UINT,
		DXGI_FORMAT_R32G32B32A32_SINT,
		DXGI_FORMAT_R32G32B32_TYPELESS,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32_UINT,
		DXGI_FORMAT_R32G32B32_SINT,
		DXGI_FORMAT_R16G16B16A16_TYPELESS,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_R16G16B16A16_UNORM,
		DXGI_FORMAT_R16G16B16A16_UINT,
		DXGI_FORMAT_R16G16B16A16_SNORM,
		DXGI_FORMAT_R16G16B16A16_SINT,
		DXGI_FORMAT_R32G32_TYPELESS,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32_UINT,
		DXGI_FORMAT_R32G32_SINT,
		DXGI_FORMAT_R32G8X24_TYPELESS,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
		DXGI_FORMAT_R10G10B10A2_TYPELESS,
		DXGI_FORMAT_R10G10B10A2_UNORM,
		DXGI_FORMAT_R10G10B10A2_UINT,
		DXGI_FORMAT_R11G11B10_FLOAT,
		DXGI_FORMAT_R8G8B8A8_TYPELESS,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_R8G8B8A8_UINT,
		DXGI_FORMAT_R8G8B8A8_SNORM,
		DXGI_FORMAT_R8G8B8A8_SINT,
		DXGI_FORMAT_R16G16_TYPELESS,
		DXGI_FORMAT_R16G16_FLOAT,
		DXGI_FORMAT_R16G16_UNORM,
		DXGI_FORMAT_R16G16_UINT,
		DXGI_FORMAT_R16G16_SNORM,
		DXGI_FORMAT_R16G16_SINT,
		DXGI_FORMAT_R32_TYPELESS,
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R24G8_TYPELESS,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT,
		DXGI_FORMAT_R8G8_TYPELESS,
		DXGI_FORMAT_R8G8_UNORM,
		DXGI_FORMAT_R8G8_UINT,
		DXGI_FORMAT_R8G8_SNORM,
		DXGI_FORMAT_R8G8_SINT,
		DXGI_FORMAT_R16_TYPELESS,
		DXGI_FORMAT_R16_FLOAT,
		DXGI_FORMAT_D16_UNORM,
		DXGI_FORMAT_R16_UNORM,
		DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT_R16_SNORM,
		DXGI_FORMAT_R16_SINT,
		DXGI_FORMAT_R8_TYPELESS,
		DXGI_FORMAT_R8_UNORM,
		DXGI_FORMAT_R8_UINT,
		DXGI_FORMAT_R8_SNORM,
		DXGI_FORMAT_R8_SINT,
		DXGI_FORMAT_A8_UNORM,
		DXGI_FORMAT_R1_UNORM,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
		DXGI_FORMAT_R8G8_B8G8_UNORM,
		DXGI_FORMAT_G8R8_G8B8_UNORM,
		DXGI_FORMAT_BC1_TYPELESS,
		DXGI_FORMAT_BC1_UNORM,
		DXGI_FORMAT_BC1_UNORM_SRGB,
		DXGI_FORMAT_BC2_TYPELESS,
		DXGI_FORMAT_BC2_UNORM,
		DXGI_FORMAT_BC2_UNORM_SRGB,
		DXGI_FORMAT_BC3_TYPELESS,
		DXGI_FORMAT_BC3_UNORM,
		DXGI_FORMAT_BC3_UNORM_SRGB,
		DXGI_FORMAT_BC4_TYPELESS,
		DXGI_FORMAT_BC4_UNORM,
		DXGI_FORMAT_BC4_SNORM,
		DXGI_FORMAT_BC5_TYPELESS,
		DXGI_FORMAT_BC5_UNORM,
		DXGI_FORMAT_BC5_SNORM,
		DXGI_FORMAT_B5G6R5_UNORM,
		DXGI_FORMAT_B5G5R5A1_UNORM,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_B8G8R8X8_UNORM,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
		DXGI_FORMAT_B8G8R8A8_TYPELESS,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
		DXGI_FORMAT_B8G8R8X8_TYPELESS,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
		DXGI_FORMAT_BC6H_TYPELESS,
		DXGI_FORMAT_BC6H_UF16,
		DXGI_FORMAT_BC6H_SF16,
		DXGI_FORMAT_BC7_TYPELESS,
		DXGI_FORMAT_BC7_UNORM,
		DXGI_FORMAT_BC7_UNORM_SRGB,
		DXGI_FORMAT_AYUV,
		DXGI_FORMAT_Y410,
		DXGI_FORMAT_Y416,
		DXGI_FORMAT_NV12,
		DXGI_FORMAT_P010,
		DXGI_FORMAT_P016,
		DXGI_FORMAT_420_OPAQUE,
		DXGI_FORMAT_YUY2,
		DXGI_FORMAT_Y210,
		DXGI_FORMAT_Y216,
		DXGI_FORMAT_NV11,
		DXGI_FORMAT_AI44,
		DXGI_FORMAT_IA44,
		DXGI_FORMAT_P8,
		DXGI_FORMAT_A8P8,
		DXGI_FORMAT_B4G4R4A4_UNORM,
		DXGI_FORMAT_P208,
		DXGI_FORMAT_V208,
		DXGI_FORMAT_V408,
		DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
		DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
		DXGI_FORMAT_FORCE_UINT
	};
	struct DDSPixelFormat {
		enum class Flags : uint32_t {
			DDPF_ALPHAPIXELS = 0x1,		//Texture contains alpha data; dwRGBAlphaBitMask contains valid data.
			DDPF_ALPHA = 0x2,			//Used in some older DDS files for alpha channel only uncompressed data (dwRGBBitCount contains the alpha channel bitcount; dwABitMask contains valid data)
			DDPF_FOURCC = 0x4,			//Texture contains compressed RGB data; dwFourCC contains valid data.
			DDPF_RGB = 0x40,			//Texture contains uncompressed RGB data; dwRGBBitCount and the RGB masks (dwRBitMask, dwGBitMask, dwBBitMask) contain valid data.
			DDPF_YUV = 0x200,			//Used in some older DDS files for YUV uncompressed data (dwRGBBitCount contains the YUV bit count; dwRBitMask contains the Y mask, dwGBitMask contains the U mask, dwBBitMask contains the V mask)
			DDPF_LUMINANCE = 0x20000,	//Used in some older DDS files for single channel color uncompressed data (dwRGBBitCount contains the luminance channel bit count; dwRBitMask contains the channel mask). Can be combined with DDPF_ALPHAPIXELS for a two channel DDS file.
		};
		friend Flags operator|(Flags lhs, Flags rhs)
		{
			return static_cast<Flags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		}
		friend bool operator&(Flags lhs, Flags rhs)
		{
			return (static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) != 0;
		}
		enum class FourCC : uint32_t {
			Empty = 0,
			DXT1 = 0x31545844u,
			DXT2 = 0x32545844u,
			DXT3 = 0x33545844u,
			DXT4 = 0x34545844u,
			DXT5 = 0x35545844u,
			BC4U = 1429488450u,
			BC4S = 1395934018u,
			BC5U = 843666497u,
			BC5S = 1395999554u,
			DX10 = 0x30315844u,

		};
		uint32_t size;					//Structure size; set to 32 (bytes).
		Flags flags;					//Values which indicate what type of data is in the surface.
		FourCC fourCC;					//Four-character codes for specifying compressed or custom formats. Possible values include: DXT1, DXT2, DXT3, DXT4, or DXT5. A FourCC of DX10 indicates the prescense of the DDS_HEADER_DXT10 extended header, and the dxgiFormat member of that structure indicates the true format. When using a four-character code, dwFlags must include DDPF_FOURCC.
		uint32_t RGBBitCount;			//Number of bits in an RGB (possibly including alpha) format. Valid when dwFlags includes DDPF_RGB, DDPF_LUMINANCE, or DDPF_YUV.
		uint32_t RBitMask;				//Red (or lumiance or Y) mask for reading color data. For instance, given the A8R8G8B8 format, the red mask would be 0x00ff0000
		uint32_t GBitMask;				//Green (or U) mask for reading color data. For instance, given the A8R8G8B8 format, the green mask would be 0x0000ff00.
		uint32_t BBitMask;				//Blue (or V) mask for reading color data. For instance, given the A8R8G8B8 format, the blue mask would be 0x000000ff.
		uint32_t ABitMask;				//Alpha mask for reading alpha data. dwFlags must include DDPF_ALPHAPIXELS or DDPF_ALPHA. For instance, given the A8R8G8B8 format, the alpha mask would be 0xff000000.
	};
	struct DDSHeader {
		/*!When you write .dds files, you should set the DDSD_CAPS and DDSD_PIXELFORMAT flags, and for mipmapped
		///textures you should also set the DDSD_MIPMAPCOUNT flag. However, when you read a .dds file, you
		///should not rely on the DDSD_CAPS, DDSD_PIXELFORMAT, and DDSD_MIPMAPCOUNT flags being set because
		///some writers of such a file might not set these flags.*/
		enum class Flags : uint32_t {
			CAPS = 0x1,				//Required in every .dds file.
			HEIGHT = 0x2,			//Required in every .dds file.
			WIDTH = 0x4,			//Required in every .dds file.
			PITCH = 0x8,			//Required when pitch is provided for an uncompressed texture.
			PIXELFORMAT = 0x1000,	//Required in every .dds file.
			MIPMAPCOUNT = 0x20000,	//Required in a mipmapped texture.
			LINEARSIZE = 0x80000,	//Required when pitch is provided for a compressed texture
			DEPTH = 0x800000		//Required in a depth texture.

		};
		friend Flags operator|(Flags lhs, Flags rhs)
		{
			return static_cast<Flags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		}
		friend bool operator&(Flags lhs, Flags rhs)
		{
			return (static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) != 0;
		}
		enum class Caps :uint32_t {
			COMPLEX = 0x8,				//Optional; must be used on any file that contains more than one surface (a mipmap, a cubic environment map, or mipmapped volume texture).
			MIPMAP = 0x400000,			//Optional; should be used for a mipmap.
			TEXTURE = 0x1000,			//Required
		};
		friend Caps operator|(Caps lhs, Caps rhs)
		{
			return static_cast<Caps>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		}
		friend bool operator&(Caps lhs, Caps rhs)
		{
			return (static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) != 0;
		}
		enum class Caps2 :uint32_t {
			CUBEMAP = 0x200,			//Required for a cube map.	
			CUBEMAP_POSITIVEX = 0x400,	//Required when these surfaces are stored in a cube map.	
			CUBEMAP_NEGATIVEX = 0x800,	//Required when these surfaces are stored in a cube map.	
			CUBEMAP_POSITIVEY = 0x1000,	//Required when these surfaces are stored in a cube map.	
			CUBEMAP_NEGATIVEY = 0x2000,	//Required when these surfaces are stored in a cube map.	
			CUBEMAP_POSITIVEZ = 0x4000,	//Required when these surfaces are stored in a cube map.	
			CUBEMAP_NEGATIVEZ = 0x8000,	//Required when these surfaces are stored in a cube map.	
			VOLUME = 0x200000,			//Required for a volume texture.	
		};
		friend Caps2 operator|(Caps2 lhs, Caps2 rhs)
		{
			return static_cast<Caps2>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		}
		friend bool operator&(Caps2 lhs, Caps2 rhs)
		{
			return (static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) != 0;
		}
		uint32_t size;					//Size of structure. This member must be set to 124.
		Flags flags;					//Flags to indicate which members contain valid data.q
		uint32_t height;				//Surface height (in pixels).
		uint32_t width;					//Surface width (in pixels).
		union {
			uint32_t pitch;				//The pitch per scan line in an uncompressed texture;
			uint32_t linearSize;		//The total number of bytes in the top level texture for a compressed texture.
		};
		uint32_t depth;					//Depth of a volume texture (in pixels), otherwise unused.
		uint32_t mipMapCount;			//Number of mipmap levels, otherwise unused.
		uint32_t reserved[11];			//Unused.
		DDSPixelFormat pixelFormat;		//The pixel format (see DDS_PIXELFORMAT).
		//When you write .dds files, you should set the DDSCAPS_TEXTURE flag, and for multiple surfaces you 
		//should also set the DDSCAPS_COMPLEX flag. However, when you read a .dds file, you should not rely on
		//the DDSCAPS_TEXTURE and DDSCAPS_COMPLEX flags being set because some writers of such a file might not set these flags.
		Caps caps;						//Specifies the complexity of the surfaces stored.
		Caps2 caps2;					//Additional detail about the surfaces stored.
		uint32_t caps3;					//Unused.
		uint32_t caps4;					//Unused.
		uint32_t reserved2;				//Unused.
	};
	struct DDSHeaderDXT10 {
		enum class RESOURCE_DIMENSION {
			TEXTURE1D = 0x2,	//Resource is a 1D texture. The dwWidth member of DDS_HEADER specifies the size of the texture. Typically, you set the dwHeight member of DDS_HEADER to 1; you also must set the DDSD_HEIGHT flag in the dwFlags member of DDS_HEADER.
			TEXTURE2D = 0x3,	//Resource is a 2D texture with an area specified by the dwWidth and dwHeight members of DDS_HEADER. You can also use this type to identify a cube-map texture. For more information about how to identify a cube-map texture, see miscFlag and arraySize members.
			TEXTURE3D = 0x4,	//Resource is a 3D texture with a volume specified by the dwWidth, dwHeight, and dwDepth members of DDS_HEADER. You also must set the DDSD_DEPTH flag in the dwFlags member of DDS_HEADER.
		};
		enum class MiscFlags : uint32_t {
			UNKNOWN = 0x0,
			TEXTURECUBE = 0x4					//	Indicates a 2D texture is a cube-map texture.	
		};
		friend MiscFlags operator|(MiscFlags lhs, MiscFlags rhs)
		{
			return static_cast<MiscFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		}
		friend bool operator&(MiscFlags lhs, MiscFlags rhs)
		{
			return (static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) != 0;
		}
		enum class MiscFlags2 : uint32_t {
			DDS_ALPHA_MODE_UNKNOWN = 0x0,		//Alpha channel content is unknown. This is the value for legacy files, which typically is assumed to be 'straight' alpha.
			DDS_ALPHA_MODE_STRAIGHT = 0x1,		//Alpha channel content is unknown. This is the value for legacy files, which typically is assumed to be 'straight' alpha.
			DDS_ALPHA_MODE_PREMULTIPLIED = 0x2, //Alpha channel content is unknown. This is the value for legacy files, which typically is assumed to be 'straight' alpha.
			DDS_ALPHA_MODE_OPAQUE = 0x3,		//Any alpha channel content is all set to fully opaque.
			DDS_ALPHA_MODE_CUSTOM = 0x4			//Any alpha channel content is being used as a 4th channel and is not intended to represent transparency (straight or premultiplied).
		};
		friend MiscFlags2 operator|(MiscFlags2 lhs, MiscFlags2 rhs)
		{
			return static_cast<MiscFlags2>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		}
		friend bool operator&(MiscFlags2 lhs, MiscFlags2 rhs)
		{
			return (static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) != 0;
		}
		DXGI_FORMAT format;						//The surface pixel format (see DXGI_FORMAT).
		RESOURCE_DIMENSION resourceDimension;	//Identifies the type of resource. The following values for this member are a subset of the values in the D3D10_RESOURCE_DIMENSION or D3D11_RESOURCE_DIMENSION enumeration:
		MiscFlags miscFlag;						//Identifies other, less common options for resources.
		uint32_t arraySize;						//The number of elements in the array. If this is a cube-map => 6*arraySize 2D textures
		MiscFlags2 miscFlags2;					//Contains additional metadata (formerly was reserved). The lower 3 bits indicate the alpha mode of the associated resource. The upper 29 bits are reserved and are typically 0.
	};

	struct DDSImage {
		uint32_t magicNumber;
		DDSHeader header;
		DDSHeaderDXT10 extHeader;
	};
	class DDSReader {
	public:
		static std::vector<std::byte> readDDSFileInMemory(const std::filesystem::path& filename);
		static TextureInfo readDDSFileHeader(const std::filesystem::path& filename, bool strict = false);
		static std::vector<vulkan::InitialImageData> parseImage(const Utility::TextureInfo& info, const std::vector<std::byte>& data, uint32_t startMipLevel = 0);
	private:
	};
}

#endif !DDSREADER_H