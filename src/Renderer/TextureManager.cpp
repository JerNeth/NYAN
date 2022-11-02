#include "Renderer/TextureManager.h"
#include "VulkanWrapper/Image.h"
#include "Utility/ImageReader.h"
#include "Utility/Exceptions.h"
#include "Renderer/ShaderInterface.h"

nyan::TextureManager::TextureManager(vulkan::LogicalDevice& device, bool streaming, const std::filesystem::path& folder) :
	r_device(device),
	m_directory(folder),
	m_streaming(streaming)
{
	std::vector< std::filesystem::path> defaultImages{ "white.png" ,"black.png", "normal.png" };
	for (const auto& img : defaultImages) {
		create_image(img);
	}
}

::vulkan::Image* nyan::TextureManager::request_texture(const std::string& name)
{
	std::filesystem::path path{ name };
	return request_texture(path);
}
vulkan::Image* nyan::TextureManager::request_texture(const std::filesystem::path& path)
{
	if (const auto& res = m_textureIndex.find(path.filename().string()); res != m_textureIndex.end()) {
		assert(m_usedTextures.find(res->second) != m_usedTextures.end());
		return m_usedTextures.find(res->second)->second.handle;
	}
	return create_image(path, m_minimumMipLevel);
}
vulkan::Image* nyan::TextureManager::request_texture(const TextureInfo& info, const std::vector<unsigned char>& data)
{
	if (const auto& res = m_textureIndex.find(info.name); res != m_textureIndex.end()) {
		assert(m_usedTextures.find(res->second) != m_usedTextures.end());
		return m_usedTextures.find(res->second)->second.handle;
	}
	return create_image(info, data);
}
uint32_t nyan::TextureManager::get_texture_idx(const std::string& name, const std::string& defaultTex)
{
	std::filesystem::path path{ name };
	if (const auto& res = m_textureIndex.find(path.filename().string()); res != m_textureIndex.end())
		return res->second;

	Utility::log_warning().format("Couldn't load \"{}\" using \"{}\" instead", name, defaultTex);

	if (const auto& res = m_textureIndex.find(defaultTex); res != m_textureIndex.end())
		return res->second;

	Utility::log_warning().format("Couldn't load \"{}\"", defaultTex);

	return nyan::shaders::INVALID_BINDING;
}
uint32_t nyan::TextureManager::get_texture_idx(const std::string& name)
{
	std::filesystem::path path{ name };
	if (const auto& res = m_textureIndex.find(path.filename().string()); res != m_textureIndex.end())
		return res->second;

	Utility::log_warning().format("Couldn't load \"{}\"", name);

	return nyan::shaders::INVALID_BINDING;
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

vulkan::ImageHandle nyan::TextureManager::create_image(const TextureInfo& info, const std::vector<unsigned char>& data)
{
	VkFormat format {VK_FORMAT_UNDEFINED};
	if (info.components == 1)
		if (info.bitsPerChannel == 8)
			if (info.sRGB)
				format = VK_FORMAT_R8_SRGB;
			else
				format = VK_FORMAT_R8_UNORM;
		else if (info.bitsPerChannel == 16)
			if (info.sRGB)
				assert(false);
			else
				format = VK_FORMAT_R16_UNORM;
		else if (info.bitsPerChannel == 32)
			if (info.sRGB)
				assert(false);
			else
				format = VK_FORMAT_R32_UINT;
		else
			assert(false);
	else if (info.components == 2)
		if (info.bitsPerChannel == 8)
			if (info.sRGB)
				format = VK_FORMAT_R8G8_SRGB;
			else
				format = VK_FORMAT_R8G8_UNORM;
		else if (info.bitsPerChannel == 16)
			if (info.sRGB)
				assert(false);
			else
				format = VK_FORMAT_R16G16_UNORM;
		else if (info.bitsPerChannel == 32)
			if (info.sRGB)
				assert(false);
			else
				format = VK_FORMAT_R32G32_UINT;
		else
			assert(false);
	else if (info.components == 3)
		if (info.bitsPerChannel == 8)
			if (info.sRGB)
				format = VK_FORMAT_R8G8B8_SRGB;
			else
				format = VK_FORMAT_R8G8B8_UNORM;
		else if (info.bitsPerChannel == 16)
			if (info.sRGB)
				assert(false);
			else
				format = VK_FORMAT_R16G16B16_UNORM;
		else if (info.bitsPerChannel == 32)
			if (info.sRGB)
				assert(false);
			else
				format = VK_FORMAT_R32G32B32_UINT;
		else
			assert(false);
	else if (info.components == 4)
		if (info.bitsPerChannel == 8)
			if (info.sRGB)
				format = VK_FORMAT_R8G8B8A8_SRGB;
			else
				format = VK_FORMAT_R8G8B8A8_UNORM;
		else if (info.bitsPerChannel == 16)
			if (info.sRGB)
				assert(false);
			else
				format = VK_FORMAT_R16G16B16A16_UNORM;
		else if (info.bitsPerChannel == 32)
			if (info.sRGB)
				assert(false);
			else
				format = VK_FORMAT_R32G32B32A32_UINT;
		else
			assert(false);
	else
		assert(false);

	assert(format != VK_FORMAT_UNDEFINED);
	Utility::TextureInfo texInfo {
		.format {format},
		.type {VK_IMAGE_TYPE_2D},
		.width {info.width},
		.height {info.height},
		.depth {1},
		.arrayLayers {1},
		.mipLevels {1},
		.cube {false},
	};
	vulkan::ImageInfo imageInfo{
		.format = texInfo.format,
		.width = texInfo.width,
		.height = texInfo.height,
		.depth = texInfo.depth,
		.mipLevels = texInfo.mipLevels,
		.arrayLayers = texInfo.arrayLayers,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.type = texInfo.type,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.flags = texInfo.cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : static_cast<VkImageCreateFlags>(0x0u),
		.createFlags {}
	};
	vulkan::InitialImageData imageData{
		.data {data.data()},
		.mipOffsets {0},
		.mipCounts {1},
	};
	//info.createFlags.set(vulkan::ImageInfo::Flags::ConcurrentAsyncCompute);
	//info.createFlags.set(vulkan::ImageInfo::Flags::ConcurrentGraphics);
	imageInfo.createFlags.set(vulkan::ImageInfo::Flags::GenerateMips);
	if (m_useSparse) {
		imageInfo.flags |= (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT);
		auto image = r_device.create_sparse_image(imageInfo, &imageData);

		r_device.wait_idle();
		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = *image->get_view(), .imageLayout = imageInfo.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(info.name, idx);
		return image;
	}
	else {
		auto image = r_device.create_image(imageInfo, &imageData);

		r_device.wait_idle();

		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = *image->get_view(), .imageLayout = imageInfo.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(info.name, idx);

		return image;
	}
}

vulkan::ImageHandle nyan::TextureManager::create_image(const std::filesystem::path& file, uint32_t mipLevel)
{
	std::filesystem::path path = file;
	if (path.is_relative())
		path = m_directory / path;
	if (!file.extension().compare(".dds"))
		return create_dds_image(path, mipLevel);
	
	auto [data, texInfo] = Utility::ImageReader::read_image_file(path);


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
	//info.createFlags.set(vulkan::ImageInfo::Flags::ConcurrentAsyncCompute);
	//info.createFlags.set(vulkan::ImageInfo::Flags::ConcurrentGraphics);
	info.createFlags.set(vulkan::ImageInfo::Flags::GenerateMips);
	if (m_useSparse) {
		info.flags |= (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT);
		auto image = r_device.create_sparse_image(info, &data);

		r_device.wait_idle();
		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = *image->get_view(), .imageLayout = info.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(path.filename().string(), idx);
		Utility::ImageReader::free_image(data.data);
		return image;
	}
	else {
		auto image = r_device.create_image(info, &data);

		r_device.wait_idle();

		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = *image->get_view(), .imageLayout = info.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(path.filename().string(), idx);
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
	//info.createFlags.set(vulkan::ImageInfo::Flags::ConcurrentAsyncCompute);
	//info.createFlags.set(vulkan::ImageInfo::Flags::ConcurrentGraphics);
	if (m_useSparse) {
		info.flags |= (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT);
		auto image = r_device.create_sparse_image(info, initalImageData.data());
		r_device.wait_idle();
		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = *image->get_view(), .imageLayout = info.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(file.filename().string(), idx);
		return image;
	}
	else {
		auto image = r_device.create_image(info, initalImageData.data());
		r_device.wait_idle();
		auto idx = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = *image->get_view(), .imageLayout = info.layout });
		m_usedTextures.emplace(idx, ::nyan::TextureManager::Texture{ image, texInfo });
		m_textureIndex.emplace(file.filename().string(), idx);
		return image;
	}
}