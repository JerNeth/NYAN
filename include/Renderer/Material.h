#pragma once
#ifndef RDMATERIAL_H
#define RDMATERIAL_H
#include "VkWrapper.h"
namespace nyan {
	using MaterialId = uint32_t;
	constexpr MaterialId invalidMaterialId = ~0u;
	class Material {
	public:
		Material(MaterialId id, const std::string& shaderName);
		MaterialId get_id() const noexcept {
			return m_id;
		}
		void bind(vulkan::CommandBufferHandle& cmd);
		const std::string& get_shader_name() const noexcept {
			return m_shaderName;
		}
		void add_texture(vulkan::Image* texture);
		void set_texture(size_t idx, vulkan::Image* texture);
	private:
		MaterialId m_id = invalidMaterialId;
		std::vector<vulkan::Image*> m_textures;
		//vulkan::BufferHandle m_materialData;
		std::string m_shaderName;
	};
}

#endif !RDMATERIAL_H