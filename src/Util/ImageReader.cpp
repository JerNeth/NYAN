#include "Utility/ImageReader.h"
#include "stb_image.h"
#include "VkWrapper.h"

std::pair<vulkan::InitialImageData, Utility::TextureInfo> Utility::ImageReader::read_image_file(const std::filesystem::path& filename)
{
	if (!std::filesystem::exists(filename))
		throw std::runtime_error("File does not exist " +filename.string());
	if (filename.extension().compare(".png") &&
		filename.extension().compare(".jpg") &&
		filename.extension().compare(".hdr") &&
		filename.extension().compare(".tga"))
		throw std::runtime_error("Not supported file extension " + filename.extension().string());
	int width, height, channels;
	vulkan::InitialImageData data 
	{
		.data = stbi_load(filename.string().c_str(), &width, &height, &channels, 4),
		
	};
	if(!data.data)
		throw std::runtime_error("Failed to load image " + filename.string());
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
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