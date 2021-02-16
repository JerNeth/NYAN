#include "..\..\include\Renderer\TextureManager.h"
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
	//Could in fact only load interested Mip levels into memory
	auto ImgData = Utility::DDSReader::readDDSFileInMemory(name);
	std::vector<vulkan::InitialImageData> initalImageData;
	auto a = Utility::DDSReader::parseImage(ImgData, initalImageData);
	a.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	a.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	auto tex = r_device.create_image(a, initalImageData.data());
	m_usedTextures.emplace(name, tex);
	return tex;
}

void nyan::TextureManager::change_mip(const std::string& name, uint32_t targetMip)
{
	const auto& res = m_usedTextures.find(name);
	if (res == m_usedTextures.end())
		return;
	vulkan::Image& image = *res->second;
	if (image.get_available_mip() == targetMip)
		return;
	else if (targetMip < image.get_available_mip()) {
		auto ImgData = Utility::DDSReader::readDDSFileInMemory(name);
		std::vector<vulkan::InitialImageData> initalImageData;
		auto a = Utility::DDSReader::parseImage(ImgData, initalImageData);
		a.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		a.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		r_device.upsize_sparse_image(image, initalImageData.data(), targetMip);
	}
	else if(targetMip <= image.get_info().mipLevels) {
		r_device.downsize_sparse_image(image, targetMip);
	}

}
