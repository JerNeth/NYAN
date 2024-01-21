#include "Renderer/MaterialManager.h"
#include "Renderer/TextureManager.h"
#include "VulkanWrapper/Sampler.hpp"
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
		.emissiveTexId {r_textureManager.get_texture_idx(data.diffuseTex, "black.png")},
		.emissiveSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.albedo_R {1.0f},
		.albedo_G {1.0f},
		.albedo_B {1.0f},
		.albedo_A {1.0f},
		//.roughness {0.05f},
		.roughness {0.5f},
		//.F0_R {0.04},
		//.F0_G {0.04},
		//.F0_B {0.04},
		.alphaDiscard {0.5}
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
		.emissiveTexId {r_textureManager.get_texture_idx(data.diffuseTex, "black.png")},
		.emissiveSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.albedo_R {1.0f},
		.albedo_G {1.0f},
		.albedo_B {1.0f},
		.albedo_A {1.0f},
		//.roughness {0.05f},
		.roughness {0.5f},
		//.F0_R {0.04},
		//.F0_G {0.04},
		//.F0_B {0.04},
		.alphaDiscard {0.5}
	});
	m_materialIndex.emplace(data.name, binding);
	return binding;
}

nyan::MaterialId nyan::MaterialManager::add_material(const PBRMaterialData& data)
{
	auto binding = add(nyan::shaders::Material{
		.albedoTexId {r_textureManager.get_texture_idx(data.albedoTex)},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.normalTexId {r_textureManager.get_texture_idx(data.normalTex)},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.pbrTexId {r_textureManager.get_texture_idx(data.roughnessMetalnessTex)},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.emissiveTexId {r_textureManager.get_texture_idx(data.emissiveTex)},
		.emissiveSampler {static_cast<uint32_t>(vulkan::DefaultSampler::TrilinearWrap)},
		.emissive_R {data.emissiveFactor.x()},
		.emissive_G {data.emissiveFactor.y()},
		.emissive_B {data.emissiveFactor.z()},
		.albedo_R {data.albedoFactor.x()},
		.albedo_G {data.albedoFactor.y()},
		.albedo_B {data.albedoFactor.z()},
		.albedo_A {data.albedoFactor.w()},
		.metalness {data.metallicFactor},
		.roughness {data.roughnessFactor},
		.alphaDiscard {data.alphaCutoff},
		.flags {(data.doubleSided ? nyan::shaders::MATERIAL_DOUBLE_SIDED_FLAG : 0u ) |
				((data.alphaMode == AlphaMode::AlphaTest) ? nyan::shaders::MATERIAL_ALPHA_TEST_FLAG : 0u) |
				((data.alphaMode == AlphaMode::AlphaBlend) ? nyan::shaders::MATERIAL_ALPHA_BLEND_FLAG : 0u)}
		});
	m_materialIndex.emplace(data.name, binding);
	return binding;
}

nyan::MaterialId nyan::MaterialManager::get_material(const std::string& name)
{
	if (m_materialIndex.find(name) == m_materialIndex.end()) {
		Utility::Logger::warning().format("Couldn't get material \"{}\"", name);
		return nyan::shaders::INVALID_BINDING;
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
