#include "Renderer/TextureManager.h"

nyan::TextureManager::TextureManager(vulkan::LogicalDevice& device, bool streaming) :
	r_device(device),
	m_streaming(streaming)
{
}

::vulkan::Image* nyan::TextureManager::request_texture(const std::string& name)
{
	if (const auto& res = m_usedTextures.find(name); res != m_usedTextures.end()) {
		return res->second.first;
	}
	return create_image(name, m_minimumMipLevel);
}

void nyan::TextureManager::change_mip(const std::string& name, uint32_t targetMip)
{
	const auto& res = m_usedTextures.find(name);
	if (res == m_usedTextures.end())
		return;
	if (targetMip < m_minimumMipLevel)
		targetMip = m_minimumMipLevel;
	auto& pair = res->second;
	vulkan::Image& image = *pair.first;
	if (image.is_being_resized())
		return;
	if (image.is_sparse()) {
		if (image.get_available_mip() == targetMip)
			return;
		else if (targetMip < image.get_available_mip()) {
			auto imgData = Utility::DDSReader::readDDSFileInMemory(name);
			std::vector<vulkan::InitialImageData> initalImageData = Utility::DDSReader::parseImage(pair.second,imgData);
			r_device.upsize_sparse_image(image, initalImageData.data(), targetMip);
		}
		else if (targetMip <= image.get_info().mipLevels) {
			r_device.downsize_sparse_image(image, targetMip);
		}
	}
	else {
		if ((pair.second.mipLevels - targetMip) == image.get_info().mipLevels)
			return;
		auto newImage = create_image(name, targetMip);
		image = std::move(*newImage);
	}

}

vulkan::ImageHandle nyan::TextureManager::create_image(const std::string& name, uint32_t mipLevel)
{

	auto texInfo = Utility::DDSReader::readDDSFileHeader(name);
	auto imgData = Utility::DDSReader::readDDSFileInMemory(name);
	std::vector<vulkan::InitialImageData> initalImageData = Utility::DDSReader::parseImage(texInfo, imgData, mipLevel);

	vulkan::ImageInfo info{
		.format = texInfo.format,
		.width = Math::max(1u, texInfo.width >> mipLevel),
		.height = Math::max(1u, texInfo.height >> mipLevel),
		.depth = texInfo.depth,
		.mipLevels = texInfo.mipLevels - mipLevel,
		.arrayLayers = texInfo.arrayLayers,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.type = texInfo.type,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.flags = texInfo.cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : static_cast<VkImageCreateFlags>(0x0u),
	};
	if (m_useSparse) {
		info.flags |= (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT);
		auto image =  r_device.create_sparse_image(info, initalImageData.data());
		m_usedTextures.emplace(name,std::make_pair( image, texInfo));
		return image;
	}
	else {
		auto image = r_device.create_image(info, initalImageData.data());
		m_usedTextures.emplace(name, std::make_pair(image, texInfo));
		return image;
	}
}
