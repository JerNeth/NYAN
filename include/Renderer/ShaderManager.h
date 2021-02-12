#ifndef RDSHADERMANAGER_H
#define RDSHADERMANAGER_H
#pragma once
#include "VkWrapper.h"
namespace vulkan {
	class ShaderManager {
	public:
		ShaderManager(LogicalDevice& device);
		template<typename ...Args>
		Program* request_program(Args&&... args) {
			ShaderLayout layout{};
			std::vector<std::string> shaderFileNames = {args ...};
			Utility::DataHasher<const char> hasher;
			for (const auto& filename : shaderFileNames) {
				hasher(filename.data(), filename.size());
			}
			auto hash = hasher();
			if (m_cachedPrograms.contains(hash))
			{
				return m_cachedPrograms.get(hash).value();
			}
			std::vector<Shader*> shaders;
			for (const auto& filename : shaderFileNames) {
				shaders.push_back(request_shader(filename, layout));
			}
			auto* program = r_device.request_program(shaders);
			program->set_pipeline_layout(r_device.request_pipeline_layout(layout));
			m_cachedPrograms.insert(hash, program);
			return program;
		}
	private:
		Shader* request_shader(const std::string& filename, ShaderLayout& layout);
		LogicalDevice& r_device;
		Utility::HashMap<Program*> m_cachedPrograms;
	};
}

#endif !RDSHADERMANAGER_H