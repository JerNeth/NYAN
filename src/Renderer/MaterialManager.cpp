#include "Renderer/MaterialManager.h"
#include "Renderer/TextureManager.h"
#include "VulkanWrapper/Sampler.h"

nyan::MaterialManager::MaterialManager(vulkan::LogicalDevice& device, nyan::TextureManager& textureManager) :
	DataManager(device),
	r_textureManager(textureManager)
{

}

void nyan::MaterialManager::set_material(MaterialId idx, const MaterialData& data)
{
	set(idx, nyan::shaders::Material{
		.albedoTexId {r_textureManager.get_texture_idx(data.diffuseTex, "white.png")},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.normalTexId {r_textureManager.get_texture_idx(data.normalTex, "black.png")},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.pbrTexId {r_textureManager.get_texture_idx(data.diffuseTex, "white.png")},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
	});
}

nyan::MaterialId nyan::MaterialManager::add_material(const nyan::MaterialData& data)
{
	auto binding = add(nyan::shaders::Material{
		.albedoTexId {r_textureManager.get_texture_idx(data.diffuseTex, "white.png")},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.normalTexId {r_textureManager.get_texture_idx(data.normalTex, "black.png")},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.pbrTexId {r_textureManager.get_texture_idx(data.diffuseTex, "white.png")},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
	});
	m_materialIndex.emplace(data.name, binding);
	return binding;
}

nyan::MaterialId nyan::MaterialManager::get_material(const std::string& name)
{
	if (m_materialIndex.find(name) == m_materialIndex.end()) {
		Utility::log(std::format("Couldn't get material \"{}\"", name));
		return 0;
	}
	assert(m_materialIndex.find(name) != m_materialIndex.end());
	return m_materialIndex[name];
}