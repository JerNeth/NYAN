#include "Renderer/MaterialManager.h"
#include "Renderer/TextureManager.h"

nyan::MaterialManager::MaterialManager(vulkan::LogicalDevice& device, nyan::TextureManager& textureManager) :
	DataManager(device),
	r_textureManager(textureManager)
{

}

void nyan::MaterialManager::set_material(const nyan::MaterialBinding& idx, const MaterialData& data)
{
	set(idx, InternalMaterialData{
	.diffuseTexId {r_textureManager.get_texture_idx(data.diffuseTex, "white.png")},
	.normalTexId {r_textureManager.get_texture_idx(data.normalTex, "black.png")},
	.ambientColor {data.ambientColor},
	.ambientFactor {data.ambientFactor},
	.diffuseColor {data.diffuseColor},
	.diffuseFactor {data.diffuseFactor},
	.shininessFactor {data.shininessFacor},
	});
}


nyan::MaterialBinding nyan::MaterialManager::add_material(const nyan::MaterialData& data)
{
	auto binding = add(InternalMaterialData{
		.diffuseTexId {r_textureManager.get_texture_idx(data.diffuseTex, "white.png")},
		.normalTexId {r_textureManager.get_texture_idx(data.normalTex, "black.png")},
		.ambientColor {data.ambientColor},
		.ambientFactor {data.ambientFactor},
		.diffuseColor {data.diffuseColor},
		.diffuseFactor {data.diffuseFactor},
		.shininessFactor {data.shininessFacor},
	});
	m_materialIndex.emplace(data.name, binding);
	return binding;
}

nyan::MaterialBinding nyan::MaterialManager::get_material(const std::string& name)
{
	assert(m_materialIndex.find(name) != m_materialIndex.end());
	return m_materialIndex[name];
}