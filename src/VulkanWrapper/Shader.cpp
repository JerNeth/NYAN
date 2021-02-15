#include "Shader.h"
#include "DescriptorSet.h"
#include "LogicalDevice.h"
vulkan::Shader::Shader(LogicalDevice& parent, const std::vector<uint32_t>& shaderCode) :
	m_parent(parent),
	m_layout{}
{
	Utility::VectorHash<uint32_t> hasher;
	m_hashValue = hasher(shaderCode);
	create_module(shaderCode);
	parse_shader( shaderCode);
}

vulkan::Shader::~Shader()
{
	if(m_module != VK_NULL_HANDLE)
		vkDestroyShaderModule(m_parent.m_device, m_module, m_parent.m_allocator);
}

vulkan::ShaderStage vulkan::Shader::get_stage()
{
	return m_stage;
}

inline std::tuple<uint32_t, uint32_t, const spirv_cross::SPIRType&> vulkan::Shader::get_values(const spirv_cross::Resource& resource, const spirv_cross::Compiler& comp) const
{
	auto set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
	auto binding = comp.get_decoration(resource.id, spv::DecorationBinding);
	auto& type = comp.get_type(resource.type_id);
	return std::make_tuple(set, binding, type);
}

inline void vulkan::Shader::array_info(std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS>& layouts, const spirv_cross::SPIRType& type, uint32_t set, uint32_t binding) const
{

	layouts[set].stages[binding].set(m_stage);
	//layouts[set].stages[binding] |= 1u << static_cast<uint32_t>(m_stage);
	auto& size = layouts[set].arraySizes[binding];
	if (type.array.empty()) {
		size = 1;
	}
	else {
		//TODO Error handling
		size = static_cast<uint8_t>(type.array.front());
	}
}

void vulkan::Shader::parse_shader(const std::vector<uint32_t>& shaderCode)
{
	using namespace spirv_cross;
	//bool usesBinding = false;
	Compiler comp(shaderCode); 
	auto executionModel = comp.get_execution_model();
	if (executionModel > NUM_SHADER_STAGES)
		throw std::runtime_error("Unsupported Shadertype");
	m_stage = static_cast<ShaderStage>(executionModel);
	ShaderResources resources = comp.get_shader_resources();
	
	
	for (auto& uniformBuffer : resources.uniform_buffers) {
		auto [set, binding, type] = get_values(uniformBuffer, comp);
		m_layout.used.set(set);
		m_layout.descriptors[set].uniformBuffer.set(binding);
		array_info(m_layout.descriptors, type, set, binding);
	}
	for (auto& sampledImage : resources.sampled_images) {
		auto [set, binding, type] = get_values(sampledImage, comp);
		m_layout.used.set(set);
		if (type.image.dim == spv::DimBuffer) {
			m_layout.descriptors[set].sampledBuffer.set(binding);
		}
		else {
			m_layout.descriptors[set].imageSampler.set(binding);
		}
		if (comp.get_type(type.image.type).basetype == SPIRType::BaseType::Float) {
			m_layout.descriptors[set].fp.set(binding);
		}
		array_info(m_layout.descriptors, type, set, binding);
	}
	for (auto& subpassInput : resources.subpass_inputs) {
		auto [set, binding, type] = get_values(subpassInput, comp);
		m_layout.used.set(set);
		m_layout.descriptors[set].inputAttachment.set(binding);

		if (comp.get_type(type.image.type).basetype == SPIRType::BaseType::Float) {
			m_layout.descriptors[set].fp.set(binding);
		}
		array_info(m_layout.descriptors, type, set, binding);
	}
	for (auto& separateImage : resources.separate_images) {
		auto [set, binding, type] = get_values(separateImage, comp);
		m_layout.used.set(set);
		if (type.image.dim == spv::DimBuffer) {
			m_layout.descriptors[set].sampledBuffer.set(binding);
		}
		else {
			m_layout.descriptors[set].separateImage.set(binding);
		}
		if (comp.get_type(type.image.type).basetype == SPIRType::BaseType::Float) {
			m_layout.descriptors[set].fp.set(binding);
		}
		array_info(m_layout.descriptors, type, set, binding);
	}
	for (auto& separateSampler : resources.separate_samplers) {
		auto [set, binding, type] = get_values(separateSampler, comp);
		m_layout.used.set(set);
		m_layout.descriptors[set].seperateSampler.set(binding);

		array_info(m_layout.descriptors, type, set, binding);
	}
	for (auto& storageImage : resources.storage_images) {
		auto [set, binding, type] = get_values(storageImage, comp);
		m_layout.used.set(set);
		m_layout.descriptors[set].storageImage.set(binding);
		if (comp.get_type(type.image.type).basetype == SPIRType::BaseType::Float) {
			m_layout.descriptors[set].fp.set(binding);
		}
		array_info(m_layout.descriptors, type, set, binding);
	}
	for (auto& storageImage : resources.storage_buffers) {
		auto [set, binding, type] = get_values(storageImage, comp);
		m_layout.used.set(set);
		m_layout.descriptors[set].storageBuffer.set(binding);
		array_info(m_layout.descriptors, type, set, binding);
	}
	for (auto& attrib : resources.stage_inputs) {
		if (m_stage == vulkan::ShaderStage::Vertex) {
			auto& type = comp.get_type(attrib.type_id);
			auto location = comp.get_decoration(attrib.id, spv::DecorationLocation);
			m_layout.attributeElementCounts[location] = type.vecsize;
			m_layout.inputs.set(location);
		}
	}
	for (auto& attrib : resources.stage_outputs) {
		if (m_stage == vulkan::ShaderStage::Fragment) {
			auto location = comp.get_decoration(attrib.id, spv::DecorationLocation);
			m_layout.outputs.set(location);
		}
	}
	if (!resources.push_constant_buffers.empty()) {
		m_layout.pushConstantRange.stageFlags |= static_cast<VkShaderStageFlagBits>(1u << static_cast<uint32_t>(m_stage));
		m_layout.pushConstantRange.size =  static_cast<uint32_t>(comp.get_declared_struct_size(comp.get_type(resources.push_constant_buffers.front().base_type_id)));
	}
}

