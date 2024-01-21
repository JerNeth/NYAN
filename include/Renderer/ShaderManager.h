#ifndef RDSHADERMANAGER_H
#define RDSHADERMANAGER_H
#pragma once
#include <filesystem>
#include "VkWrapper.h"
#include "VulkanWrapper/Shader.h"

namespace vulkan {
	class ShaderManager {
	public:
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
		template<typename... Args>
		ShaderId get_shader_instance_id_workgroup_size(const std::string& name, const uint32_t x, const uint32_t y, const uint32_t z, Args... args) noexcept(false)
		{
			const auto& maxSize = r_device.get_physical_device_properties().limits.maxComputeWorkGroupSize;
			const auto& maxInvocations = r_device.get_physical_device_properties().limits.maxComputeWorkGroupInvocations;
			assert(x <= maxSize[0]); //Typically 1024, 1536 on older NVIDIA
			assert(y <= maxSize[1]); //Typically 1024
			assert(z <= maxSize[2]); //64 on NVIDIA and Intel, 1024 on AMD
			assert(x * y * z <= maxInvocations); //1024 on NVIDIA
			if (x > maxSize[0] ||
				y > maxSize[1] ||
				z > maxSize[2] ||
				x * y * z > maxInvocations)
				throw_size_error(x, y, z, maxSize[0], maxSize[1], maxSize[2], maxInvocations);

			return get_shader_instance_id(name, 
				ShaderStorage::SpecializationConstant{ "local_size_x", x },
				ShaderStorage::SpecializationConstant{ "local_size_y", y },
				ShaderStorage::SpecializationConstant{ "local_size_z", z },
				std::forward<Args>(args)...);
		}
		template<typename... Args>
		ShaderId get_shader_instance_id(const std::string& name, Args... args) noexcept
		{

			Utility::Hasher h;
			h(name);
			(h(args),...);
			auto res = m_shaderInstanceMapping2.find(h());
			if (res != m_shaderInstanceMapping2.end()) {
				return res->second;
			}
			if (auto it = m_shaderMapping.find(name); it != m_shaderMapping.end()) {
				auto shaderId = it->second;
				auto& shaderStorage = r_device.get_shader_storage();
				auto shaderInstanceId = shaderStorage.add_instance_with_constants(shaderId, std::forward<Args>(args)...);
				m_shaderInstanceMapping2.emplace(h(), shaderInstanceId);
				return shaderInstanceId;
			}
			Utility::Logger::warning().format("Requested shader not found {}\n", name);
			return invalidShaderId;
		}
		//ShaderId get_shader_instance_id(const std::string& name) const noexcept;

	private:
		static void throw_size_error(uint32_t x, uint32_t y, uint32_t z, uint32_t maxX, uint32_t maxY, uint32_t maxZ, uint32_t maxInv) noexcept(false);
		void load_shaders(const std::filesystem::path& shaderDirectory);
		//Shader* request_shader(const std::string& filename);
		LogicalDevice& r_device;
		//Utility::HashMap<Program*> m_cachedPrograms;
		//Utility::NonInvalidatingMap<Utility::HashValue, Program> m_cachedPrograms;
		//Utility::NonInvalidatingMap<std::string, Shader> m_cachedShaders;

		std::unordered_map<std::string, ShaderId> m_shaderMapping;
		std::unordered_map<std::string, ShaderId> m_shaderInstanceMapping;
		std::unordered_map<uint64_t, ShaderId> m_shaderInstanceMapping2;
	};
}

#endif //!RDSHADERMANAGER_H