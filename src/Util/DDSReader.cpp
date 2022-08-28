#include "Utility/DDSReader.h"
#include "LinAlg.h"
#include "VulkanWrapper/VulkanIncludes.h"
#include "VkWrapper.h"
#include "Image.h"

static VkFormat convertToVk(Utility::DXGI_FORMAT format) {
	switch (format) {
	case Utility::DXGI_FORMAT::DXGI_FORMAT_UNKNOWN:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_TYPELESS:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_UINT:
		return VK_FORMAT_R32G32B32A32_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_SINT:
		return VK_FORMAT_R32G32B32A32_SINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32B32_TYPELESS:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32B32_UINT:
		return VK_FORMAT_R32G32B32_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32B32_SINT:
		return VK_FORMAT_R32G32B32_SINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_TYPELESS:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UNORM:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UINT:
		return VK_FORMAT_R16G16B16A16_USCALED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_SNORM:
		return VK_FORMAT_R16G16B16A16_SNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_SINT:
		return VK_FORMAT_R16G16B16A16_SSCALED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32_TYPELESS:
		return VK_FORMAT_R32G32_SFLOAT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT:
		return VK_FORMAT_R32G32_SFLOAT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32_UINT:
		return VK_FORMAT_R32G32_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G32_SINT:
		return VK_FORMAT_R32G32_SINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32G8X24_TYPELESS:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_TYPELESS:
		return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_UNORM:
		return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_UINT:
		return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT:
		return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UINT:
		return VK_FORMAT_R8G8B8A8_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_SNORM:
		return VK_FORMAT_R8G8B8A8_SNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_SINT:
		return VK_FORMAT_R8G8B8A8_SINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16_TYPELESS:
		return VK_FORMAT_R16G16_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT:
		return VK_FORMAT_R16G16_SFLOAT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16_UNORM:
		return VK_FORMAT_R16G16_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT:
		return VK_FORMAT_R16G16_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16_SNORM:
		return VK_FORMAT_R16G16_SNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16G16_SINT:
		return VK_FORMAT_R16G16_SINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS:
		return VK_FORMAT_R32_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT:
		return VK_FORMAT_D32_SFLOAT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32_UINT:
		return VK_FORMAT_R32_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R32_SINT:
		return VK_FORMAT_R32_SINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8_TYPELESS:
		return VK_FORMAT_R8G8_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM:
		return VK_FORMAT_R8G8_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8_UINT:
		return VK_FORMAT_R8G8_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8_SNORM:
		return VK_FORMAT_R8G8_SNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8_SINT:
		return VK_FORMAT_R8G8_SINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16_TYPELESS:
		return VK_FORMAT_R16_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT:
		return VK_FORMAT_R16_SFLOAT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_D16_UNORM:
		return VK_FORMAT_D16_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16_UNORM:
		return VK_FORMAT_R16_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16_UINT:
		return VK_FORMAT_R16_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16_SNORM:
		return VK_FORMAT_R16_SNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R16_SINT:
		return VK_FORMAT_R16_SINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8_TYPELESS:
		return VK_FORMAT_R8_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8_UNORM:
		return VK_FORMAT_R8_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8_UINT:
		return VK_FORMAT_R8_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8_SNORM:
		return VK_FORMAT_R8_SNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8_SINT:
		return VK_FORMAT_R8_SINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_A8_UNORM:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R1_UNORM:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R8G8_B8G8_UNORM:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_G8R8_G8B8_UNORM:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC1_TYPELESS:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM_SRGB:
		return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC2_TYPELESS:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM_SRGB:
		return VK_FORMAT_BC2_SRGB_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC3_TYPELESS:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM_SRGB:
		return VK_FORMAT_BC3_SRGB_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC4_TYPELESS:
		return VK_FORMAT_BC4_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC4_UNORM:
		return VK_FORMAT_BC4_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC4_SNORM:
		return VK_FORMAT_BC4_SNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC5_TYPELESS:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC5_SNORM:
		return VK_FORMAT_BC5_SNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_B5G6R5_UNORM:
		return VK_FORMAT_B5G6R5_UNORM_PACK16;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_B5G5R5A1_UNORM:
		return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_TYPELESS:
		return VK_FORMAT_B8G8R8A8_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_TYPELESS:
		return VK_FORMAT_B8G8R8A8_UINT;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC6H_TYPELESS:
		return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16:
		return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC6H_SF16:
		return VK_FORMAT_BC6H_SFLOAT_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC7_TYPELESS:
		return VK_FORMAT_BC7_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM:
		return VK_FORMAT_BC7_UNORM_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB:
		return VK_FORMAT_BC7_SRGB_BLOCK;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_AYUV:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_Y410:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_Y416:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_NV12:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_P010:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_P016:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_420_OPAQUE:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_YUY2:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_Y210:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_Y216:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_NV11:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_AI44:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_IA44:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_P8:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_A8P8:
		return VK_FORMAT_UNDEFINED;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_B4G4R4A4_UNORM:
		return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
	case Utility::DXGI_FORMAT::DXGI_FORMAT_P208:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_V208:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_V408:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
	case Utility::DXGI_FORMAT::DXGI_FORMAT_FORCE_UINT:
	default:
		return VK_FORMAT_UNDEFINED;
	}
}
constexpr uint32_t DDSMagicNumber = 0x20534444u;
std::vector<std::byte> Utility::DDSReader::readDDSFileInMemory(const std::filesystem::path& filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!(file.is_open())) {
		throw std::runtime_error("Could not open file: \"" + filename.string() + "\"");
	}
	auto fileSize = std::filesystem::file_size(filename);
	if (fileSize < 128)
		throw std::runtime_error("File is not a valid .dds file. Reason (file too small)");
	uint32_t magicNumber;
	Utility::DDSHeader header;
	file.read(reinterpret_cast<char*>(&magicNumber), sizeof(uint32_t));
	if (magicNumber != DDSMagicNumber)
		throw std::runtime_error("File is not a valid .dds file. Reason (file too small)");
	file.read(reinterpret_cast<char*>(&header), sizeof(Utility::DDSHeader));
	if (header.size != 124u)
		throw std::runtime_error("File is not a valid .dds file. Reason (invalid header.size)");
	if (header.pixelFormat.size != 32)
		throw std::runtime_error("File is not a valid .dds file. Reason (invalid pixelFormat.size)");
	if ((header.pixelFormat.fourCC == DDSPixelFormat::FourCC::DX10) && (fileSize <= 148))
		throw std::runtime_error("File is not a valid .dds file. Reason (file too small for DX10)");
	DDSHeaderDXT10 extHeader;
	if ((header.pixelFormat.fourCC == DDSPixelFormat::FourCC::DX10))
		file.read(reinterpret_cast<char*>(&extHeader), sizeof(Utility::DDSHeaderDXT10));
	fileSize -= file.tellg();
	std::vector<std::byte> buffer(fileSize / sizeof(uint8_t));
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize );
	file.close();
	return buffer;
}
Utility::TextureInfo Utility::DDSReader::readDDSFileHeader(const std::filesystem::path& filename, bool strict)
{
	Utility::TextureInfo ret{};

	std::ifstream file(filename, std::ios::binary);
	if (!(file.is_open())) {
		throw std::runtime_error("Could not open file: \"" + filename.string() + "\"");
	}
	Utility::DDSHeader header;
	auto fileSize = std::filesystem::file_size(filename);
	if (fileSize < 128)
		throw std::runtime_error("File is not a valid .dds file. Reason (file too small)");
	uint32_t magicNumber;
	file.read(reinterpret_cast<char*>(&magicNumber), sizeof(uint32_t));

	if (magicNumber != DDSMagicNumber)
		throw std::runtime_error("File is not a valid .dds file. Reason (file too small)");

	file.read(reinterpret_cast<char*>(&header), sizeof(Utility::DDSHeader));
	if (header.size != 124u)
		throw std::runtime_error("File is not a valid .dds file. Reason (invalid header.size)");
	if (header.pixelFormat.size != 32)
		throw std::runtime_error("File is not a valid .dds file. Reason (invalid pixelFormat.size)");
	if ((header.pixelFormat.fourCC == DDSPixelFormat::FourCC::DX10) && (fileSize <= 148))
		throw std::runtime_error("File is not a valid .dds file. Reason (file too small for DX10)");

	DDSHeaderDXT10 extHeader{};
	if((header.pixelFormat.fourCC == DDSPixelFormat::FourCC::DX10))
		file.read(reinterpret_cast<char*>(&extHeader), sizeof(Utility::DDSHeaderDXT10));
	file.close();
	ret.width = header.width;
	ret.height = header.height;
	bool isCubeMap = false;
	bool isVolume = false;
	if (strict) {
		if (header.flags & DDSHeader::Flags::DEPTH) {
			ret.depth = header.depth;
			if (header.caps2 & DDSHeader::Caps2::VOLUME)
				isVolume = true;
			else
				throw std::runtime_error("File is not a valid .dds file. Reason (VOLUME caps2 not set but DEPTH specified)");
		}
		else
			ret.depth = 1;
		if (header.flags & DDSHeader::Flags::MIPMAPCOUNT) {
			ret.mipLevels = header.mipMapCount;
		}
		else
			ret.mipLevels = 1;
		if (header.caps2 & DDSHeader::Caps2::CUBEMAP) {
			if (header.caps2 & (DDSHeader::Caps2::CUBEMAP_POSITIVEX |
				DDSHeader::Caps2::CUBEMAP_NEGATIVEX |
				DDSHeader::Caps2::CUBEMAP_POSITIVEY |
				DDSHeader::Caps2::CUBEMAP_NEGATIVEY |
				DDSHeader::Caps2::CUBEMAP_POSITIVEZ |
				DDSHeader::Caps2::CUBEMAP_NEGATIVEZ))
				isCubeMap = true;
			else
				throw std::runtime_error("File is not a valid .dds file. Reason (CUBEMAP caps2 set but not all CUBEMAP faces defined)");
		}
	}
	else {
		ret.depth = header.depth;
		if (!ret.depth) {
			ret.depth = 1;
			Utility::log().format("{} has no depth defined", filename.string());
		}
		ret.mipLevels = header.mipMapCount;
		assert(ret.mipLevels);
		if (ret.depth != 1)
			isVolume = true;
		if (header.caps2 & DDSHeader::Caps2::CUBEMAP)
			isCubeMap = true;
	}
	bool dx10Header = false;
	switch (header.pixelFormat.fourCC) {
	case (DDSPixelFormat::FourCC::DXT1):
		ret.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		break;
	case (DDSPixelFormat::FourCC::DXT2):
		ret.format = VK_FORMAT_BC2_UNORM_BLOCK;
		break;
	case (DDSPixelFormat::FourCC::DXT3):
		ret.format = VK_FORMAT_BC2_UNORM_BLOCK;
		break;
	case (DDSPixelFormat::FourCC::DXT4):
		ret.format = VK_FORMAT_BC3_UNORM_BLOCK;
		break;
	case (DDSPixelFormat::FourCC::DXT5):
		ret.format = VK_FORMAT_BC3_UNORM_BLOCK;
		break;
	case (DDSPixelFormat::FourCC::BC5U):
		ret.format = VK_FORMAT_BC5_UNORM_BLOCK;
		break;
	case (DDSPixelFormat::FourCC::BC5S):
		ret.format = VK_FORMAT_BC5_SNORM_BLOCK;
		break;
	case (DDSPixelFormat::FourCC::DX10):
		dx10Header = true;
		break;
	case (DDSPixelFormat::FourCC::Empty):
		break;
	default:
		assert(false); //Not supported yet
	}
	if (isCubeMap)
		ret.arrayLayers = 6;
	else
		ret.arrayLayers = 1;
	if (dx10Header) {
		ret.format = convertToVk(extHeader.format);
		assert(ret.format != VK_FORMAT_UNDEFINED);
		if (isCubeMap)
			assert(extHeader.miscFlag & DDSHeaderDXT10::MiscFlags::TEXTURECUBE);
		isCubeMap = extHeader.miscFlag & DDSHeaderDXT10::MiscFlags::TEXTURECUBE;
		
		ret.arrayLayers *= extHeader.arraySize;
		if (isVolume)
			assert(ret.arrayLayers == 1);
		switch (extHeader.resourceDimension) {
		case DDSHeaderDXT10::RESOURCE_DIMENSION::TEXTURE1D:
			ret.type = VK_IMAGE_TYPE_1D;
			break;
		case DDSHeaderDXT10::RESOURCE_DIMENSION::TEXTURE2D:
			ret.type = VK_IMAGE_TYPE_2D;
			break;
		case DDSHeaderDXT10::RESOURCE_DIMENSION::TEXTURE3D:
			ret.type = VK_IMAGE_TYPE_3D;
			assert(isVolume);
			break;
		default:
			assert(false);
			break;
		}
	}
	else {
		if (ret.depth != 1)
			ret.type = VK_IMAGE_TYPE_3D;
		else if (ret.height != 1)
			ret.type = VK_IMAGE_TYPE_2D;
		else
			ret.type = VK_IMAGE_TYPE_1D;
		if (ret.format == VK_FORMAT_UNDEFINED) {
			if (header.pixelFormat.flags & DDSPixelFormat::Flags::DDPF_RGB) {
				if (header.pixelFormat.flags & DDSPixelFormat::Flags::DDPF_ALPHA || header.pixelFormat.flags & DDSPixelFormat::Flags::DDPF_ALPHAPIXELS) {
					if ((header.pixelFormat.RBitMask & 0xff000000) &&
						(header.pixelFormat.GBitMask & 0xff0000) &&
						(header.pixelFormat.BBitMask & 0xff00) &&
						(header.pixelFormat.ABitMask & 0xff))
						ret.format = VK_FORMAT_R8G8B8A8_UNORM;
					else if ((header.pixelFormat.RBitMask & 0xff00) &&
						(header.pixelFormat.GBitMask & 0xff0000) &&
						(header.pixelFormat.BBitMask & 0xff000000) &&
						(header.pixelFormat.ABitMask & 0xff))
						ret.format = VK_FORMAT_B8G8R8A8_UNORM; 
					else if ((header.pixelFormat.RBitMask & 0xff) &&
							(header.pixelFormat.GBitMask & 0xff00) &&
							(header.pixelFormat.BBitMask & 0xff0000) &&
							(header.pixelFormat.ABitMask & 0xff000000))
						ret.format = VK_FORMAT_R8G8B8A8_UNORM;
					else if ((header.pixelFormat.RBitMask & 0xff0000) &&
						(header.pixelFormat.GBitMask & 0xff00) &&
						(header.pixelFormat.BBitMask & 0xff) &&
						(header.pixelFormat.ABitMask & 0xff000000))
						ret.format = VK_FORMAT_B8G8R8A8_UNORM;
					else
						assert(false);
				}
				else {
					if (header.pixelFormat.RGBBitCount == 24) {
						if ((header.pixelFormat.RBitMask & 0xff0000) &&
							(header.pixelFormat.GBitMask & 0xff00) &&
							(header.pixelFormat.BBitMask & 0xff))
							ret.format = VK_FORMAT_R8G8B8_UNORM;
						else if ((header.pixelFormat.RBitMask & 0xff) &&
							(header.pixelFormat.GBitMask & 0xff00) &&
							(header.pixelFormat.BBitMask & 0xff0000))
							ret.format = VK_FORMAT_B8G8R8_UNORM;
						else
							assert(false);
					}
					else if (header.pixelFormat.RGBBitCount == 16) {
						if ((header.pixelFormat.RBitMask & 0xff00) &&
							(header.pixelFormat.GBitMask & 0xff))
							ret.format = VK_FORMAT_R8G8_UNORM;
						else
							assert(false);
					}
					else if (header.pixelFormat.RGBBitCount == 8) {
						if ((header.pixelFormat.RBitMask & 0xff))
							ret.format = VK_FORMAT_R8_UNORM;
						else
							assert(false);
					}
					else {
						assert(false);
					}
				}
			}
			else {
				assert(false);
			}
		}
	}

	if (isCubeMap)
		ret.cube =  true;
	return ret;
}
std::vector<vulkan::InitialImageData> Utility::DDSReader::parseImage(const Utility::TextureInfo& info, const std::vector<std::byte>& data, uint32_t startMipLevel) {
	//TODO could directly read from file into vulkan buffer ?
	std::vector<vulkan::InitialImageData> initalData;
	std::vector<uint32_t> levelSizes(info.mipLevels);
	size_t totalSize = 0;
	startMipLevel = Math::min(startMipLevel, info.mipLevels);
	std::array<uint32_t, 16> mipOffsets{};
	for (uint32_t mipLevel = 0; mipLevel < info.mipLevels; mipLevel++) {
		auto [blockWidth, blockHeight] = vulkan::format_to_block_size(info.format);
		auto blockStride = vulkan::format_block_size(info.format);
		uint32_t mipWidth = Math::max(1ul, info.width >> mipLevel);
		uint32_t mipHeight = Math::max(1ul, info.height >> mipLevel);
		uint32_t mipSize = Math::max(1ul, (mipWidth + blockWidth - 1) / blockWidth) *
			Math::max(1ul, (mipHeight + blockHeight - 1) / blockHeight) * blockStride;
		if(mipLevel >= startMipLevel)
			mipOffsets[mipLevel- startMipLevel] = static_cast<uint32_t>(totalSize);
		totalSize += mipSize;
		//std::cout << "Level (" << mipLevel << "): " << mipSize << " Bytes\t\tTotal: " << totalSize << " Bytes \n";
	}
	//std::cout << "Mipoffset: " << mipOffset << '\n';
	const std::byte* ptr = data.data();
	initalData.reserve(info.arrayLayers);
	for (size_t arrayLayer = 0; arrayLayer < info.arrayLayers; arrayLayer++) {
		vulkan::InitialImageData initialImageData{
			.data = reinterpret_cast<const void*>(ptr  + totalSize * arrayLayer ),
			.mipOffsets = mipOffsets,
			.mipCounts = info.mipLevels - startMipLevel
		};
		initalData.push_back(initialImageData);
	}
	return initalData;
	//ret.mipLevels = ret.mipLevels - startMipLevel;
	//ret.width = Math::max(1u, ret.width >> startMipLevel);
	//ret.height = Math::max(1u, ret.height >> startMipLevel);
}