VkPipelineShaderStageCreateInfo vulkan::Shader::get_create_info()
{
	assert(m_module != VK_NULL_HANDLE);
	VkPipelineShaderStageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = static_cast<VkShaderStageFlagBits>(1u <<static_cast<uint32_t>(m_stage)),
		.module = m_module,
		.pName = "main"
	};
	return createInfo;
}

Utility::HashValue vulkan::Shader::get_hash()
{
	return m_hashValue;
}

void vulkan::Shader::create_module(const std::vector<uint32_t>& shaderCode)
{
	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shaderCode.size() * sizeof(uint32_t),
		.pCode = shaderCode.data(),
	};
	if (auto result = vkCreateShaderModule(m_parent.m_device, &createInfo, m_parent.m_allocator, &m_module); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create shader module, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create shader module, out of device memory");
		}
		if (result == VK_ERROR_INVALID_SHADER_NV) {
			throw std::runtime_error("VK: could not create shader module, invalid shader NV");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	
}

vulkan::Program::Program(const std::vector<Shader*>& shaders)
{
	Utility::Hasher h;
	for (auto shader : shaders) {
		h(shader->get_hash());
		set_shader(shader);
	}
	m_hashValue = h();
}

vulkan::Shader* vulkan::Program::get_shader(ShaderStage stage) const
{
	assert(static_cast<uint32_t>(stage) < NUM_SHADER_STAGES);
	return m_shaders[static_cast<uint32_t>(stage)];
}

void vulkan::Program::set_pipeline_layout(PipelineLayout* layout)
{
	m_layout = layout;
}

vulkan::PipelineLayout* vulkan::Program::get_pipeline_layout() const
{
	return m_layout;
}

void vulkan::Program::set_shader(Shader* shader)
{
	assert(shader);
	assert(static_cast<uint32_t>(shader->get_stage()) < NUM_SHADER_STAGES);
	m_shaders[static_cast<uint32_t>(shader->get_stage())] = shader;
}
