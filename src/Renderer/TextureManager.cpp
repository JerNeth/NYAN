#include "Renderer/TextureManager.h"
#include "Utility/ImageReader.h"

nyan::TextureManager::TextureManager(vulkan::LogicalDevice& device, bool streaming) :
	r_device(device),
	m_streaming(streaming)
{
	create_image("white.png");
	create_image("normal.png");
	create_image("black.png");
}

::vulkan::Image* nyan::TextureManager::request_texture(const std::string& name)
{
	if (const auto& res = m_textureIndex.find(name); res != m_textureIndex.end()) {
		assert(m_usedTextures.find(res->second) != m_usedTextures.end());
		return m_usedTextures.find(res->second)->second.handle;
	}
	return create_image(name, m_minimumMipLevel);
}
uint32_t nyan::TextureManager::get_texture_idx(const std::string& name, const std::string& defaultTex)
{
	if (const auto& res = m_textureIndex.find(name); res != m_textureIndex.end())
		return res->second;

	Utility::log().format("Couldn't load \"{}\" using \"{}\" instead", name, defaultTex);

	if (const auto& res = m_textureIndex.find(defaultTex); res != m_textureIndex.end())
		return res->second;

	Utility::log().format("Couldn't load \"{}\"", defaultTex);

	return 0;
}
void nyan::TextureManager::change_mip(const std::string& name, uint32_t targetMip)
{
	const auto& res = m_textureIndex.find(name);
	if (res == m_textureIndex.end())
		return;
	if (targetMip < m_minimumMipLevel)
		targetMip = m_minimumMipLevel;
	assert(m_usedTextures.find(res->second) != m_usedTextures.end());
	auto& pair = m_usedTextures.find(res->second)->second;
	vulkan::Image& image = *pair.handle;
	if (image.is_being_resized())
		return;
	if (image.is_sparse()) {
		if (image.get_available_mip() == targetMip)
			return;
		else if (targetMip < image.get_available_mip()) {
			auto imgData = Utility::DDSReader::readDDSFileInMemory(name);
			std::vector<vulkan::InitialImageData> initalImageData = Utility::DDSReader::parseImage(pair.info,imgData);
			r_device.upsize_sparse_image(image, initalImageData.data(), targetMip);
		}
		else if (targetMip <= image.get_info().mipLevels) {
			r_device.downsize_sparse_image(image, targetMip);
		}
	}
	else {
		if ((pair.info.mipLevels - targetMip) == image.get_info().mipLevels)
			return;
		auto newImage = create_image(name, targetMip);
		image = std::move(*newImage);
	}

}

vulkan::ImageHandle nyan::TextureManager::create_image(const std::filesystem::path& file, uint32_t mipLevel)
{
	if (!file.extension().compare(".dds"))
		return create_dds_image(file, mipLevel);
	
	auto [data, texInfo] = Utility::ImageReader::read_image_file(file);


	vulkan::ImageInfo info{
		.format = texInfo.format,
		.width = Math::max(1u, texInfo.width >> mipLevel),
		.height = Math::max(1u, texInfo.height >> mipLevel),
		.depth = texInfo.depth,
		.mipLevels = texInfo.mipLevels - Math::min(mipLevel, texInfo.mipLevels -1),
		.arrayLayers = texInfo.arrayLayers,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.type = texInfo.type,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.flags = texInfo.cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : static_cast<VkImageCreateFlags>(0x0u),
		.createFlags {}
	};
	info.createFlags.set(vulkan::ImageInfo::Flags::GenerateMips);
	//info.createFlags.set(vulkan::ImageInfo::Flags::ConcurrentAsyncCompute);
	//info.createFlags.set(vulkan::ImageInfo::Flags::ConcurrentGraphics);
	if (m_useSparse) {
		info.flags |= (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT);
		auto image = r_device.create_sparse_image(info, &data);

		r_device.wait_idle();
		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = image->get_view()->get_image_view(), .imageLayout = info.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(file.string(), idx);
		Utility::ImageReader::free_image(data.data);
		return image;
	}
	else {
		auto image = r_device.create_image(info, &data);

		r_device.wait_idle();

		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = image->get_view()->get_image_view(), .imageLayout = info.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(file.string(), idx);
		Utility::ImageReader::free_image(data.data);
			
		return image;
	}
}
vulkan::ImageHandle nyan::TextureManager::create_dds_image(const std::filesystem::path& file, uint32_t mipLevel)
{
	auto texInfo = Utility::DDSReader::readDDSFileHeader(file);
	auto imgData = Utility::DDSReader::readDDSFileInMemory(file);
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
		auto image = r_device.create_sparse_image(info, initalImageData.data());
		r_device.wait_idle();
		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = image->get_view()->get_image_view(), .imageLayout = info.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(file.string(), idx);
		return image;
	}
	else {
		auto image = r_device.create_image(info, initalImageData.data());
		r_device.wait_idle();
		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = image->get_view()->get_image_view(), .imageLayout = info.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(file.string(), idx);
		return image;
	}
}