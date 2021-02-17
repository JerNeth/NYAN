#include "Renderer/TextureManager.h"
#include "DDSReader.h"

nyan::TextureManager::TextureManager(vulkan::LogicalDevice& device, bool streaming) :
	r_device(device),
	m_streaming(streaming)
{
}

::vulkan::Image* nyan::TextureManager::request_texture(const std::string& name)
{
	if (const auto& res = m_usedTextures.find(name); res != m_usedTextures.end()) {
		return res->second;
	}
	auto newRes = m_usedTextures.emplace(name, create_image(name, 0));
	return newRes.first->second;
}

void nyan::TextureManager::change_mip(const std::string& name, uint32_t targetMip)
{
	const auto& res = m_usedTextures.find(name);
	if (res == m_usedTextures.end())
		return;
	vulkan::Image& image = *res->second;
	if (image.is_sparse()) {
		if (image.get_available_mip() == targetMip)
			return;
		else if (targetMip < image.get_available_mip()) {
			auto info = Utility::DDSReader::readDDSFileHeader(name);
			auto imgData = Utility::DDSReader::readDDSFileInMemory(name);
			std::vector<vulkan::InitialImageData> initalImageData = Utility::DDSReader::parseImage(info,imgData);
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			r_device.upsize_sparse_image(image, initalImageData.data(), targetMip);
		}
		else if (targetMip <= image.get_info().mipLevels) {
			r_device.downsize_sparse_image(image, targetMip);
		}
	}
	else {
		auto info = Utility::DDSReader::readDDSFileHeader(name);
		if ((info.mipLevels -targetMip) == image.get_info().mipLevels)
			return;
		auto newImage = create_image(name, targetMip);
		image = std::move(*newImage);
	}

}

vulkan::ImageHandle nyan::TextureManager::create_image(const std::string& name, uint32_t mipLevel)
{

	auto info = Utility::DDSReader::readDDSFileHeader(name);
	auto imgData = Utility::DDSReader::readDDSFileInMemory(name);
	std::vector<vulkan::InitialImageData> initalImageData = Utility::DDSReader::parseImage(info, imgData, mipLevel);
	info.mipLevels = info.mipLevels - mipLevel;
	info.width = Math::max(1u, info.width >> mipLevel);
	info.height = Math::max(1u, info.height >> mipLevel);
	info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	if (m_useSparse) 
		return r_device.create_sparse_image(info, initalImageData.data());
	else
		return r_device.create_image(info, initalImageData.data());
}
