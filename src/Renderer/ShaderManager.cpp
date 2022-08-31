#include "Renderer/ShaderManager.h"
#include "VulkanWrapper/Shader.h"
#include "Utility/Exceptions.h"

static std::vector<uint32_t> read_binary_file(const std::filesystem::path& path)
{

	std::vector<uint32_t> data;
	std::ifstream in(path, std::ios::binary);
	if (in.is_open()) {
		auto size = std::filesystem::file_size(path);
		data.resize(size / sizeof(uint32_t));
		in.read(reinterpret_cast<char*>(data.data()), size);
		in.close();
	}
	else {
		Utility::log("Could not open: \"" + path.string() + '\"');
	}

	return data;
}

vulkan::ShaderManager::ShaderManager(LogicalDevice& device, const std::filesystem::path& shaderDirectory)
	: r_device(device) 
{
	load_shaders(shaderDirectory);
}

vulkan::ShaderId vulkan::ShaderManager::get_shader_id(const std::string& name) const noexcept
{
	if (auto it = m_shaderMapping.find(name); it != m_shaderMapping.end())
		return it->second;
	return invalidShaderId;
}

void vulkan::ShaderManager::throw_size_error(uint32_t x, uint32_t y, uint32_t z, uint32_t maxX, uint32_t maxY, uint32_t maxZ) noexcept(false)
{
	throw Utility::DevicePropertyException(std::format("Requested WorkgroupSize ({}, {}, {}) exceeds supported: ({}, {}, {})",
		x, y, z, maxX, maxY, maxZ));
}
void vulkan::ShaderManager::load_shaders(const std::filesystem::path& shaderDirectory) 
{
	if (std::filesystem::exists(shaderDirectory)) {
		r_device.wait_idle();
		auto& shaderStorage = r_device.get_shader_storage();
		shaderStorage.clear();
		for (const auto& entry : std::filesystem::directory_iterator{ shaderDirectory }) {
			if (!entry.is_regular_file())
				continue;
			if (entry.path().extension().compare(".spv"))
				continue;
			auto shaderCode = read_binary_file(entry.path());
			auto shaderId = shaderStorage.add_shader(shaderCode);
			auto* shader = shaderStorage.get_shader(shaderId);
			shader->set_debug_label(entry.path().stem().string().c_str());
			m_shaderMapping[entry.path().stem().string()] = shaderId;
			//auto shaderInstanceId = shaderStorage.add_instance(shaderId);
			//m_shaderInstanceMapping[entry.path().stem().string()] = shaderInstanceId;
		}
	}
	else {
		Utility::log("Invalid shader directory given");
	}
}

//vulkan::ShaderId vulkan::ShaderManager::get_shader_instance_id(const std::string& name) const noexcept
//{
//	if (auto it = m_shaderInstanceMapping.find(name); it != m_shaderInstanceMapping.end())
//		return it->second;
//	Utility::log(std::format("Requested shader not found {}\n", name));
//	return invalidShaderId;
//}

//vulkan::Shader* vulkan::ShaderManager::request_shader(const std::string& filename)
//{
//	//Utility::DataHash<const char> hasher;
//	//auto hash = hasher(filename.data(), filename.size());
//	if (m_cachedShaders.contains(filename))
//		return &m_cachedShaders.get(filename);
//
//	auto shaderCode = read_binary_file(filename);
//	//auto val = r_device.register_shader(shaderCode);
//	//auto* shader = r_device.request_shader(val);
//	return &m_cachedShaders.emplace(filename, r_device, shaderCode);
//}