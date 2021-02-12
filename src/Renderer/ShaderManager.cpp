#include "..\..\include\Renderer\ShaderManager.h"

#include "ShaderManager.h"
static std::vector<uint32_t> read_binary_file(const std::string& filename) {

	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!(file.is_open())) {
		throw std::runtime_error("Could not open file: \"" + filename + "\"");
	}

	auto fileSize = file.tellg();
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();
	return buffer;
}
vulkan::ShaderManager::ShaderManager(LogicalDevice& device) : r_device(device) {

}
vulkan::Shader* vulkan::ShaderManager::request_shader(const std::string& filename, ShaderLayout& layout)
{
	//Utility::DataHash<const char> hasher;
	//auto hash = hasher(filename.data(), filename.size());
	auto shaderCode = read_binary_file(filename);
	auto val = r_device.register_shader(shaderCode);
	auto* shader = r_device.request_shader(val);
	shader->parse_shader(layout, shaderCode);
	return shader;
}