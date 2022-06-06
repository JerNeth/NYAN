#include "Renderer/MaterialManager.h"
#include "Renderer/TextureManager.h"
#include "VulkanWrapper/Sampler.h"
#include "Buffer.h"

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
		.normalTexId {r_textureManager.get_texture_idx(data.normalTex, "normal.png")},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.pbrTexId {r_textureManager.get_texture_idx(data.diffuseTex, "white.png")},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.albedo_R {1.0f},
		.albedo_G {1.0f},
		.albedo_B {1.0f},
		.albedo_A {1.0f},
		.roughness {0.05f},
		.F0_R {0.04},
		.F0_G {0.04},
		.F0_B {0.04},
	});
}

nyan::MaterialId nyan::MaterialManager::add_material(const nyan::MaterialData& data)
{
	auto binding = add(nyan::shaders::Material{
		.albedoTexId {r_textureManager.get_texture_idx(data.diffuseTex, "white.png")},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.normalTexId {r_textureManager.get_texture_idx(data.normalTex, "normal.png")},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.pbrTexId {r_textureManager.get_texture_idx(data.diffuseTex, "white.png")},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.albedo_R {1.0f},
		.albedo_G {1.0f},
		.albedo_B {1.0f},
		.albedo_A {1.0f},
		.roughness {0.05f},
		.F0_R {0.04},
		.F0_G {0.04},
		.F0_B {0.04},
	});
	m_materialIndex.emplace(data.name, binding);
	return binding;
}

nyan::MaterialId nyan::MaterialManager::get_material(const std::string& name)
{
	if (m_materialIndex.find(name) == m_materialIndex.end()) {
		Utility::log().format("Couldn't get material \"{}\"", name);
		return 0;
	}
	assert(m_materialIndex.find(name) != m_materialIndex.end());
	return m_materialIndex[name];
}

nyan::shaders::Material& nyan::MaterialManager::get_material(MaterialId idx)
{
	return get(idx);
}

const nyan::shaders::Material& nyan::MaterialManager::get_material(MaterialId idx) const
{
	return get(idx);
}
