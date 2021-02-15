#include "Renderer/TextureManager.h"
#include "DDSReader.h"

nyan::TextureManager::TextureManager(vulkan::LogicalDevice& device) :
	r_device(device)
{
}

::vulkan::Image* nyan::TextureManager::request_texture(const std::string& name)
{
	if (const auto& res = m_usedTextures.find(name); res != m_usedTextures.end()) {
		return res->second;
	}
	auto ImgData = Utility::DDSReader::readDDSFileInMemory(name);
	std::vector<vulkan::InitialImageData> initalImageData;
	auto a = Utility::DDSReader::parseImage(ImgData, initalImageData);
	a.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	a.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	auto tex = r_device.create_image(a, initalImageData.data());
	m_usedTextures.emplace(name, tex);
	return tex;
}
