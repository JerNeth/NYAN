#include "Utility/ImageReader.h"
#include "Utility/Exceptions.h"
#include "stb_image.h"
#include "VkWrapper.h"

std::pair<vulkan::InitialImageData, Utility::TextureInfo> Utility::ImageReader::read_image_file(const std::filesystem::path& filename)
{
	if (!std::filesystem::exists(filename))
		throw Utility::FileNotFoundException(std::format("File does not exist: \"{}\"", filename.string()));
	if (filename.extension().compare(".png") &&
		filename.extension().compare(".jpg") &&
		filename.extension().compare(".hdr") &&
		filename.extension().compare(".tga"))
		throw Utility::FileTypeNotSupportedException("Not supported file extension " + filename.extension().string());
	int width, height, channels;
	vulkan::InitialImageData data 
	{
		.data = stbi_load(filename.string().c_str(), &width, &height, &channels, 4),
		
	};
	assert(data.data);
	if(!data.data)
		throw std::runtime_error("Failed to load image " + filename.string());

	//const uint8_t* tmp = reinterpret_cast<const uint8_t*>(data.data);
	//std::vector<uint8_t> debugData( width * height * channels );
	//for (int i = 0; i < width * height * channels; ++i)
	//	debugData[i] = tmp[i];

	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	//VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	//switch (channels) {
	//case 1:
	//	format = VK_FORMAT_R8_SRGB;
	//	break;
	//case 2:
	//	format = VK_FORMAT_R8G8_SRGB;
	//	break;
	//case 3:
	//	format = VK_FORMAT_R8G8B8_SRGB;
	//	break;
	//case 4:
	//	format = VK_FORMAT_R8G8B8A8_SRGB;
	//	break;
	//}

	TextureInfo info
	{
		.format = format,
		.type = VK_IMAGE_TYPE_2D,
		.width = static_cast<uint32_t>(width),
		.height = static_cast<uint32_t>(height),
		.depth = 1,
		.arrayLayers = 1,
		.mipLevels = 1,
		.cube = false,
	};

	return { data, info };
}
void Utility::ImageReader::free_image(const void* img)
{
	stbi_image_free(const_cast<void*>(img));
}