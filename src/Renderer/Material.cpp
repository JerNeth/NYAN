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
		cmd->bind_texture(1, binding++, *texture->get_view(), vulkan::DefaultSampler::TrilinearClamp);
	}
}
void nyan::Material::add_texture(vulkan::Image* texture)
{
	m_textures.push_back(texture);
}