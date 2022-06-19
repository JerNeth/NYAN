#ifndef RDSHADERMANAGER_H
#define RDSHADERMANAGER_H
#pragma once
#include <filesystem>
#include "VkWrapper.h"

namespace vulkan {
	class ShaderManager {
	public:
		ShaderManager(LogicalDevice& device, const std::filesystem::path& shaderDirectory = (std::filesystem::current_path() / "shaders"));
		//template<typename ...Args>
		//Program* request_program(Args&&... args) {
		//	ShaderLayout layout{};
		//	std::vector<std::string> shaderFileNames = {args ...};
		//	Utility::DataHasher<const char> hasher;
		//	for (const auto& filename : shaderFileNames) {
		//		hasher(filename.data(), filename.size());
		//	}
		//	auto hash = hasher();
		//	if (m_cachedPrograms.contains(hash))
		//	{
		//		return &m_cachedPrograms.get(hash);
		//	}
		//	std::vector<Shader*> shaders;
		//	for (const auto& filename : shaderFileNames) {
		//		auto* shader = request_shader(filename);
		//		shaders.push_back(shader);
		//		layout.combine(shader->get_layout());
		//	}
		//	auto& program = m_cachedPrograms.emplace(hash, shaders);
		//	program.set_pipeline_layout(r_device.request_pipeline_layout(layout));
		//	return &program;
		//}
		ShaderId get_shader_id(const std::string& name) const noexcept;
		ShaderId get_shader_instance_id(const std::string& name) const noexcept;

	private:
		//Shader* request_shader(const std::string& filename);
		LogicalDevice& r_device;
		//Utility::HashMap<Program*> m_cachedPrograms;
		//Utility::NonInvalidatingMap<Utility::HashValue, Program> m_cachedPrograms;
		//Utility::NonInvalidatingMap<std::string, Shader> m_cachedShaders;

		std::unordered_map<std::string, ShaderId> m_shaderMapping;
		std::unordered_map<std::string, ShaderId> m_shaderInstanceMapping;
	};
}

#endif !RDSHADERMANAGER_H