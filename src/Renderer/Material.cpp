#include "Renderer/Material.h"

nyan::Material::Material(MaterialId id, const std::string& shaderName) : 
	m_id(id), 
	m_shaderName(shaderName)
{
}

void nyan::Material::bind(vulkan::CommandBufferHandle& cmd)
{
	uint32_t binding = 0;
	for (auto texture : m_textures) {
		cmd->bind_texture(1, binding++, 0, *texture->get_view(), vulkan::DefaultSampler::TrilinearClamp);
	}
}
void nyan::Material::add_texture(vulkan::Image* texture)
{
	m_textures.push_back(texture);
}

void nyan::Material::set_texture(size_t idx, vulkan::Image* texture)
{
	m_textures[idx] = texture;
}
